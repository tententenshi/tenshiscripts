#!/usr/bin/perl

use FindBin;
use strict;

require $FindBin::Bin . "/smf_util.pm";

my $file = shift;
my %info;
my @track;

&smf_util::ReadAndParse($file, \%info, \@track);
&smf_util::ShowAllEvent(\%info, \@track);
