use strict;
use FindBin;
use POSIX 'ceil';

package smf_util;

require $FindBin::RealBin . "/smf_read.pm";
require $FindBin::RealBin . "/smf_write.pm";


my $TPQN;
my %tempoHash;
my %beatHash;

sub Update
{
	my ($infoHashRef, $trackArrayRef) = @_;

	$TPQN = $$infoHashRef{ "TPQN" };

	my $prevTempoTickTime = 0;
	my $prevBeatTickTime = 0;
	$tempoHash{ 0 } = [500000, 0];
	$beatHash{ 0 } = [4, 2, 1];
	foreach my $track (@$trackArrayRef) {
		foreach my $event (@$track) {
			my ($tickTime, $isRunningStatus, @data) = @{ $event };

			if ($data[0] == 0xff) {
				my $metaEvent = $data[1];
				my @dataBody = @data;
				shift @dataBody; shift @dataBody;	# skip 0xff and meta event code
				while (my $val = shift @dataBody) {	# skip size
					if (!($val & 0x80)) { last; }
				}
				if      ($metaEvent == 0x51) {	# tempo
					my $tempo = ($dataBody[0] << 16) | ($dataBody[1] << 8) | $dataBody[2];
					my $realTime_TPQN = ($tickTime - $prevTempoTickTime) * $tempoHash{ $prevTempoTickTime }[0] + $tempoHash{ $prevTempoTickTime }[1];
					$tempoHash{ $tickTime } = [$tempo, $realTime_TPQN];
					$prevTempoTickTime = $tickTime;
				} elsif ($metaEvent == 0x58) {	# beat
					my $beat_num = $dataBody[0];
					my $beat_den = $dataBody[1];
					my ($prevBeatNum, $prevBeatDen, $prevMeas) = @{ $beatHash{ $prevBeatTickTime } };
					my $measure = ::ceil(($tickTime - $prevBeatTickTime) / MeasDuration($prevBeatNum, $prevBeatDen)) + $prevMeas;
					$beatHash{ $tickTime } = [$beat_num, $beat_den, $measure];
					$prevBeatTickTime = $tickTime;
				}
			}
		}
	}
}

sub MeasDuration
{
	my ($beat_num, $beat_den) = @_;
	return 4 * $TPQN * $beat_num / (1 << $beat_den);
}
sub Tick2MeasBeatTick
{
	my ($targetTickTime) = @_;

	my $prevTickTime = 0;
	foreach my $key (sort {$a <=> $b} (keys %beatHash)) {
		my $delta = $key - $prevTickTime;

###	if not accept beat on mid of measure
#		my ($beat_num, $beat_den, $meas) = $beatHash{ $prevTickTime };
#		my $remain = $delta % MeasDuration($beat_num, $beat_den);
#		if ($remain != 0) { next; }

		if (($key > $targetTickTime) && ($prevTickTime <= $targetTickTime)) {
			last;
		}
		$prevTickTime = $key;
	}

	{
		my ($beat_num, $beat_den, $meas) = @{ $beatHash{ $prevTickTime } };
		my $delta = $targetTickTime - $prevTickTime;
		$meas += int($delta / MeasDuration($beat_num, $beat_den));
		my $remain = $delta % MeasDuration($beat_num, $beat_den);
		my $beat = int($remain / ($TPQN * 4 / (1 << $beat_den))) + 1;
		my $tick = ($remain % ($TPQN * 4 / (1 << $beat_den)));
		return [$meas, $beat, $tick];
	}
}

sub MeasBeatTick2Tick
{
	my ($meas, $beat, $tick) = @_;

	my ($prev_meas, $prev_beat, $prev_tick);
	my $prev_tick;
	foreach my $key (sort {$a <=> $b} (keys %beatHash)) {
		my ($tmp_meas, $tmp_beat, $tmp_tick) = @{ Tick2MeasBeatTick($key) };
		if (($tmp_meas >= $meas) && ($beat >= $beat) && ($tick >= $tick)) {
			last;
		} else {
			$prev_meas = $tmp_meas; $prev_beat = $tmp_beat; $prev_tick = $tmp_tick;
			$prev_tick = $key;
		}
	}
	my ($prev_beat_num, $prev_beat_den, $dummy) = @{ $beatHash{ $prev_tick } };
	my $delta = ($meas - $prev_meas) * $TPQN * 4 * $prev_beat_num / (1 << $prev_beat_den) + 
		        ($beat - $prev_beat) * $TPQN * 4 / (1 << $prev_beat_den) +
				($tick - $prev_tick);
	return $prev_tick + $delta;
}

sub Tick2MeasDuration
{
	my ($targetTickTime) = @_;
	my ($beat_num, $beat_den) = (4, 2);

	my $prevTickTime = 0;
	foreach my $key (sort {$a <=> $b} (keys %beatHash)) {
		my $delta = $key - $prevTickTime;
###		not accept beat on mid of measure
		my $remain = $delta % MeasDuration($beat_num, $beat_den);

		if (($key - $remain > $targetTickTime) && ($prevTickTime <= $targetTickTime)) {
			return MeasDuration($beat_num, $beat_den);
		} else {
			($beat_num, $beat_den, my $dummy) = @{ $beatHash{ $key } };
			$prevTickTime = $key - $remain;
		}
	}

	return MeasDuration($beat_num, $beat_den);
}

sub Tick2RealElapseTime
{
	my ($targetTickTime) = @_;

	my $prevTickTime = 0;
	my $realTime_TPQN = 0;	# usec * $TPQN
	foreach my $key (sort {$a <=> $b} (keys %tempoHash)) {
		if (($key > $targetTickTime) && ($prevTickTime <= $targetTickTime)) {
			last;
		}
		$prevTickTime = $key;
#		print "key: $key, realTime_TPQN: $realTime_TPQN\n";
	}
	my $realTime_TPQN = ($targetTickTime - $prevTickTime) * $tempoHash{ $prevTickTime }[0] + $tempoHash{ $prevTickTime }[1];
	return $realTime_TPQN / $TPQN;
}


##########################################################################
##########################################################################


sub PrintEventTick
{
	my ($tickTime) = @_;

	my ($meas, $beat, $tick) = @{ Tick2MeasBeatTick($tickTime) };
	return sprintf("%8d (%3d:%3d:%03d)", $tickTime, $meas, $beat, $tick);
}

sub PrintEventTime
{
	my ($tickTime) = @_;

	my $usec = Tick2RealElapseTime($tickTime);
	my $seconds = $usec % 60000000;
	my $miliseconds = $usec % 1000000;
	return sprintf("(%3d'%02d\"%03d)", int($usec / 60000000), int($seconds / 1000000), $miliseconds / 1000);
}

sub ParseMetaEvent
{
	my ($tickTime, @data) = @_;

	my $metaEvent = $data[1];
	my @dataBody = @data;
	shift @dataBody; shift @dataBody;	# skip 0xff and meta event code
	while (my $val = shift @dataBody) {	# skip size
		if (!($val & 0x80)) { last; }
	}
	if      ($metaEvent == 0x00) {	# sequence number
		return sprintf("sequence number\n");
	} elsif ($metaEvent == 0x01) {	# text
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		return sprintf("text: %s\n", $str);
	} elsif ($metaEvent == 0x02) {	# copyright
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		return sprintf("copyright: %s\n", $str);
	} elsif ($metaEvent == 0x03) {	# sequence title
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		return sprintf("sequence title: %s\n", $str);
	} elsif ($metaEvent == 0x04) {	# instrument
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		return sprintf("instrument: %s\n", $str);
	} elsif ($metaEvent == 0x05) {	# lyric
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		return sprintf("lyric: %s\n", $str);
	} elsif ($metaEvent == 0x06) {	# marker
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		return sprintf("marker: %s\n", $str);
	} elsif ($metaEvent == 0x07) {	# que point
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		return sprintf("que point: %s\n", $str);
	} elsif ($metaEvent == 0x08) {	# program
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		return sprintf("program: %s\n", $str);
	} elsif ($metaEvent == 0x09) {	# device
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		return sprintf("device: %s\n", $str);
	} elsif ($metaEvent == 0x20) {	# MIDI channel prefix
		return sprintf("MIDI channel prefix (%d)\n", $dataBody[0]);
	} elsif ($metaEvent == 0x21) {	# output port
		return sprintf("output port (%d)\n", $dataBody[0]);
	} elsif ($metaEvent == 0x2f) {	# track end
		return sprintf("track end\n");
	} elsif ($metaEvent == 0x51) {	# tempo
		my $tempo = ($dataBody[0] << 16) | ($dataBody[1] << 8) | $dataBody[2];
		return sprintf("tempo (%d usec / BPM%g)\n", $tempo, 60*1000000/$tempo);
	} elsif ($metaEvent == 0x54) {	# smpte offset
		return sprintf("SMPTE offset\n");
	} elsif ($metaEvent == 0x58) {	# beat
		my $beat_num = $dataBody[0];
		my $beat_den = $dataBody[1];
		return sprintf("beat %d/%d\n", $beat_num, (1<<$beat_den));
	} elsif ($metaEvent == 0x59) {	# key signature
		my $key = ($dataBody[0] >= 128) ? ($dataBody[0] - 256) : $dataBody[0];
		my $isMinor = $dataBody[1];
		my @key_str = ("C", "G", "D", "A", "Fes", "Ces", "Ges", "Des", "As", "Es", "B", "F", "C", "G", "D", "A", "E", "H", "Fis", "Cis", "Gis", "Dis", "Ais", "Eis",);
		return sprintf("key signature %s %s\n", $key_str[$key + 12], ($isMinor) ? "mol" : "dur");
	} elsif ($metaEvent == 0x7f) {	# sequencer meta
		return sprintf("sequencer meta\n");
	} else {
		return sprintf("unknown meta event %d\n", $metaEvent);
	}
}


sub ShowAllEvent
{
	my ($infoHashRef, $trackArrayRef, $isIndicateTime) = @_;

	Update($infoHashRef, $trackArrayRef);

	my $cur_track = 1;
	foreach my $track (@$trackArrayRef) {
		print "next track ($cur_track) ...\n";
		foreach my $event (@$track) {
			my ($tickTime, $isRunningStatus, @data) = @{ $event };

			my $str = PrintEventTick($tickTime) . (($isIndicateTime) ? (" " . PrintEventTime($tickTime)) : "") . ": ";
			if ($data[0] == 0xff) {
				$str .= ParseMetaEvent($tickTime, @data);
			} else {
				$str .= ($isRunningStatus) ? "!" : " ";
				foreach my $val (@data) {
					$str .= sprintf("%02x ", $val);
				}
				$str .= "\n";
			}
			print $str;
		}
		$cur_track++;
	}
}



1;
