use strict;


sub round {
	my($val) = @_;
	if ($val >= 0) { $val += 0.5; }
	else { $val -= 0.5; }
	return int($val);
}

sub my_hex_float {
	my ($str)=@_;
	if ($str =~ /([+-]?0[xX][0-9a-fA-F]+)\.([0-9a-fA-F]*)?[pP]([+-]?\d+)/) {
		my ($int_val, $frac_val, $shift) = ($1, $2, $3);
		my $frac_digit = length($frac_val);
		$shift -= $frac_digit * 4;
		my $shift_val = 2**$shift;
		my $val = eval($int_val . $frac_val) * $shift_val;
		return $val;
	} else {
		return 0;
	}
}

sub hex2dec {
	my ($val) = @_;

	while ($val =~ /(.*)([+-]?0[xX][0-9a-fA-F]+)(\.[0-9a-fA-F]*)?([pP][+-]?\d+)(.*)/) {
		my ($pre, $hex_float_val, $post) = ($1, $2 . $3 . $4, $5);
		$val = $pre . my_hex_float($hex_float_val) . $post;
	}

	while ($val =~ /^(.*)(0[Xx][\da-fA-F]+)(.*)$/) {
		my ($pre, $hex, $post) = ($1, $2, $3);
		my $VAL1_0;
		my $VAL_POS_MAX;
		if ($hex =~ /0[Xx][\da-fA-F]{6}/) {			# CoefData
			$VAL1_0 = 0x00800000;
			$VAL_POS_MAX = 0x80000000;
		} else {
			$VAL1_0 = 0x8000;
			$VAL_POS_MAX = 0x8000;
		}
		my $dec;
		if (eval($hex) >= $VAL_POS_MAX) {
			$dec = (eval($hex) - $VAL_POS_MAX * 2) / $VAL1_0;
		} else {
			$dec = eval($hex) / $VAL1_0;
		}
		$val = $pre . $dec . $post;
	}
	$val =~ s/<</*2**/g;
	$val =~ s/>>/*2**-/g;
	return eval($val);
}

sub tan { sin($_[0]) / cos($_[0]); }
sub acos { POSIX::acos($_[0]); }


sub ParseArgument
{
	my ($ARGV_REF) = @_;

	my @input;
	my %result;

	foreach my $str (@$ARGV_REF) {
		if ($str =~ /(\w+)\s*=\s*(\w+)/) {
			$result{ "option" }->{ $1 } = $2;
		} else {
			my @tmp_array = split(/,/, $str);
			push @input, @tmp_array;
		}
	}

	foreach my $str (@input) {
		my $val = ($str =~ /0[Xx]/) ? hex2dec($str) : eval($str);
		push @{ $result{ "argument" } }, $val;
	}

	return %result;
}


1;
