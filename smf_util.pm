use strict;
use FindBin;

package smf_util;

require $FindBin::Bin . "/smf_read.pm";
require $FindBin::Bin . "/smf_write.pm";


my $TPQN;
my %tempoHash;
my %beatHash;

sub Update
{
	my ($infoHashRef, $trackArrayRef) = @_;

	$TPQN = $$infoHashRef{ "TPQN" };

	foreach my $track (@$trackArrayRef) {
		foreach my $event (@$track) {
			my ($elapseTime, $isRunningStatus, @data) = @{ $event };

			if ($data[0] == 0xff) {
				my $metaEvent = $data[1];
				my @dataBody = @data;
				shift @dataBody; shift @dataBody;	# skip 0xff and meta event code
				while (my $val = shift @dataBody) {	# skip size
					if (!($val & 0x80)) { last; }
				}
				if      ($metaEvent == 0x51) {	# tempo
					my $tempo = ($dataBody[0] << 16) | ($dataBody[1] << 8) | $dataBody[2];
					$tempoHash{ $elapseTime } = $tempo;
				} elsif ($metaEvent == 0x58) {	# beat
					my $beat_num = $dataBody[0];
					my $beat_den = $dataBody[1];
					$beatHash{ $elapseTime } = [$beat_num, $beat_den];
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
sub Elapse2MeasBeatTick
{
	my ($targetElapseTime) = @_;
	my ($beat_num, $beat_den) = (4, 2);

	my $prevElapseTime = 0;
	my $meas = 1;
	my $beat = 1;
	my $tick = 0;
	foreach my $key (sort {$a <=> $b} (keys %beatHash)) {
		my $delta = $key - $prevElapseTime;
###		not accept beat on mid of measure
		my $remain = $delta % MeasDuration($beat_num, $beat_den);
#		printf("Measure Duration %d\n", MeasDuration($beat_num, $beat_den));
#		printf("meas increment %d\n",  int($delta / MeasDuration($beat_num, $beat_den)));
 
###		not accept beat on mid of measure

		if (($key - $remain > $targetElapseTime) && ($prevElapseTime <= $targetElapseTime)) {
			my $delta = $targetElapseTime - $prevElapseTime;
			$meas += int($delta / MeasDuration($beat_num, $beat_den));
			my $tmp_remain = $delta % MeasDuration($beat_num, $beat_den);
			$beat = int($tmp_remain / ($TPQN * 4 / (1 << $beat_den))) + 1;
			$tick = ($tmp_remain % ($TPQN * 4 / (1 << $beat_den)));
			return [$meas, $beat, $tick];
		} else {
			$meas += int($delta / MeasDuration($beat_num, $beat_den));

			($beat_num, $beat_den) = @{ $beatHash{ $key } };
#			printf("processing beat on $key ($meas, $remain) -- $beat_num, $beat_den\n");
			$prevElapseTime = $key - $remain;
		}
	}

	{
		my $delta = $targetElapseTime - $prevElapseTime;
		$meas += int($delta / MeasDuration($beat_num, $beat_den));
		my $remain = $delta % MeasDuration($beat_num, $beat_den);
		my $beat = int($remain / ($TPQN * 4 / (1 << $beat_den))) + 1;
		my $tick = ($remain % ($TPQN * 4 / (1 << $beat_den)));
		return [$meas, $beat, $tick];
	}
}

sub MeasBeatTick2Elapse
{
	my ($meas, $beat, $tick) = @_;

	my ($prev_meas, $prev_beat, $prev_tick);
	my $prev_elapse;
	foreach my $key (sort {$a <=> $b} (keys %beatHash)) {
		my ($tmp_meas, $tmp_beat, $tmp_tick) = @{ Elapse2MeasBeatTick($key) };
		if (($tmp_meas >= $meas) && ($beat >= $beat) && ($tick >= $tick)) {
			last;
		} else {
			$prev_meas = $tmp_meas; $prev_beat = $tmp_beat; $prev_tick = $tmp_tick;
			$prev_elapse = $key;
		}
	}
	my ($prev_beat_num, $prev_beat_den) = @{ $beatHash{ $prev_elapse } };
	my $delta = ($meas - $prev_meas) * $TPQN * 4 * $prev_beat_num / (1 << $prev_beat_den) + 
		        ($beat - $prev_beat) * $TPQN * 4 / (1 << $prev_beat_den) +
				($tick - $prev_tick);
	return $prev_elapse + $delta;
}

sub Elapse2MeasDuration
{
	my ($targetElapseTime) = @_;
	my ($beat_num, $beat_den) = (4, 2);

	my $prevElapseTime = 0;
	foreach my $key (sort {$a <=> $b} (keys %beatHash)) {
		my $delta = $key - $prevElapseTime;
###		not accept beat on mid of measure
		my $remain = $delta % MeasDuration($beat_num, $beat_den);

		if (($key - $remain > $targetElapseTime) && ($prevElapseTime <= $targetElapseTime)) {
			return MeasDuration($beat_num, $beat_den);
		} else {
			($beat_num, $beat_den) = @{ $beatHash{ $key } };
			$prevElapseTime = $key - $remain;
		}
	}

	return MeasDuration($beat_num, $beat_den);
}


##########################################################################
##########################################################################


sub ParseMetaEvent
{
	my ($elapseTime, @data) = @_;

	my ($meas, $beat, $tick) = @{ Elapse2MeasBeatTick($elapseTime) };

	my $metaEvent = $data[1];
	my @dataBody = @data;
	shift @dataBody; shift @dataBody;	# skip 0xff and meta event code
	while (my $val = shift @dataBody) {	# skip size
		if (!($val & 0x80)) { last; }
	}
	if      ($metaEvent == 0x00) {	# sequence number
		printf("%8d (%3d:%3d:%03d): sequence number\n", $elapseTime, $meas, $beat, $tick);
	} elsif ($metaEvent == 0x01) {	# text
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		printf("%8d (%3d:%3d:%03d): text: %s\n", $elapseTime, $meas, $beat, $tick, $str);
	} elsif ($metaEvent == 0x02) {	# copyright
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		printf("%8d (%3d:%3d:%03d): copyright: %s\n", $elapseTime, $meas, $beat, $tick, $str);
	} elsif ($metaEvent == 0x03) {	# sequence title
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		printf("%8d (%3d:%3d:%03d): sequence title: %s\n", $elapseTime, $meas, $beat, $tick, $str);
	} elsif ($metaEvent == 0x04) {	# instrument
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		printf("%8d (%3d:%3d:%03d): instrument: %s\n", $elapseTime, $meas, $beat, $tick, $str);
	} elsif ($metaEvent == 0x05) {	# lyric
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		printf("%8d (%3d:%3d:%03d): lyric: %s\n", $elapseTime, $meas, $beat, $tick, $str);
	} elsif ($metaEvent == 0x06) {	# marker
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		printf("%8d (%3d:%3d:%03d): marker: %s\n", $elapseTime, $meas, $beat, $tick, $str);
	} elsif ($metaEvent == 0x07) {	# que point
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		printf("%8d (%3d:%3d:%03d): que point: %s\n", $elapseTime, $meas, $beat, $tick, $str);
	} elsif ($metaEvent == 0x08) {	# program
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		printf("%8d (%3d:%3d:%03d): program: %s\n", $elapseTime, $meas, $beat, $tick, $str);
	} elsif ($metaEvent == 0x09) {	# device
		my $val = pack("C*", @dataBody);
		my $str = unpack("a*", $val);
		printf("%8d (%3d:%3d:%03d): device: %s\n", $elapseTime, $meas, $beat, $tick, $str);
	} elsif ($metaEvent == 0x20) {	# MIDI channel prefix
		printf("%8d (%3d:%3d:%03d): MIDI channel prefix (%d)\n", $elapseTime, $meas, $beat, $tick, $dataBody[0]);
	} elsif ($metaEvent == 0x21) {	# output port
		printf("%8d (%3d:%3d:%03d): output port (%d)\n", $elapseTime, $meas, $beat, $tick, $dataBody[0]);
	} elsif ($metaEvent == 0x2f) {	# track end
		printf("%8d (%3d:%3d:%03d): track end\n", $elapseTime, $meas, $beat, $tick);
	} elsif ($metaEvent == 0x51) {	# tempo
		my $tempo = ($dataBody[0] << 16) | ($dataBody[1] << 8) | $dataBody[2];
		printf("%8d (%3d:%3d:%03d): tempo (%d usec / BPM%g)\n", $elapseTime, $meas, $beat, $tick, $tempo, 60*1000000/$tempo);
	} elsif ($metaEvent == 0x54) {	# smpte offset
		printf("%8d (%3d:%3d:%03d): SMPTE offset\n", $elapseTime, $meas, $beat, $tick);
	} elsif ($metaEvent == 0x58) {	# beat
		my $beat_num = $dataBody[0];
		my $beat_den = $dataBody[1];
		printf("%8d (%3d:%3d:%03d): beat %d/%d\n", $elapseTime, $meas, $beat, $tick, $beat_num, (1<<$beat_den));
	} elsif ($metaEvent == 0x59) {	# key signature
		my $key = ($dataBody[0] >= 128) ? ($dataBody[0] - 256) : $dataBody[0];
		my $isMinor = $dataBody[1];
		my @key_str = ("C", "G", "D", "A", "Fes", "Ces", "Ges", "Des", "As", "Es", "B", "F", "C", "G", "D", "A", "E", "H", "Fis", "Cis", "Gis", "Dis", "Ais", "Eis",);
		printf("%8d (%3d:%3d:%03d): key signature %s %s\n", $elapseTime, $meas, $beat, $tick, $key_str[$key + 12], ($isMinor) ? "mol" : "dur");
	} elsif ($metaEvent == 0x7f) {	# sequencer meta
		printf("%8d (%3d:%3d:%03d): sequencer meta\n", $elapseTime, $meas, $beat, $tick);
	} else {
		printf("%8d (%3d:%3d:%03d): unknown meta event %d\n", $elapseTime, $meas, $beat, $tick, $metaEvent);
	}
}


sub ShowAllEvent
{
	my ($infoHashRef, $trackArrayRef) = @_;

	Update($infoHashRef, $trackArrayRef);

	my $cur_track = 1;
	foreach my $track (@$trackArrayRef) {
		print "next track ($cur_track) ...\n";
		foreach my $event (@$track) {
			my ($elapseTime, $isRunningStatus, @data) = @{ $event };

			if ($data[0] == 0xff) {
				ParseMetaEvent($elapseTime, @data);
			} else {
				my ($meas, $beat, $tick) = @{ Elapse2MeasBeatTick($elapseTime) };
				printf "%8d (%3d:%3d:%03d): %s", $elapseTime, $meas, $beat, $tick, ($isRunningStatus) ? "!" : " ";
				foreach my $val (@data) {
					printf("%02x ", $val);
				}
				print "\n";
			}
		}
		$cur_track++;
	}
}



1;
