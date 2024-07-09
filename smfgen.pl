#!/usr/bin/env perl

use FindBin;
use strict;

require $FindBin::RealBin . "/smf_util.pm";

sub usage {
	my $cmd = `basename $0`;
	chomp $cmd;
	print "generate SMF from text\n";
	print "usage: text_file.txt smf_file.mid\n";
}


#
# main
#
{
	if ($#ARGV < 1) {
		usage();
		exit;
	}

	my $file_in = shift;
	my $file_out = shift;

	my $fh;
	open ($fh, "$file_in") or die "cannot open $file_in\n";

	my %info;
	my @track;

	&smf_util::ParseText($fh, \%info, \@track);
	&smf_util::Write($file_out, \%info, \@track);
}
