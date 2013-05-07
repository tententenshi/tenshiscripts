
use Math::Complex;
use strict;

my $Fs = 44100;
my $pi = 4 * atan2(1,1);
sub tan { sin($_[0]) / cos($_[0]); }
sub w { return tan($pi * $_[0] / $Fs); }
sub z {
	my ($x) = @_;
	my $i = cplx(0, 1);
	return exp($i * 2 * $pi * $x / $Fs);
}
sub dB2Gain { return 10.0**($_[0] / 20.0); }


sub SetFs {
	$Fs = $_[0];
}

sub H_FA {
	my ($z, $a0, $a1, $b1) = @_;
	return ($a0 + $a1 * $z**-1) / (1 - $b1 * $z**-1);
}

sub H_EA {
	my ($z, $a0, $a1, $a2, $b1, $b2) = @_;
	return ($a0 + $a1 * $z**-1 + $a2 * $z**-2) / (1 - $b1 * $z**-1 - $b2 * $z**-2);
}

sub apf {
	my ($x, $f) = @_;

	my $k = (1 - w($f)) / (1 + w($f));
	return H_FA(z($x), $k, -1, $k);
}

sub lpf {
	my ($x, $f) = @_;

	my $a = w($f) / (1 + w($f));
	my $b = (1 - w($f)) / (1 + w($f));
	return H_FA(z($x), $a, $a, $b);
}

sub hpf {
	my ($x, $f) = @_;

	my $a = 1 / (1 + w($f));
	my $b = (1 - w($f)) / (1 + w($f));
	return H_FA(z($x), $a, -$a, $b);
}

sub lsv_new {
	my ($x, $f, $g) = @_;

	my $pole = ($g < 0) ? w($f) * dB2Gain(-$g) : w($f);
	my $zero = ($g < 0) ? w($f) : w($f) * dB2Gain($g);

	my $a0 = (1.0 + $zero) / (1.0 + $pole);
	my $a1 = -1.0 * (1.0 - $zero) / (1.0 + $pole);
	my $b1 = (1.0 - $pole) / (1.0 + $pole);
	return H_FA(z($x), $a0, $a1, $b1);
}

sub lsv_trad {
	my ($x, $f, $g) = @_;

	my $pole = ($g < 0) ? w($f) : w($f) * dB2Gain(-$g);
	my $zero = ($g < 0) ? w($f) * dB2Gain($g) : w($f);

	my $a0 = (1.0 + $zero) / (1.0 + $pole);
	my $a1 = -1.0 * (1.0 - $zero) / (1.0 + $pole);
	my $b1 = (1.0 - $pole) / (1.0 + $pole);
	return H_FA(z($x), $a0, $a1, $b1);
}

sub hsv_new {
	my ($x, $f, $g) = @_;

	my $pole = ($g < 0) ? w($f) * dB2Gain($g) : w($f);
	my $zero = ($g < 0) ? w($f) : w($f) * dB2Gain(-$g);

	my $a0 = (1.0 + $zero) / (1.0 + $pole) * dB2Gain($g);
	my $a1 = -1.0 * (1.0 - $zero) / (1.0 + $pole) * dB2Gain($g);
	my $b1 = (1.0 - $pole) / (1.0 + $pole);
	return H_FA(z($x), $a0, $a1, $b1);
}

sub hsv_trad {
	my ($x, $f, $g) = @_;

	my $pole = ($g < 0) ? w($f) : w($f) * dB2Gain($g);
	my $zero = ($g < 0) ? w($f) * dB2Gain(-$g) : w($f);

	my $a0 = (1.0 + $zero) / (1.0 + $pole) * dB2Gain($g);
	my $a1 = -1.0 * (1.0 - $zero) / (1.0 + $pole) * dB2Gain($g);
	my $b1 = (1.0 - $pole) / (1.0 + $pole);
	return H_FA(z($x), $a0, $a1, $b1);
}

sub prew_q {
	my ($f, $q) = @_;
	return $q * $f * $pi / ($Fs * tan($pi * $f / $Fs));
}

sub lpf2 {
	my ($x, $f, $q) = @_;

	$q = prew_q($f, $q);

	my $a0 =     w($f)**2                 / (1. + w($f) / $q + w($f)**2);
	my $a1 = 2 * w($f)**2                 / (1. + w($f) / $q + w($f)**2);
	my $a2 =     w($f)**2                 / (1. + w($f) / $q + w($f)**2);
	my $b1 = -(2. * (w($f)**2 - 1.))      / (1. + w($f) / $q + w($f)**2);
	my $b2 = -(1 - w($f) / $q + w($f)**2) / (1. + w($f) / $q + w($f)**2);
	return H_EA(z($x), $a0, $a1, $a2, $b1, $b2);
}

sub hpf2 {
	my ($x, $f, $q) = @_;

	$q = prew_q($f, $q);

	my $a0 =  1                           / (1. + w($f) / $q + w($f)**2);
	my $a1 = -2                           / (1. + w($f) / $q + w($f)**2);
	my $a2 =  1                           / (1. + w($f) / $q + w($f)**2);
	my $b1 = -(2. * (w($f)**2 - 1.))      / (1. + w($f) / $q + w($f)**2);
	my $b2 = -(1 - w($f) / $q + w($f)**2) / (1. + w($f) / $q + w($f)**2);
	return H_EA(z($x), $a0, $a1, $a2, $b1, $b2);
}

sub bpf {
	my ($x, $f, $q) = @_;

	$q = prew_q($f, $q);

	my $a0 =  (w($f) / $q)                / (1. + w($f) / $q + w($f)**2);
	my $a1 = 0;
	my $a2 = -(w($f) / $q)                / (1. + w($f) / $q + w($f)**2);
	my $b1 = -(2. * (w($f)**2 - 1.))      / (1. + w($f) / $q + w($f)**2);
	my $b2 = -(1 - w($f) / $q + w($f)**2) / (1. + w($f) / $q + w($f)**2);
	return H_EA(z($x), $a0, $a1, $a2, $b1, $b2);
}

sub pkg {
	my ($x, $f, $g, $q) = @_;

	$q = ($g < 0) ? ($q * dB2Gain($g)) : $q;
	return 1 + (dB2Gain($g) - 1) * bpf($x, $f, $q);
}

sub lsv2 {
	my ($x, $f, $g, $q) = @_;

	my $h = dB2Gain($g);
	if ($g >= 0) {
		my $a0 = (1 + sqrt($h) * w($f) / $q + $h * w($f)**2) / (1. + w($f) / $q + w($f)**2);
		my $a1 = 2 * ($h * w($f)**2 - 1)                     / (1. + w($f) / $q + w($f)**2);
		my $a2 = (1 - sqrt($h) * w($f) / $q + $h * w($f)**2) / (1. + w($f) / $q + w($f)**2);
		my $b1 = -(2. * (w($f)**2 - 1.))                     / (1. + w($f) / $q + w($f)**2);
		my $b2 = -(1 - w($f) / $q + w($f)**2)                / (1. + w($f) / $q + w($f)**2);
		return H_EA(z($x), $a0, $a1, $a2, $b1, $b2);
	} else {
		my $a0 = $h * (1 + w($f) / $q + w($f)**2)         / ($h + sqrt($h) * w($f) / $q + w($f)**2);
		my $a1 = $h * 2 * (w($f)**2 - 1)                  / ($h + sqrt($h) * w($f) / $q + w($f)**2);
		my $a2 = $h * (1 - w($f) / $q + w($f)**2)         / ($h + sqrt($h) * w($f) / $q + w($f)**2);
		my $b1 = -2 * (w($f)**2 - $h)                     / ($h + sqrt($h) * w($f) / $q + w($f)**2);
		my $b2 = -($h - sqrt($h) * w($f) / $q + w($f)**2) / ($h + sqrt($h) * w($f) / $q + w($f)**2);
		return H_EA(z($x), $a0, $a1, $a2, $b1, $b2);
	}
}

sub hsv2 {
	my ($x, $f, $g, $q) = @_;

	my $h = dB2Gain($g);
	if ($g >= 0) {
		my $a0 = ($h + sqrt($h) * w($f) / $q + w($f)**2) / (1. + w($f) / $q + w($f)**2);
		my $a1 = 2 * (w($f)**2 - $h)                     / (1. + w($f) / $q + w($f)**2);
		my $a2 = ($h - sqrt($h) * w($f) / $q + w($f)**2) / (1. + w($f) / $q + w($f)**2);
		my $b1 = -(2. * (w($f)**2 - 1.))                 / (1. + w($f) / $q + w($f)**2);
		my $b2 = -(1 - w($f) / $q + w($f)**2)            / (1. + w($f) / $q + w($f)**2);
		return H_EA(z($x), $a0, $a1, $a2, $b1, $b2);
	} else {
		my $a0 = $h * (1 + w($f) / $q + w($f)**2)             / (1 + sqrt($h) * w($f) / $q + $h * w($f)**2);
		my $a1 = $h * 2 * (w($f)**2 - 1)                      / (1 + sqrt($h) * w($f) / $q + $h * w($f)**2);
		my $a2 = $h * (1 - w($f) / $q + w($f)**2)             / (1 + sqrt($h) * w($f) / $q + $h * w($f)**2);
		my $b1 = -2 * ($h * w($f)**2 - 1)                     / (1 + sqrt($h) * w($f) / $q + $h * w($f)**2);
		my $b2 = -(1 - sqrt($h) * w($f) / $q + $h * w($f)**2) / (1 + sqrt($h) * w($f) / $q + $h * w($f)**2);
		return H_EA(z($x), $a0, $a1, $a2, $b1, $b2);
	}
}

1;
