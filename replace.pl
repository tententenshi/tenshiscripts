#! /usr/bin/env perl

use strict;


sub usage {
	my $cmd = `basename $0`;
	chomp $cmd;
	print "replace description using regular expression\n";
	print "usage: $cmd \"PATTERN_BEFORE\" \"PATTERN_AFTER\" file1 [file2 ...]\n";
	exit 1;
}


#
# main
#
{
	my $PATTERN_BEFORE = shift;
	my $PATTERN_AFTER = shift;
	$PATTERN_AFTER =~ s/\Q\a\E/\a/g;
	$PATTERN_AFTER =~ s/\Q\b\E/\b/g;
	$PATTERN_AFTER =~ s/\Q\n\E/\n/g;
	$PATTERN_AFTER =~ s/\Q\r\E/\r/g;
	$PATTERN_AFTER =~ s/\Q\f\E/\f/g;
	$PATTERN_AFTER =~ s/\Q\t\E/\t/g;
	$PATTERN_AFTER =~ s/\Q\e\E/\e/g;
	$PATTERN_AFTER =~ s/\Q\s\E/\s/g;

	$PATTERN_AFTER =~ s/\Q\{\E/\{/g;
	$PATTERN_AFTER =~ s/\Q\}\E/\}/g;
	$PATTERN_AFTER =~ s/\Q\[\E/\[/g;
	$PATTERN_AFTER =~ s/\Q\]\E/\]/g;
	$PATTERN_AFTER =~ s/\Q\(\E/\(/g;
	$PATTERN_AFTER =~ s/\Q\)\E/\)/g;
	$PATTERN_AFTER =~ s/\Q\^\E/\^/g;
	$PATTERN_AFTER =~ s/\Q\$\E/\$/g;
	$PATTERN_AFTER =~ s/\Q\.\E/\./g;
	$PATTERN_AFTER =~ s/\Q\|\E/\|/g;
	$PATTERN_AFTER =~ s/\Q\*\E/\*/g;
	$PATTERN_AFTER =~ s/\Q\+\E/\+/g;
	$PATTERN_AFTER =~ s/\Q\?\E/\?/g;
	$PATTERN_AFTER =~ s/\Q\\\E/\\/g;

#	print "\n\nPATTERN_BEFORE: $PATTERN_BEFORE\n\n";
#	print "\n\nPATTERN_AFTER: $PATTERN_AFTER\n\n";

	while (my $file = shift) {
		open (IN, $file) or die "cannot open $file\n";
		my $text = do { local $/; <IN> };
		$text =~ s/$PATTERN_BEFORE/$PATTERN_AFTER/sg;
		print $text;
	}
}

