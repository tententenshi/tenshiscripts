use strict;
use FindBin;
use POSIX 'ceil';

package smf_util;

require $FindBin::RealBin . "/smf_read.pm";
require $FindBin::RealBin . "/smf_write.pm";


# %infoHash
#	$infoHash{ (format|num_of_track|TPQN) }

# @trackArray
#	$trackArray[0]->[$elapseTime, $isRunningStatus, @data]


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

sub PrintControlChange
{
	my ($cc_num) = @_;

	if      ($cc_num == 0x00) {
		return "Bank Select MSB";
	} elsif ($cc_num == 0x01) {
		return "Modulation";
	} elsif ($cc_num == 0x02) {
		return "Breath Controller";
	} elsif ($cc_num == 0x04) {
		return "Foot Controller";
	} elsif ($cc_num == 0x05) {
		return "Portament Time";
	} elsif ($cc_num == 0x06) {
		return "Data Entry MSB";
	} elsif ($cc_num == 0x07) {
		return "Volume";
	} elsif ($cc_num == 0x08) {
		return "Balance";
	} elsif ($cc_num == 0x0a) {
		return "Panning";
	} elsif ($cc_num == 0x0b) {
		return "Expression";
	} elsif ($cc_num == 0x10) {
		return "General Purpose Controllers #1";
	} elsif ($cc_num == 0x11) {
		return "General Purpose Controllers #2";
	} elsif ($cc_num == 0x12) {
		return "General Purpose Controllers #3";
	} elsif ($cc_num == 0x13) {
		return "General Purpose Controllers #4";
	} elsif ($cc_num == 0x20) {
		return "Bank Select LSB";
	} elsif ($cc_num == 0x21) {
		return "Modulation LSB";
	} elsif ($cc_num == 0x22) {
		return "Breath Controller LSB";
	} elsif ($cc_num == 0x24) {
		return "Foot Controller LSB";
	} elsif ($cc_num == 0x25) {
		return "Portament Time LSB";
	} elsif ($cc_num == 0x26) {
		return "Data Entry LSB";
	} elsif ($cc_num == 0x27) {
		return "Volume LSB";
	} elsif ($cc_num == 0x28) {
		return "Balance LSB";
	} elsif ($cc_num == 0x2a) {
		return "Panning LSB";
	} elsif ($cc_num == 0x2b) {
		return "Expression LSB";
	} elsif ($cc_num == 0x30) {
		return "General Purpose Controllers #1 LSB";
	} elsif ($cc_num == 0x31) {
		return "General Purpose Controllers #2 LSB";
	} elsif ($cc_num == 0x32) {
		return "General Purpose Controllers #3 LSB";
	} elsif ($cc_num == 0x33) {
		return "General Purpose Controllers #4 LSB";
	} elsif ($cc_num == 0x40) {
		return "Hold1 Sustain Pedal";
	} elsif ($cc_num == 0x41) {
		return "Portamento";
	} elsif ($cc_num == 0x42) {
		return "Sostenuto";
	} elsif ($cc_num == 0x43) {
		return "Soft Pedal";
	} elsif ($cc_num == 0x45) {
		return "Hold2";
	} elsif ($cc_num == 0x46) {
		return "Sound Variation";
	} elsif ($cc_num == 0x47) {
		return "Harmonic Intensity";
	} elsif ($cc_num == 0x48) {
		return "Release Time";
	} elsif ($cc_num == 0x49) {
		return "Attack Time";
	} elsif ($cc_num == 0x4a) {
		return "Brightness";
	} elsif ($cc_num == 0x50) {
		return "General Purpose Controllers #5";
	} elsif ($cc_num == 0x51) {
		return "General Purpose Controllers #6";
	} elsif ($cc_num == 0x52) {
		return "General Purpose Controllers #7";
	} elsif ($cc_num == 0x53) {
		return "General Purpose Controllers #8";
	} elsif ($cc_num == 0x54) {
		return "Portament Control";
	} elsif ($cc_num == 0x5b) {
		return "Reverb Depth";
	} elsif ($cc_num == 0x5c) {
		return "Tremolo Depth";
	} elsif ($cc_num == 0x5d) {
		return "Chorus Depth";
	} elsif ($cc_num == 0x5e) {
		return "Celeste (Detune) Depth";
	} elsif ($cc_num == 0x5f) {
		return "Phaser Depth";
	} elsif ($cc_num == 0x60) {
		return "Data Increment";
	} elsif ($cc_num == 0x61) {
		return "Data Decrement";
	} elsif ($cc_num == 0x62) {
		return "Non-Registered Parmeter Number (NRPN) LSB";
	} elsif ($cc_num == 0x63) {
		return "Non-Registered Parmeter Number (NRPN) MSB";
	} elsif ($cc_num == 0x64) {
		return "Registered Parmeter Number (RPN) LSB";
	} elsif ($cc_num == 0x65) {
		return "Registered Parmeter Number (RPN) MSB";
	} elsif ($cc_num == 0x79) {
		return "Reset All Controller";
	} elsif ($cc_num == 0x7a) {
		return "Local Control";
	} elsif ($cc_num == 0x7b) {
		return "All Notes Off";
	} elsif ($cc_num == 0x7c) {
		return "Omni Mode Off";
	} elsif ($cc_num == 0x7d) {
		return "Omni Mode On";
	} elsif ($cc_num == 0x7e) {
		return "Mono Mode On (Poly Off)";
	} elsif ($cc_num == 0x7f) {
		return "Mono Mode Off (Poly On)";
	} else {
		return "";
	}
}

sub PrintProgramChange
{
	my ($program_num) = @_;

	my @tone_tbl = (
		"Acoustic Grand",
		"Bright Acoustic",
		"Electric Grand",
		"Honky-Tonk",
		"Electric Piano 1",
		"Electric Piano 2",
		"Harpsichord",
		"Clavinet",
		"Celesta",
		"Glockenspiel",
		"Music Box",
		"Vibraphone",
		"Marimba",
		"Xylophone",
		"Tubular Bells",
		"Dulcimer",
		"Drawbar Organ",
		"Percussive Organ",
		"Rock Organ",
		"Church Organ",
		"Reed Organ",
		"Accoridan",
		"Harmonica",
		"Tango Accordian",
		"Nylon String Guitar",
		"Steel String Guitar",
		"Electric Jazz Guitar",
		"Electric Clean Guitar",
		"Electric Muted Guitar",
		"Overdriven Guitar",
		"Distortion Guitar",
		"Guitar Harmonics",
		"Acoustic Bass",
		"Electric Bass(finger)",
		"Electric Bass(pick)",
		"Fretless Bass",
		"Slap Bass 1",
		"Slap Bass 2",
		"Synth Bass 1",
		"Synth Bass 2",
		"Violin",
		"Viola",
		"Cello",
		"Contrabass",
		"Tremolo Strings",
		"Pizzicato Strings",
		"Orchestral Strings",
		"Timpani",
		"String Ensemble 1",
		"String Ensemble 2",
		"SynthStrings 1",
		"SynthStrings 2",
		"Choir Aahs",
		"Voice Oohs",
		"Synth Voice",
		"Orchestra Hit",
		"Trumpet",
		"Trombone",
		"Tuba",
		"Muted Trumpet",
		"French Horn",
		"Brass Section",
		"SynthBrass 1",
		"SynthBrass 2",
		"Soprano Sax",
		"Alto Sax",
		"Tenor Sax",
		"Baritone Sax",
		"Oboe",
		"English Horn",
		"Bassoon",
		"Clarinet",
		"Piccolo",
		"Flute",
		"Recorder",
		"Pan Flute",
		"Blown Bottle",
		"Skakuhachi",
		"Whistle",
		"Ocarina",
		"Square Wave",
		"Saw Wave",
		"Syn. Calliope",
		"Chiffer Lead",
		"Charang",
		"Solo Vox",
		"5th Saw Wave",
		"Bass& Lead",
		"Fantasia",
		"Warm Pad",
		"Polysynth",
		"Space Voice",
		"Bowed Glass",
		"Metal Pad",
		"Halo Pad",
		"Sweep Pad",
		"Ice Rain",
		"Soundtrack",
		"Crystal",
		"Atmosphere",
		"Brightness",
		"Goblin",
		"Echo Drops",
		"Star Theme",
		"Sitar",
		"Banjo",
		"Shamisen",
		"Koto",
		"Kalimba",
		"Bagpipe",
		"Fiddle",
		"Shanai",
		"Tinkle Bell",
		"Agogo",
		"Steel Drums",
		"Woodblock",
		"Taiko Drum",
		"Melodic Tom",
		"Synth Drum",
		"Reverse Cymbal",
		"Guitar Fret Noise",
		"Breath Noise",
		"Seashore",
		"Bird Tweet",
		"Telephone Ring",
		"Helicopter",
		"Applause",
		"Gunshot",
	);

	return $tone_tbl[$program_num];
}

sub PrintCommands
{
	my (@data) = @_;

	my @key_name_tbl = (
		"C ", "C#", "D ", "Eb", "E ", "F ", "F#", "G ", "Ab", "A ", "Bb", "B ",
	);
	if      (($data[0] & 0xf0) == 0x80) {
		my $note_num = $data[1];
		my $key_name = $key_name_tbl[$note_num % 12];
		my $key_octave = int($note_num / 12) - 1;
		return sprintf("Note Off %s%d", $key_name, $key_octave);
	} elsif (($data[0] & 0xf0) == 0x90) {
		my $note_num = $data[1];
		my $key_name = $key_name_tbl[$note_num % 12];
		my $key_octave = int($note_num / 12) - 1;
		if ($data[2] == 0) {
			return sprintf("Note Off %s%d", $key_name, $key_octave);
		} else {
			return sprintf("Note On  %s%d", $key_name, $key_octave);
		}
	} elsif (($data[0] & 0xf0) == 0xa0) {
		my $note_num = $data[1];
		my $key_name = $key_name_tbl[$note_num % 12];
		my $key_octave = int($note_num / 12) - 1;
		return sprintf("Poly Aft %s%d", $key_name, $key_octave);
	} elsif (($data[0] & 0xf0) == 0xb0) {
		my $cc_num = $data[1];
		my $cc_val = $data[2];
		return sprintf("Control Change No.%3d(0x%02x): %3d(0x%02x)	// %s", $cc_num, $cc_num, $cc_val, $cc_val, PrintControlChange($cc_num));
	} elsif (($data[0] & 0xf0) == 0xc0) {
		my $program_num = $data[1];
		return sprintf("Program Change %3d(0x%02x)	// %s", $program_num + 1, $program_num, PrintProgramChange($program_num));
	} elsif (($data[0] & 0xf0) == 0xd0) {
		my $val = $data[1];
		return sprintf("Channel Pressure %3d(0x%02x)", $val, $val);
	} elsif (($data[0] & 0xf0) == 0xe0) {
		my $val = ($data[1] << 7) | $data[2];
		my $norm_val = $val - 0x2000;
		return sprintf("Pitch Bend %5d(0x%02x)", $norm_val, $val);
	}
}

sub PrintMetaEvent
{
	my (@data) = @_;

	my $metaEvent = $data[1];
	my @dataBody = @data;
	shift @dataBody; shift @dataBody;	# skip 0xff and meta event code
	my $size;
	while (my $val = shift @dataBody) {	# skip size
		$size = ($size << 7) | ($val & 0x7f);
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
		if ($size == 2) {
			return sprintf("beat %d/%d\n", $beat_num, (1<<$beat_den));
		} else {
			return sprintf("beat %d/%d metro_tick(%d) quater_note(%d/32)\n", $beat_num, (1<<$beat_den), $dataBody[2], $dataBody[3]);
		}
	} elsif ($metaEvent == 0x59) {	# key signature
		my $key = ($dataBody[0] >= 128) ? ($dataBody[0] - 256) : $dataBody[0];
		my $isMinor = $dataBody[1];
		my @key_str = ("C", "G", "D", "A", "Fes", "Ces", "Ges", "Des", "As", "Es", "B", "F", "C", "G", "D", "A", "E", "H", "Fis", "Cis", "Gis", "Dis", "Ais", "Eis",);
		return sprintf("key signature %s %s\n", $key_str[$key + 12], ($isMinor) ? "mol" : "dur");
	} elsif ($metaEvent == 0x7f) {	# sequencer meta
		my $text;
		for (my $i = 0; $i < $size; $i++) {
			$text .= sprintf("%02x ", $dataBody[$i]);
		}
		return sprintf("sequencer meta $text\n");
	} else {
		return sprintf("unknown meta event %d\n", $metaEvent);
	}
}


sub ShowAllEvent
{
	my ($infoHashRef, $trackArrayRef, $isIndicateTime) = @_;

	Update($infoHashRef, $trackArrayRef);

	print "format: $infoHashRef->{ format }\n";
	print "num_of_track: $infoHashRef->{ num_of_track }\n";
	print "TPQN: $infoHashRef->{ TPQN }\n";
	print "\n";

	my $cur_track = 1;
	foreach my $track (@$trackArrayRef) {
		print "next track ($cur_track) ...\n";
		foreach my $event (@$track) {
			my ($tickTime, $isRunningStatus, @data) = @{ $event };

			my $str = PrintEventTick($tickTime) . (($isIndicateTime) ? (" " . PrintEventTime($tickTime)) : "") . ": ";
			if ($data[0] == 0xff) {
				$str .= PrintMetaEvent(@data);
			} else {
				$str .= ($isRunningStatus) ? "!" : " ";
				foreach my $val (@data) {
					$str .= sprintf("%02x ", $val);
				}
				$str .= "	// " . PrintCommands(@data);
				$str .= "\n";
			}
			print $str;
		}
		$cur_track++;
	}
}


sub ConvertToVariableVal
{
	my ($originalVal) = @_;

	my @data;
	unshift @data, ($originalVal & 0x7f);
	while ($originalVal > 0x7f) {
		$originalVal >>= 7;
		unshift @data, (0x80 | ($originalVal & 0x7f));
	}
	return @data;
}

sub ParseMetaEvent
{
	my ($metaEventStr) = @_;

	my @data;
	$data[0] = 0xff;

	my $metaEvent;

	if      ($metaEventStr =~ /sequence number/) {	# sequence number
		$data[1] = 0x00;
		push @data, ConvertToVariableVal(2);
		push @data, 0;
		push @data, 0;
	} elsif ($metaEventStr =~ /text: (.*)/) {	# text
		my $text = $1;
		$data[1] = 0x01;
		push @data, ConvertToVariableVal(length($text));
		my $val = pack("a*", $text);
		push @data, unpack("C*", $val);
	} elsif ($metaEventStr =~ /copyright: (.*)/) {	# copyright
		my $text = $1;
		$data[1] = 0x02;
		push @data, ConvertToVariableVal(length($text));
		my $val = pack("a*", $text);
		push @data, unpack("C*", $val);
	} elsif ($metaEventStr =~ /sequence title: (.*)/) {	# sequence title
		my $text = $1;
		$data[1] = 0x03;
		push @data, ConvertToVariableVal(length($text));
		my $val = pack("a*", $text);
		push @data, unpack("C*", $val);
	} elsif ($metaEventStr =~ /instrument: (.*)/) {	# instrument
		my $text = $1;
		$data[1] = 0x04;
		push @data, ConvertToVariableVal(length($text));
		my $val = pack("a*", $text);
		push @data, unpack("C*", $val);
	} elsif ($metaEventStr =~ /lyric: (.*)/) {	# lyric
		my $text = $1;
		$data[1] = 0x05;
		push @data, ConvertToVariableVal(length($text));
		my $val = pack("a*", $text);
		push @data, unpack("C*", $val);
	} elsif ($metaEventStr =~ /marker: (.*)/) {	# marker
		my $text = $1;
		$data[1] = 0x06;
		push @data, ConvertToVariableVal(length($text));
		my $val = pack("a*", $text);
		push @data, unpack("C*", $val);
	} elsif ($metaEventStr =~ /que point: (.*)/) {	# que point
		my $text = $1;
		$data[1] = 0x07;
		push @data, ConvertToVariableVal(length($text));
		my $val = pack("a*", $text);
		push @data, unpack("C*", $val);
	} elsif ($metaEventStr =~ /program: (.*)/) {	# program
		my $text = $1;
		$data[1] = 0x08;
		push @data, ConvertToVariableVal(length($text));
		my $val = pack("a*", $text);
		push @data, unpack("C*", $val);
	} elsif ($metaEventStr =~ /device: (.*)/) {	# device
		my $text = $1;
		$data[1] = 0x09;
		push @data, ConvertToVariableVal(length($text));
		my $val = pack("a*", $text);
		push @data, unpack("C*", $val);
	} elsif ($metaEventStr =~ /MIDI channel prefix \((\d+)\)/) {	# MIDI channel prefix
		my $val = eval($1);
		$data[1] = 0x20;
		push @data, ConvertToVariableVal(1);
		push @data, $val;
	} elsif ($metaEventStr =~ /output port \((\d+)\)/) {	# output port
		my $val = eval($1);
		$data[1] = 0x21;
		push @data, ConvertToVariableVal(1);
		push @data, $val;
	} elsif ($metaEventStr =~ /track end/) {	# track end
		$data[1] = 0x2f;
		push @data, ConvertToVariableVal(0);
	} elsif ($metaEventStr =~ /tempo \((\d+) usec \/ BPM.+\)/) {	# tempo
		my $val = eval($1);
		$data[1] = 0x51;
		push @data, ConvertToVariableVal(3);
		push @data, ($val >> 16) & 0xff;
		push @data, ($val >>  8) & 0xff;
		push @data, ($val >>  0) & 0xff;
	} elsif ($metaEventStr =~ /SMPTE offset/) {	# smpte offset
		$data[1] = 0x51;
		push @data, ConvertToVariableVal(5);
		push @data, (0, 0, 0, 0, 0);
	} elsif ($metaEventStr =~ /beat (\d+)\/(\d+) metro_tick\((\d+)\) quater_note\((\d+)\/32\)/) {	# beat in 4 bytes
		my $beat_num = eval($1);
		my $beat_den = eval($2);
		my $metro_tick = eval($3);
		my $quater_note = eval($4);
		my $beat_den_shift = 0;
		while ($beat_den > 1) { $beat_den >>= 1; $beat_den_shift++; }
		$data[1] = 0x58;
		push @data, ConvertToVariableVal(4);
		push @data, $beat_num;
		push @data, $beat_den_shift;
		push @data, $metro_tick;
		push @data, $quater_note;
	} elsif ($metaEventStr =~ /beat (\d+)\/(\d+)/) {	# beat in 2 bytes
		my $beat_num = eval($1);
		my $beat_den = eval($2);
		my $beat_den_shift = 0;
		while ($beat_den > 1) { $beat_den >>= 1; $beat_den_shift++; }
		$data[1] = 0x58;
		push @data, ConvertToVariableVal(2);
		push @data, $beat_num;
		push @data, $beat_den_shift;
	} elsif ($metaEventStr =~ /key signature (.+) (.+)/) {	# key signature
		my ($key, $MajMin) = ($1, $2);
		my @key_str_tbl = ("Fes", "Ces", "Ges", "Des", "As", "Es", "B", "F", "C", "G", "D", "A", "E", "H", "Fis", "Cis", "Gis", "Dis", "Ais", "Eis",);
		my $key_val = -8;
		foreach my $key_str (@key_str_tbl) {
			if ($key_str eq $key) { last; }
			$key_val++;
		}
		$data[1] = 0x59;
		push @data, ConvertToVariableVal(2);
		push @data, ($key_val < 0) ? ($key_val + 256) : $key_val;
		push @data, ($MajMin eq "mol") ? 1 : 0;
	} elsif ($metaEventStr =~ /sequencer meta (.+)/) {	# sequencer meta
		my $remain_str = $1;
		my @tmp_data;
		my $size = 0;
		while ($remain_str =~ / ?([0-9a-f][0-9a-f])( .+)?/) {
			push @tmp_data, eval("0x" . $1);
			$remain_str = $2;
			$size++;
		}

		$data[1] = 0x7f;
		push @data, ConvertToVariableVal($size);
		push @data, @tmp_data;
	}

	return @data;
}

sub ParseText
{
	my ($fh, $infoHashRef, $trackArrayRef) = @_;

	my $cur_track = 0;

	while (my $str = <$fh>) {
		if ($str =~ /format: (\d+)/) {
			$infoHashRef->{ format } = eval($1);
		} elsif ($str =~ /num_of_track: (\d+)/) {
			$infoHashRef->{ num_of_track } = eval($1);
		} elsif ($str =~ /TPQN: (\d+)/) {
			$infoHashRef->{ TPQN } = eval($1);
		} elsif ($str =~ /next track \((\d+)\) \.\.\./) {
			$cur_track++;
			if ($cur_track > $infoHashRef->{ num_of_track }) {
				print "track number is larger than specified num of tracks\n";
				exit 1;
			}
		} elsif ($str =~ /\s*(\d+)\s+\(.+:.+:.+\)(\( .+\'.+\".+\))?: ([! ])([89abcdef][0-9a-f]) (.+)/) {
			my $tickTime = eval($1);
			my $isRunningStatus = ($3 eq "!") ? 1 : 0;
			my @data;
			$data[0] = eval("0x" . $4);
			my $remain_str = $5;
			while ($remain_str =~ /^ ?([0-9a-f][0-9a-f])( .+)?/) {
				push @data, eval("0x" . $1);
				$remain_str = $2;
			}
			push @{ $trackArrayRef->[$cur_track - 1] }, [$tickTime, $isRunningStatus, @data];
		} elsif ($str =~ /\s*(\d+)\s+\(.+:.+:.+\)(\( .+\'.+\".+\))?: (.+)/) {
			my $tickTime = eval($1);
			my $isRunningStatus = 0;
			my $metaEventStr = $3;
			my @data = ParseMetaEvent($metaEventStr);
			push @{ $trackArrayRef->[$cur_track - 1] }, [$tickTime, $isRunningStatus, @data];
		}
	}
}



1;
