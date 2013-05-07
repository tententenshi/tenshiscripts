#

#use Encode;

package HTML::Dump;

require HTML::Parser;
@ISA=qw(HTML::Parser);
 
 

sub declaration {
#    print "[declaration]\n";
#    print " text:$_[1]\n";
}

sub process {
#    print "[process]\n";
#    print " token0:$_[1]\n";
#    print " text:$_[2]\n";
}

sub comment {
#    print "[comment]\n";
#    print " tokens:$_[1]\n";
}

my $cur_line = 1;
my $nest = 0;
my @nest_line = ();
my @text_output = ();
my $tag_nest = '';
my $text_sum = '';
my $cur_href = '';

sub Print {
    my @Buf = @_;
    for (my $i = 0; $i < $nest; $i++) {
#	print " |";
    }
#    print @Buf;
}

sub IgnoreTag {
    my ($tag) = @_;
    if (($tag eq "meta") ||
	($tag eq "x-claris-window") ||
	($tag eq "x-claris-tagview") ||
	($tag eq "p") ||
	($tag eq "span") ||
	($tag eq "font") ||
	($tag eq "b") ||
	($tag eq "center") ||
	($tag eq "tbody")) {
	return 1;
    } else {
	return 0;
    }
}

sub IsUnaryTag {
    my ($tag) = @_;
    if (($tag eq "meta") ||
	($tag eq "hr") ||
	($tag eq "basefont") ||
	($tag eq "option") ||
	($tag eq "img")) {
	return 1;
    } else {
	return 0;
    }
}

sub IsBRTag {
    my ($tag) = @_;
    if (($tag eq "br")) {
	return 1;
    } else {
	return 0;
    }
}

sub checkHierarchy {
    my ($self, $cur_tag) = @_;
    my @hierarchy = (
		     ["td", "tr"],
		     ["tr", "table"],
		     );
    foreach my $tmp (@hierarchy) {
	if ($cur_tag eq $tmp->[0]) {
	    my $parent = $tmp->[1];
	    while (!($tag_nest =~ /$parent$/)) {
#		print "tag is irregular $tag_nest($cur_tag)\n";
		$tag_nest =~ /( \| )(\S+?)$/;
		my $last_tag = $2;
		$self->end($last_tag);
	    }
	}
    }
}

sub start {
    my ($self,$tagname,$attr,$attrseq,$text) = @_;
    my ($k,$i);

    if (IgnoreTag($tagname) || IsUnaryTag($tagname)) {
	return;
    }
    if (IsBRTag($tagname)) {
	$text_sum .= "<br/>";
	return;
    }

    Print "[start] $tagname\n";

    $self->checkHierarchy($tagname);

    if ($nest > 0) {
	$tag_nest .= " | ";
    }
    $tag_nest .= $tagname;
    $nest++;

    if ($tagname eq 'td') {
	$text_output[$cur_line] = "$tag_nest: ";
	$nest_line[$nest] = $cur_line;
	$text_sum = '';
	$cur_href = '';
	$cur_line++;
    }
    if (($tagname eq 'table') || ($tagname eq 'tr')) {
	$text_output[$cur_line] = "$tag_nest ^\n";
	$cur_line++;
    }
    if ($tagname eq 'a') {
	$cur_href = $$attr{ 'href' };
    }

    foreach $k ( keys %$attr ){
	Print "-attr:$k = $$attr{$k}\n";
    }
#    for($i = 0 ; $i < @$attrseq ;$i++){
#	Print "-attrseq[$i] = $$attrseq[$i]\n";
#    }
}

sub end {
    my ($self,$tagname,$text) = @_;

    if (IgnoreTag($tagname)) {
	return;
    }

    if ($tagname eq 'td') {
#	print "$tag_nest: $text_sum ($cur_href)\n";
	$text_output[$nest_line[$nest]] .= "$text_sum \\($cur_href\\)\n";
	$text_sum = '';
	$cur_href = '';
    }
    if (($tagname eq 'table') || ($tagname eq 'tr')) {
	while (!($tag_nest =~ /$tagname$/)) {
#	    print "tag is irregular $tag_nest($cur_tag)\n";
	    $tag_nest =~ /( \| )(\S+?)$/;
	    my $last_tag = $2;
	    $self->end($last_tag);
	}

	$text_output[$cur_line] = "$tag_nest v\n";
	$cur_line++;
    }
    if ($tagname eq 'html') {
	foreach $str (@text_output) {
	    print $str;
	}
    }

    $tag_nest =~ s/( \| )?$tagname$//;
    $nest--;
    Print "[end] $tagname\n";
}

sub text {
    my ($self,$text,$is_cdata) = @_;
    $text =~ s/\n//g;
    $text =~ s/\r//g;
    if (($text =~ /^[\sÅ@]*$/) || ($text =~ /^\*$/) || ($text =~ /^\.$/)) {
	return;
    } else {
	Print "[text] $text\n";
	Print " is_cdata:TRUE\n" if ($is_cdata);

	$text_sum .= $text;
    }
}

1;
 __END__ 
