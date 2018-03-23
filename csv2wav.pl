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
			$dat =~ s/,\s+/,/g;
			$dat =~ s/\s+/,/g;
			my @dat_array = split(/,/, $dat);
			foreach my $tmpDat (@dat_array) {
				if ($infoHash{ "FORMAT_ID" } == 1) {
#					$tmpDat = eval($tmpDat) / $unity_val;
					$tmpDat = eval($tmpDat);
				} else {
					$tmpDat = eval($tmpDat);
				}
			}
			$#dat_array = $infoHash{ "NUM_OF_CH" } - 1;	# adjust array size
			if ($#dat_array >= 0) {
				wav_util::WriteWavData($fhOut, \%infoHash, \@dat_array);
				$wav_size += $infoHash{ "BLOCK_SIZE" };
			}
		}
	}

	close(IN);
	close($fhOut);

	wav_util::MaintainWavHeader($out_filename, \%infoHash, $wav_size);
}
