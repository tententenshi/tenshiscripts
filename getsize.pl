#!/usr/bin/perl
###########################################################
#
#       Display each section size in object files.
#               (Using "nm" command.)
#
#                               2006.5.09 kurahasi
#
###########################################################

if ($ARGV[0] eq "-s") {
    $mode = 1;
    $file = $ARGV[1];
} elsif ($ARGV[0] eq "-d") {
    $mode = 2;
    $file = $ARGV[1];
} elsif ($ARGV[0] ne "") {
    $mode = 0;
    $file = $ARGV[0];
} else {
    print "\n";
    print "Usage: getsize.pl [-sd] file(*.o *.a, *.lib)\n";
    print "  -s: Display each symbol.\n";
    print "  -d: Display each directory.\n";
    print "\n";
    print " Section Information:\n";
    print "  B:	bss: uninitialized data section\n";
    print "  C:	common: uninitialized data\n";
    print "  D:	initialized data section\n";
    print "  G:	initialized data section for small objects\n";
#   print "  I	indirect reference to another symbol\n";
#   print "  N	debugging symbol\n";
    print "  R:	read only data section\n";
    print "  S:	uninitialized data section for small objects\n";
    print "  T:	text: text (code) section\n";
#   print "  U: undefined symbol\n";
    print "  V:	weak object\n";
    print "  W: weak symbol\n";
    exit;
}

$tmp = "getsize.tmp";

system("rm -f $tmp");
system("nm -A --size-sort -r $file > $tmp");

#Analize($tmp, "A");	# absolute
 Analize($tmp, "B");	# bss: uninitialized data section
 Analize($tmp, "C");	# common: uninitialized data
 Analize($tmp, "D");	# initialized data section
 Analize($tmp, "G");	# initialized data section for small objects
#Analize($tmp, "I");	# indirect reference to another symbol
#Analize($tmp, "N");	# debugging symbol
 Analize($tmp, "R");	# read only data section
 Analize($tmp, "S");	# uninitialized data section for small objects
 Analize($tmp, "T");	# text: text (code) section
#Analize($tmp, "U");	# undefined symbol
 Analize($tmp, "V");	# weak object
 Analize($tmp, "W");	# weak symbol

system("rm -f $tmp");

exit;

#===============================================
# Analize Function
#===============================================
sub Analize {

    $name = $_[0];
    $section = $_[1];

    $prev_file = 0;
    $prev_dir = 0;
    $sum = 0;
    $line = 0;

    open(fp, $name) || die "cannot open $name";

    while(<fp>)
    {
	if (/^(.*):(.*):(.*) ([A-Za-z]) (.*)$/) {
	    $s = $4;
	    $s =~ tr/a-z/A-Z/;
	    if ($s eq $section) {
		$dir = $1;
		$file = $2;
		$size = $3;
		$symbol = $5;
		calc();
	    }
	} elsif (/^(.*):(.*) ([A-Za-z]) (.*)$/) {
	    $s = $3;
	    $s =~ tr/a-z/A-Z/;
	    if ($s eq $section) {
		$dir = "";
		$file = $1;
		$size = $2;
		$symbol = $4;
		calc();
	    }
	} else {
	    print "Invalid Line";
	}
    }
    DispSize();
    close(fp);
}

sub calc {

    if ($line == 0) {
	$prev_dir = $dir;
	$prev_file = $file;
    }
    $line++;

    if ($mode == 2) {
	if ($dir ne "") {
	    if ($prev_dir eq $dir) {
		$sum += hex($size);
	    } else {
		DispSize();
		$sum = hex($size);
	    }
	} else {
	    if ($prev_file eq $file) {
		$sum += hex($size);
	    } else {
		DispSize();
		$sum = hex($size);
	    }
	}
    } else {
	if ($prev_file eq $file) {
	    DispSymbol();
	    $sum += hex($size);
	} else {
	    DispSize();
	    $sum = hex($size);
	    $prev_dir = $dir;
	    $prev_file = $file;
	    DispSymbol();
	}
    }	

    $prev_dir = $dir;
    $prev_file = $file;
}

sub DispSymbol {
    if ($mode == 1) {
	printf("%s\t0x%x\t%s:%s:%s\n", $section, hex($size), $prev_dir, $prev_file, $symbol);
    }
}

sub DispSize {
    if ($line > 0) {
	if ($mode == 1) {
	    printf("------------------------------------------------------------------------------\n");
	    printf("%s\t0x%x\t%s:%s\n", $section, $sum, $prev_dir, $prev_file);
	    printf("\n");
	} elsif ($mode == 2) {
	    if ($dir ne "") {
		printf("%s\t0x%x\t%s:%s\n", $section, $sum, $prev_dir);
	    } else {
		printf("%s\t0x%x\t%s:%s\n", $section, $sum, $prev_file);
	    }
	} else {
	    printf("%s\t0x%x\t%s:%s\n", $section, $sum, $prev_dir, $prev_file);
	}
    }
}
