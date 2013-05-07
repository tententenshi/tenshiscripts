#!/usr/bin/perl

use FindBin;

require $FindBin::Bin . "/smf_util.pm";

$file = shift;

&smf_util::ReadAndParse($file, \%info, \@track);
&smf_util::ShowAllEvent(\%info, \@track);
