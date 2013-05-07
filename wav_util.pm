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
		print($fh "pad ");
		my $out = pack "V", ($pad_size); # long by little endian
		print($fh $out);
		for (my $i = 0; $i < $pad_size; $i++) {
			my $zero = pack "c", 0;
			print $fh $zero;
		}
	}
}

sub MaintainWavHeader {
	my ($outFileName, $infoHashRef, $dataSize) = @_;

	my $fh;
	open ($fh, "+< $outFileName") or die "cannot open $outFileName!\n";
	binmode $fh;

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


sub ReadWavData {
	my ($fh, $infoHashRef) = @_;

	my $buf;
	read $fh, $buf, $$infoHashRef{ "BLOCK_SIZE" };	# wav body

	my @readData;
	if ($$infoHashRef{ "NUM_OF_CH" } == 1) {
		if ($$infoHashRef{ "BIT_LENGTH" } == 8) {
			my $data = unpack "c", $buf;
			push @readData, $data;
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 16) {
			my $data = unpack "v", $buf;
			if ($data & 0x8000) { $data -= 0x10000; }
			push @readData, $data;
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 24) {
			my @tmp = unpack "C3", $buf;
			my $data;
			if ($tmp[2] & 0x80) { # negative value
				$data = (0xff << 24) | ($tmp[2] << 16) | ($tmp[1] << 8) | $tmp[0];
			} else {
				$data = (0x00 << 24) | ($tmp[2] << 16) | ($tmp[1] << 8) | $tmp[0];
			}
			my $tmpbin = pack "L", $data;
			$data = unpack "l", $tmpbin;
			push @readData, $data;
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 32) {
			if ($$infoHashRef{ "FORMAT_ID" } == 3) {
				my $tmpdata = unpack "V", $buf;
				$tmpdata = pack "L", $tmpdata;
				my $data = unpack "f", $tmpdata;
				push @readData, $data;
			} else {
				my $data = unpack "V", $buf;
				push @readData, $data;
			}
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 64) {
			if ($$infoHashRef{ "FORMAT_ID" } == 3) {
				my @tmpdata = unpack "V2", $buf;
				my $tmpdata = pack "L2", @tmpdata;
				my $data = unpack "d", $tmpdata;
				push @readData, $data;
			}
		}
	} else {
		if ($$infoHashRef{ "BIT_LENGTH" } == 8) {
			my ($dataL, $dataR) = unpack "c2", $buf;
			push @readData, ($dataL, $dataR);
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 16) {
			my ($dataL, $dataR) = unpack "v2", $buf;
			if ($dataL & 0x8000) { $dataL -= 0x10000; }
			if ($dataR & 0x8000) { $dataR -= 0x10000; }
			push @readData, ($dataL, $dataR);
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 24) {
			my @tmp = unpack "C6", $buf;

			my $dataL = ($tmp[2] << 16) | ($tmp[1] << 8) | $tmp[0];
			my $dataR = ($tmp[5] << 16) | ($tmp[4] << 8) | $tmp[3];
			if ($tmp[2] & 0x80) { # negative value
				$dataL |= (0xff << 24);
			}
			if ($tmp[5] & 0x80) { # negative value
				$dataR |= (0xff << 24);
			}
			my $tmpbin = pack "L", $dataL;
			$dataL = unpack "l", $tmpbin;
			my $tmpbin = pack "L", $dataR;
			$dataR = unpack "l", $tmpbin;
			push @readData, ($dataL, $dataR);
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 32) {
			if ($$infoHashRef{ "FORMAT_ID" } == 3) {
				my ($tmpL, $tmpR) = unpack "V2", $buf;
				$tmpL = pack "L", $tmpL;
				$tmpR = pack "L", $tmpR;
				my $dataL = unpack "f", $tmpL;
				my $dataR = unpack "f", $tmpR;
				push @readData, ($dataL, $dataR);
			} else {
				my ($dataL, $dataR) = unpack "V2", $buf;
				push @readData, ($dataL, $dataR);
			}
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 64) {
			if ($$infoHashRef{ "FORMAT_ID" } == 3) {
				my @tmpdata = unpack "V4", $buf;
				my $tmpdata = pack "L4", @tmpdata;
				my ($dataL, $dataR) = unpack "d2", $tmpdata;
				push @readData, ($dataL, $dataR);
			}
		}
	}

	return @readData;
}

sub WriteWavData {
	my ($fh, $infoHashRef, @dataArray) = @_;

	my $out;
	if ($$infoHashRef{ "NUM_OF_CH" } == 1) {
		if ($$infoHashRef{ "BIT_LENGTH" } == 8) {
			$out = pack "c", $dataArray[0];
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 16) {
			$out = pack "v", ($dataArray[0] & 0xffff);
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 24) {
			$out = pack "C3", (($dataArray[0] & 0xff), (($dataArray[0] >> 8) & 0xff), (($dataArray[0] >> 16) & 0xff));
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 32) {
			if ($$infoHashRef{ "FORMAT_ID" } == 3) {
				my $tmpdata = pack "f", $dataArray[0];
				$tmpdata = unpack "L", $tmpdata;
				$out = pack "V", $tmpdata;
			} else {
				$out = pack "V", $dataArray[0];
			}
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 64) {
			if ($$infoHashRef{ "FORMAT_ID" } == 3) {
				my $tmpdata = pack "d", $dataArray[0];
				my @tmpdata = unpack "L2", $tmpdata;
				$out = pack "V2", @tmpdata;
			}
		}
	} else {
		if ($$infoHashRef{ "BIT_LENGTH" } == 8) {
			$out = pack "c2", ($dataArray[0], $dataArray[1]);
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 16) {
			$out = pack "v2", (($dataArray[0] & 0xffff), ($dataArray[1] & 0xffff));
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 24) {
			$out = pack "C6", (($dataArray[0] & 0xff), (($dataArray[0] >> 8) & 0xff), (($dataArray[0] >> 16) & 0xff),
							   ($dataArray[1] & 0xff), (($dataArray[1] >> 8) & 0xff), (($dataArray[1] >> 16) & 0xff));
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 32) {
			if ($$infoHashRef{ "FORMAT_ID" } == 3) {
				my $tmpdataL = pack "f", $dataArray[0];
				$tmpdataL = unpack "L", $tmpdataL;
				my $tmpdataR = pack "f", $dataArray[1];
				$tmpdataR = unpack "L", $tmpdataR;
				$out = pack "V2", ($tmpdataL, $tmpdataR);
			} else {
				$out = pack "V2", ($dataArray[0], $dataArray[1]);
			}
		} elsif ($$infoHashRef{ "BIT_LENGTH" } == 64) {
			if ($$infoHashRef{ "FORMAT_ID" } == 3) {
				my $tmpdata = pack "d2", ($dataArray[0], $dataArray[1]);
				my @tmpdata = unpack "L4", $tmpdata;
				$out = pack "V4", @tmpdata;
			}
		}
	}

	print ($fh $out);
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
