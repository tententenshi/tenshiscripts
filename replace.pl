#! /usr/bin/env perl

use strict;


sub usage {
	my $cmd = `basename $0`;
	chomp $cmd;
	print "replace description using regular expression\n";
	print "usage: $cmd \"PATTERN_BEFORE\" \"PATTERN_AFTER\" file1 [file2 ...]\n";
	print "	tips: double quotation '\"' should be preceded by triple backslashes '\\\\\\\"'\n";
	print "	      doller '\$' should be preceded by single backslashes '\\\$'\n";
	print "	      backslash itself '\\' should be preceded by triple backslashes '\\\\\\\\'\n";
	print "	      add single backslash to escape ()[]{}/|^+?* as same as describing in the perl script\n";
	print "	      add single backslash to escape ! not to parsed by bash\n";
	print "	      new lines are expressed as \"\\n\" (linux) \"\\r\\n\" (windows)\n";
	print "	      no particular escaping is needed for #&<>_~='%\n";
	exit 1;
}


#
# main
#
{
	my $PATTERN_BEFORE = shift;
	my $PATTERN_AFTER = '"' . shift . '"';	# add '"' preparing for eval(eval($PATTERN_AFTER))

	if ($#ARGV < 0) {
		usage();
	}

#	print "\n\nPATTERN_BEFORE: $PATTERN_BEFORE\n\n";
#	print "\n\nPATTERN_AFTER: $PATTERN_AFTER\n\n";

	while (my $file = shift) {
		open (IN, $file) or die "cannot open $file\n";
		my $text = do { local $/; <IN> };
		$text =~ s/$PATTERN_BEFORE/$PATTERN_AFTER/sgee;	# eval(eval($PATTERN_AFTER)) to interpret "$1"
		print $text;
	}
}

