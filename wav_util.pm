#
# wav_util
#

use strict;
package wav_util;

my $WAV_ALIGN_SIZE = 0x1000;
my $USE_WAV_ALIGNMENT = 1;

# info hash
# FORMAT_ID (= 1)		# Linear PCM
# NUM_OF_CH (= 2)		# Number of Channels
# FS (= 44100)			# Sampling Rate
# BIT_LENGTH (= 16)
# BLOCK_SIZE (= BIT_LENGTH / 8 * NUM_OF_CH)
# DATA_RATE (= FS * BLOCK_SIZE)



sub ParseWavFormat {
	my ($fh, $size, $infoHashRef) = @_;
#	print "fmt size --> $size\n";

	my $buf;
	read $fh, $buf, 2;		# short by little endian
	my $FORMAT_ID = unpack "v", $buf;
	print "FORMAT_ID --> $FORMAT_ID\n";
	if (($FORMAT_ID != 1) && ($FORMAT_ID != 3)) {
		die "unknown format ($FORMAT_ID) used\n";
	}

	read $fh, $buf, 2;		# short by little endian
	my $NUM_OF_CH = unpack "v", $buf;
	print "Number of Ch is $NUM_OF_CH\n";

	read $fh, $buf, 4;		# long by little endian
	my $FS = unpack "V", $buf;
	print "Sample Rate is $FS\n";

	read $fh, $buf, 4;		# long by little endian
	my $DATA_RATE = unpack "V", $buf;
	print "Data Rate is $DATA_RATE\n";

	read $fh, $buf, 2;		# short by little endian
	my $BLOCK_SIZE = unpack "v", $buf;
	print "Block Size is $BLOCK_SIZE\n";

	read $fh, $buf, 2;		# short by little endian
	my $BIT_LENGTH = unpack "v", $buf;
	print "Bit Length is $BIT_LENGTH\n";

	if (($BIT_LENGTH > 64) || ($BIT_LENGTH % 8 != 0)) {
		die "not proper bit length\n";
	}
	if ($BLOCK_SIZE != $BIT_LENGTH / 8 * $NUM_OF_CH) {
		die "not proper block size\n";
	}
	if ($DATA_RATE != $FS * $BLOCK_SIZE) {
		die "not proper data rate\n";
	}

	if ($size - 16 > 0) {
		seek($fh, $size - 16, 1);
	}

	$$infoHashRef{ "FORMAT_ID" } =  $FORMAT_ID;
	$$infoHashRef{ "NUM_OF_CH" } =  $NUM_OF_CH;
	$$infoHashRef{ "FS" } = 	    $FS;
	$$infoHashRef{ "BIT_LENGTH" } = $BIT_LENGTH;
	$$infoHashRef{ "BLOCK_SIZE" } = $BLOCK_SIZE;
	$$infoHashRef{ "DATA_RATE" } =  $DATA_RATE;
}


sub ReadWavHeader {
	my ($fh, $infoHashRef, $preChunkBufRef, $postChunkBufRef) = @_;

	my $buf;
	read $fh, $buf, 4;	# RIFF
	my $str = unpack "a4", $buf;
	if ($str ne "RIFF") {
		die "not proper format (RIFF not found)\n";
	}

	read $fh, $buf, 4;	# size
	read $fh, $buf, 4;	# WAVE
	$str = unpack "a4", $buf;
	if ($str ne "WAVE") {
		die "not proper format (WAVE not found)\n";
	}

	my $dataPos;
	my $dataSize;

	while (read $fh, $buf, 4) {	# new chunk
		$str = unpack "a4", $buf;

		if ($str eq "fmt ") {	# fmt 
			read $fh, $buf, 4;	# size of fmt
			my $size = unpack "V", $buf;
			ParseWavFormat($fh, $size, $infoHashRef);
		} elsif ($str eq "data") {
			read $fh, $buf, 4;	# size of data
			$dataSize = unpack "V", $buf;
			$dataPos = tell($fh);
			read $fh, $buf, $dataSize;
			last;
		} else {			# skip unknown chunk
			print "$str chunk is found\n";
			read $fh, $buf, 4;	# size of data
			my $size = unpack "V", $buf;
			if ($str ne "pad ") {
				seek($fh, -8, 1);
				read $fh, $buf, $size + 8;
				if (!defined($dataPos)) {
					push @$preChunkBufRef, $buf;
				} else {
					push @$postChunkBufRef, $buf;
				}
			} else {
				read $fh, $buf, $size;
			}
		}
	}

	seek($fh, $dataPos, 0);
	return $dataSize;
}

sub WriteWavHeader {
	my ($fh, $preChunkBufRef) = @_;

	my $out;
	print($fh "RIFF");
	$out = pack "V", (0);			# long by little endian
	print($fh $out);
	print($fh "WAVE");

	my $fmt_chunk_size = 8 + 16;
	for (my $i = 0; $i < $fmt_chunk_size; $i++) {
		my $zero = pack "c", 0;
		print $fh $zero;
	}

	foreach my $buf (@$preChunkBufRef) {
		print $fh $buf;
	}

	if ($USE_WAV_ALIGNMENT) {
		my $cur_pos = tell($fh);
		my $optimized_data_start = (($cur_pos + ($WAV_ALIGN_SIZE - 1)) & (~($WAV_ALIGN_SIZE - 1))) - 8;
		my $pad_size = $optimized_data_start - $cur_pos - 8;
		if ($pad_size < 0) { $pad_size += $WAV_ALIGN_SIZE; }
		print($fh "pad ");
		$out = pack "V", ($pad_size); # long by little endian
		print($fh $out);
		for (my $i = 0; $i < $pad_size; $i++) {
			my $zero = pack "c", 0;
			print $fh $zero;
		}
	}

	print($fh "data");
	$out = pack "V", (0);			# long by little endian
	print($fh $out);
}

sub WriteWavTrailer {
	my ($fh, $postChunkBufRef) = @_;

	foreach my $buf (@$postChunkBufRef) {
		print $fh $buf;
	}

	if ($USE_WAV_ALIGNMENT) {
		my $cur_pos = tell($fh);
		my $optimized_file_end = (($cur_pos + ($WAV_ALIGN_SIZE - 1)) & (~($WAV_ALIGN_SIZE - 1))) - 8;
		my $pad_size = $optimized_file_end - $cur_pos;
		if ($pad_size < 0) { $pad_size += $WAV_ALIGN_SIZE; }
		print($fh "pad ");
		my $out = pack "V", ($pad_size); # long by little endian
		print($fh $out);
		for (my $i = 0; $i < $pad_size; $i++) {
			my $zero = pack "c", 0;
			print $fh $zero;
		}
	}
}

sub MaintainWavHash {
	my ($infoHashRef) = @_;

	if ($$infoHashRef{ "BLOCK_SIZE" } != $$infoHashRef{ "BIT_LENGTH" }  / 8 * $$infoHashRef{ "NUM_OF_CH" }) {
		$$infoHashRef{ "BLOCK_SIZE" } = $$infoHashRef{ "BIT_LENGTH" }  / 8 * $$infoHashRef{ "NUM_OF_CH" };
		print "correct BLOCK_SIZE to " . $$infoHashRef{ "BLOCK_SIZE" } . "\n";
	}
	if ($$infoHashRef{ "DATA_RATE" }  != $$infoHashRef{ "FS" } * $$infoHashRef{ "BLOCK_SIZE" }) {
		$$infoHashRef{ "DATA_RATE" }  = $$infoHashRef{ "FS" } * $$infoHashRef{ "BLOCK_SIZE" };
		print "correct DATA_RATE to " . $$infoHashRef{ "DATA_RATE" } . "\n";
	}
}

sub MaintainWavHeader {
	my ($outFileName, $infoHashRef, $dataSize) = @_;

	my $fh;
	open ($fh, "+< $outFileName") or die "cannot open $outFileName!\n";
	binmode $fh;

	MaintainWavHash($infoHashRef);

	seek($fh, 4, 0);
	my $totalSize = (stat($fh))[7];
	my $out = pack "V", ($totalSize - 8);			# long by little endian
	print($fh $out);

	seek($fh, 12, 0);
	print($fh "fmt ");
	$out = pack "V", (16);				# long by little endian
	print($fh $out);
	$out = pack "v", ($$infoHashRef{ "FORMAT_ID" });			# short by little endian
	print($fh $out);
	$out = pack "v", ($$infoHashRef{ "NUM_OF_CH" });			# short by little endian
	print($fh $out);
	$out = pack "V", ($$infoHashRef{ "FS" });				# long by little endian
	print($fh $out);
	$out = pack "V", ($$infoHashRef{ "DATA_RATE" });			# long by little endian
	print($fh $out);
	$out = pack "v", ($$infoHashRef{ "BLOCK_SIZE" });			# short by little endian
	print($fh $out);
	$out = pack "v", ($$infoHashRef{ "BIT_LENGTH" });			# short by little endian
	print($fh $out);

	# search data chunk
	my $buf;
	while (read $fh, $buf, 4) {	# new chunk
		my $str = unpack "a4", $buf;

		if ($str eq "data") {
			my $dataPos = tell($fh);
			seek($fh, $dataPos, 0);		# to fix suspicious operation on gota's cygwin
			$out = pack "V", ($dataSize);			# long by little endian
			print($fh $out);
			last;
		} else {			# skip other chunk
#			print "skipping $str chunk\n";
			read $fh, $buf, 4;	# size of data
			my $size = unpack "V", $buf;
			read $fh, $buf, $size;
		}
	}

	close($fh);
}


sub GetUnityVal {
	my ($infoHashRef) = @_;

	my $format = $$infoHashRef{ "FORMAT_ID" };
	my $bitLength = $$infoHashRef{ "BIT_LENGTH" };
	if ($format == 1) {
		if ($bitLength == 16) {
			return 1 << 15;
		} elsif ($bitLength == 24) {
			return 0x8000 * 0x10000;
		} elsif ($bitLength == 32) {
			return 0x8000 * 0x10000;
		}
	} elsif ($format == 3) {
		if ($bitLength == 32) {
			return 1.0;
		} elsif ($bitLength == 64) {
			return 1.0;
		}
	} else {
		return 0;
	}
}

sub ReadWavData {
	my ($fh, $infoHashRef, $samples) = @_;

	if (!defined($samples)) { $samples = 1; }

	my $NUM_OF_CH = $$infoHashRef{ "NUM_OF_CH" };
	my $BIT_LENGTH = $$infoHashRef{ "BIT_LENGTH" };

	if ($BIT_LENGTH == 16) {
		read $fh, my $buf, $BIT_LENGTH / 8 * $samples * $NUM_OF_CH;	# wav body
		my @dataArray = unpack "v*", $buf;	# unsigned 16bit little endian
		my $tmpbin = pack "S*", @dataArray;	# unsigned 16bit environment dependent
		@dataArray = unpack "s*", $tmpbin;	# signed 16bit environment dependent
		foreach my $data (@dataArray) { $data = $data / 0x8000; }
		return @dataArray;
	} elsif ($BIT_LENGTH == 24) {
		my @dataArray;
		for (my $i = 0; $i < $samples; $i++) {
			for (my $ch = 0; $ch < $NUM_OF_CH; $ch++) {
				read $fh, my $buf, $BIT_LENGTH / 8;	# wav body
				my @tmp = unpack "C3", $buf;
				my $data = ($tmp[2] << 24) | ($tmp[1] << 16) | ($tmp[0] << 8);
				my $tmpbin = pack "L", $data;	# unsigned 32bit environment dependent
				$data = unpack "l", $tmpbin;	# signed 32bit environment dependent
				push @dataArray, ($data / (0x8000 * 0x10000));
			}
		}
		return @dataArray;
	} elsif ($BIT_LENGTH == 32) {
		read $fh, my $buf, $BIT_LENGTH / 8 * $samples * $NUM_OF_CH;	# wav body
		if ($$infoHashRef{ "FORMAT_ID" } == 3) {
			my @dataArray = unpack "V*", $buf;	# unsigned 32bit little endian
			my $tmpbin = pack "L*", @dataArray;	# unsigned 32bit environment dependent
			@dataArray = unpack "f*", $tmpbin;	# float 32bit environment dependent
			return @dataArray;
		} else {
			my @dataArray = unpack "V*", $buf;	# unsigned 32bit little endian
			my $tmpbin = pack "L*", @dataArray;	# unsigned 32bit environment dependent
			@dataArray = unpack "l*", $tmpbin;	# signed 32bit environment dependent
			foreach my $data (@dataArray) { $data = $data / (0x8000 * 0x10000); }
			return @dataArray;
		}
	} elsif ($BIT_LENGTH == 64) {
		read $fh, my $buf, $BIT_LENGTH / 8 * $samples * $NUM_OF_CH;	# wav body
		if ($$infoHashRef{ "FORMAT_ID" } == 3) {
			my @tmpdata = unpack "V*", $buf;	# unsigned 32bit little endian
			my $tmpbin = pack "L*", @tmpdata;	# unsigned 32bit environment dependent
			my @dataArray = unpack "d*", $tmpbin;	# double 64bit environment dependent
			return @dataArray;
		}
	}
}

sub Bound {
	my ($min, $val, $max) =@_;
	return ($val < $min) ? $min : ($val > $max) ? $max : $val;
}

sub WriteWavData {
	my ($fh, $infoHashRef, $ref_dataArray, $samples) = @_;

	if (!defined($samples)) { $samples = 1; }
	my $NUM_OF_CH = $$infoHashRef{ "NUM_OF_CH" };
	my $BIT_LENGTH = $$infoHashRef{ "BIT_LENGTH" };

	if (!defined($$ref_dataArray[$samples * $NUM_OF_CH -1])) { $$ref_dataArray[$samples * $NUM_OF_CH -1] = 0; }
	if ($BIT_LENGTH == 16) {
		foreach my $data (@$ref_dataArray) { $data = Bound(-0x8000, $data * 0x8000, 0x7fff); }
		my $out = pack "v*", @$ref_dataArray;	# unsigned 16bit little endian
		print ($fh $out);
	} elsif ($BIT_LENGTH == 24) {
		my $pointer = 0;
		for (my $i = 0; $i < $samples; $i++) {
			for (my $ch = 0; $ch < $NUM_OF_CH; $ch++) {
				my $data = defined($$ref_dataArray[$pointer]) ? $$ref_dataArray[$pointer] : 0;	$pointer++;
				my $val = Bound(-0x80000000, $data * 0x8000 * 0x10000, 0x7fffffff);
				my $out = pack "C3", ((($val >> 8) & 0xff), (($val >> 16) & 0xff), (($val >> 24) & 0xff));
				print ($fh $out);
			}
		}
	} elsif ($BIT_LENGTH == 32) {
		if ($$infoHashRef{ "FORMAT_ID" } == 3) {
			my $tmpbin = pack "f*", @$ref_dataArray;	# float 32bit environment dependent
			my @tmpdata = unpack "L*", $tmpbin;	# unsigned 32bit environment dependent
			my $out = pack "V*", @tmpdata;		# unsigned 32bit little endian
			print ($fh $out);
		} else {
			foreach my $data (@$ref_dataArray) { $data = Bound(-0x80000000, $data * 0x8000 * 0x10000, 0x7fffffff); }
			my $out = pack "V*", @$ref_dataArray;	# unsigned 32bit little endian
			print ($fh $out);
		}
	} elsif ($BIT_LENGTH == 64) {
		if ($$infoHashRef{ "FORMAT_ID" } == 3) {
			my $tmpbin = pack "d*", @$ref_dataArray;	# double 64bit environment dependent
			my @tmpdata = unpack "L*", $tmpbin;	# unsigned 32bit environment dependent
			my $out = pack "V*", @tmpdata;	# unsigned 32bit little endian
			print ($fh $out);
		}
	}
}


#
# confirm system endian
#
sub ConfirmSystemEndian
{
	my $testVal = 0x12345678;
	my $binTestValBig = unpack("L", pack("N", $testVal));
	my $binTestValLittle = unpack("L", pack("V", $testVal));
	printf("L:0x%8x, N:0x%8x, V:0x%8x\n", $testVal, $binTestValBig, $binTestValLittle);
	if    ($testVal == $binTestValBig) { return "BIG"; }
	elsif ($testVal == $binTestValLittle) { return "LITTLE"; }
	else { return "ERROR"; }
}

#
# main sample
#
sub MainSample
{
	my $infile = shift;
	my $outfile = shift;

	my $fhIn, my $fhOut;
	open($fhIn, "$infile") or die "cannot open $infile!\n";
	open($fhOut, "> $outfile") or die "cannot open $outfile!\n";
	binmode $fhIn;
	binmode $fhOut;

	my %infoHash;
	my @preChunkBuf, my @postChunkBuf;
	my $dataSize = wav_util::ReadWavHeader($fhIn, \%infoHash, \@preChunkBuf, \@postChunkBuf);
	wav_util::WriteWavHeader($fhOut, \@preChunkBuf);

	my $remain = $dataSize;
	while($remain > 0) {
		my @dataArray = wav_util::ReadWavData($fhIn, \%infoHash);
		wav_util::WriteWavData($fhOut, \%infoHash, @dataArray);

		$remain -= $infoHash{ "BLOCK_SIZE" };
	}

	wav_util::WriteWavTrailer($fhOut, \@postChunkBuf);

	close ($fhIn);
	close ($fhOut);

	wav_util::MaintainWavHeader($outfile, \%infoHash, $dataSize);
}


1;
