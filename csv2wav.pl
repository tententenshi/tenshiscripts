#! /usr/bin/env perl

use strict;
use FindBin;
use POSIX;

require $FindBin::RealBin . "/wav_util.pm";


sub usage {
	my $cmd = `basename $0`;
	chomp $cmd;
	print "usage: $cmd inFile.csv [FORMAT_ID=3] [FS=96000] [NUM_OF_CH=3] [BIT_LENGTH=32] [outWavFile]\n";
	exit 1;
}


#
# main
#
{
	my %infoHash;
	$infoHash{ "FORMAT_ID" } =  3;	# float
	$infoHash{ "NUM_OF_CH" } =  3;	# Number of Channels
	$infoHash{ "FS" } = 	    96000;	# Sampling Rate
	$infoHash{ "BIT_LENGTH" } = 32;
	$infoHash{ "BLOCK_SIZE" } = $infoHash{ "BIT_LENGTH" } / 8 * $infoHash{ "NUM_OF_CH" };
	$infoHash{ "DATA_RATE" } =  $infoHash{ "BLOCK_SIZE" } * $infoHash{ "FS" };

	my $in_filename = "";
	my $out_filename = "";

	while (my $str = shift) {
		if ($str =~ /.+\.csv$/) {
			$in_filename = $str;
		} elsif ($str =~ /FORMAT_ID=(.+)/) {
			$infoHash{ "FORMAT_ID" } = eval($1);
		} elsif ($str =~ /FS=(.+)/) {
			$infoHash{ "FS" } = eval($1);
		} elsif ($str =~ /NUM_OF_CH=(.+)/) {
			$infoHash{ "NUM_OF_CH" } = eval($1);
		} elsif ($str =~ /BIT_LENGTH=(.+)/) {
			$infoHash{ "BIT_LENGTH" } = eval($1);
		} else {
			$out_filename = $str;
		}
	}
	if ($in_filename eq "") {
		usage();
	}
	if ($out_filename eq "") {
		$out_filename = $in_filename;
		$out_filename =~ s/\.csv/.wav/;
	}

	open(IN, "$in_filename") or die "cannot open $in_filename!\n";

	my $fhOut;
	open($fhOut, "> $out_filename") or die "cannot open $out_filename!\n";
	wav_util::WriteWavHeader($fhOut);

	$infoHash{ "BLOCK_SIZE" } = $infoHash{ "BIT_LENGTH" } / 8 * $infoHash{ "NUM_OF_CH" };
	$infoHash{ "DATA_RATE" } =  $infoHash{ "BLOCK_SIZE" } * $infoHash{ "FS" };


	my $wav_size = 0;

	{
		my $unity_val = wav_util::GetUnityVal(\%infoHash);
		while (my $dat = <IN>) {
			if      (($infoHash{ "NUM_OF_CH" } == 3) && ($infoHash{ "FORMAT_ID" } == 3)) {	#3ch float
				if ($dat =~ /(-?\d*(\d|\.)[-\de]*)\s*?[,\s]\s*?(-?\d*(\d|\.)[-\de]*)\s*?[,\s]\s*?(-?\d*(\d|\.)[-\de]*)/) {
					my ($ch0, $ch1, $ch2) = (eval($1), eval($3), eval($5));
					wav_util::WriteWavData($fhOut, \%infoHash, [$ch0, $ch1, $ch2]);
#					print "3ch float val $ch0, $ch1, $ch2\n";
				}
			} elsif (($infoHash{ "NUM_OF_CH" } == 2) && ($infoHash{ "FORMAT_ID" } == 3)) {	#stereo float
				$dat =~ /(-?\d*(\d|\.)[-\de]*)\s*?[,\s]\s*?(-?\d*(\d|\.)[-\de]*)/;
				wav_util::WriteWavData($fhOut, \%infoHash, [eval($1), eval($3)]);
#				print "stereo float val $1/$2\n";
			} elsif (($infoHash{ "NUM_OF_CH" } == 1) && ($infoHash{ "FORMAT_ID" } == 3)) {	#mono float
				$dat =~ /(-?\d*(\d|\.)[-\de]*)/;
				wav_util::WriteWavData($fhOut, \%infoHash, [eval($1)]);
#				print "mono float val $1\n";
			} elsif ($infoHash{ "NUM_OF_CH" } == 2) {	#stereo
				$dat =~ /(-?0x[0-9a-f]+|-?\d+)\s*?[,\s]\s*(-?0x[0-9a-f]+|-?\d+)/i;
				wav_util::WriteWavData($fhOut, \%infoHash, [eval($1) / $unity_val, eval($2) / $unity_val]);
#				print "stereo decimal val $1/$2\n";
			} elsif ($infoHash{ "NUM_OF_CH" } == 1) {	#mono
				$dat =~ /(-?0x[0-9a-f]+|^-?\d+)/i;
				wav_util::WriteWavData($fhOut, \%infoHash, [eval($1) / $unity_val]);
#				print "mono decimal val $1\n";
			} else {
				print "skipped $dat";
				next;
			}
			$wav_size += $infoHash{ "BLOCK_SIZE" };
		}
	}

	close(IN);
	close($fhOut);

	wav_util::MaintainWavHeader($out_filename, \%infoHash, $wav_size);
}
