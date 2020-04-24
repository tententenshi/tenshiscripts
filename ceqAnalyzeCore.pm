use strict;
use POSIX;
use Math::Complex;

my $ERROR_LEVEL = 1 / (1 << 19);




sub CeqFA_Analyze {
	my ($a0, $a1, $b1, $Fs, $result_ref) = @_;

	$$result_ref{ q_a } = 0;
	$$result_ref{ q_d } = 0;

	$$result_ref{ zero1 } = ($a0 != 0) ? -$a1 / $a0 : 0;
	$$result_ref{ pole1 } = $b1;

	if (abs($$result_ref{ pole1 }) >= 1) {
		$$result_ref{ type } = "ERROR";
		$$result_ref{ memo } = "pole is outof stable area!";
		return;
	}

	$$result_ref{ mag_dc } = (1 - $b1 != 0) ? ($a0 + $a1) / (1 - $b1) : "infinite";
	$$result_ref{ mag_nyquist } = (1 + $b1 != 0) ? ($a0 - $a1) / (1 + $b1) : "infinite";
	$$result_ref{ gain_dc }      = ($$result_ref{ mag_dc }      eq "infinite") ? "infinite" : ($$result_ref{ mag_dc }      != 0) ? 20 * log(abs($$result_ref{ mag_dc })) / log(10) : "-infinite";
	$$result_ref{ gain_nyquist } = ($$result_ref{ mag_nyquist } eq "infinite") ? "infinite" : ($$result_ref{ mag_nyquist } != 0) ? 20 * log(abs($$result_ref{ mag_nyquist })) / log(10) : "-infinite";

	if (abs($$result_ref{ zero1 }) > 1.0) {
		$$result_ref{ memo } = "****** not minimum phase filter (zero is out of unit circle) ******";
		$$result_ref{ zero1 } = 1 / $$result_ref{ zero1 };
	}

	my $w0 = (1 - $$result_ref{ pole1 }) / (1 + $$result_ref{ pole1 });
	$$result_ref{ freq_a } = $w0 * $Fs / pi();
	$$result_ref{ freq_d } = atan2($w0, 1) / pi() * $Fs;	# reverse prewarping

	if (($a0 == 0) || ($a1 == 0)) {
		if ($b1 >= 3 - 2 * sqrt(2)) {
			$$result_ref{ memo } = "filter without zero, calculate through Z-plane direct method";
			# reverse calculation from b1 = 2 - cos - sqrt((2 - cos)**2 - 1)
			$$result_ref{ freq_d } = acos(-(1 + $b1**2 - 4 * $b1) / (2 * $b1)) * $Fs / (2 * pi());
		}
	}

	if ($$result_ref{ mag_dc } < $ERROR_LEVEL) {
		$$result_ref{ type } = "HPF1";
	} elsif ($$result_ref{ mag_nyquist } < $ERROR_LEVEL) {
		$$result_ref{ type } = "LPF1";
	} elsif (abs($$result_ref{ mag_dc } - $$result_ref{ mag_nyquist }) < $ERROR_LEVEL) {
		$$result_ref{ type } = "THRU";
	} elsif (abs($$result_ref{ mag_dc } + $$result_ref{ mag_nyquist }) < $ERROR_LEVEL) {
		$$result_ref{ type } = "APF1";
	} elsif (abs($$result_ref{ mag_dc }) < abs($$result_ref{ mag_nyquist })) {
		$$result_ref{ type } = "HI_BOOST1";
		my $w0_svf = $w0 * abs($$result_ref{ mag_dc }) / abs($$result_ref{ mag_nyquist });
		$$result_ref{ freq1_d } = atan2($w0_svf, 1) / pi() * $Fs;
		$$result_ref{ freq2_d } = $$result_ref{ freq_d };
	} elsif (abs($$result_ref{ mag_dc }) > abs($$result_ref{ mag_nyquist })) {
		$$result_ref{ type } = "HI_CUT1";
		my $w0_svf = $w0 * abs($$result_ref{ mag_dc }) / abs($$result_ref{ mag_nyquist });
		$$result_ref{ freq2_d } = atan2($w0_svf, 1) / pi() * $Fs;
		$$result_ref{ freq1_d } = $$result_ref{ freq_d };
	} else {
		$$result_ref{ type } = "ERROR";
		$$result_ref{ memo } = "unknown filter type\n";
	}
}


# return value
# result
#	mag_dc:       magnitude at DC (0Hz)
#	mag_nyquist:  magnitude at Nyquist frequency (Fs / 2 Hz)
#	mag_cutoff:   magnitude at cutoff frequency (available for 2nd order filters)
#	mag_pkg:      magnitude as peaking filter at cutoff frequency (available for 2nd order filters)
#	gain_dc:      logscale (dB) magnitude at DC (0Hz)
#	gain_nyquist: logscale (dB) magnitude at Nyquist frequency (Fs / 2 Hz)
#	gain_cutoff:  logscale (dB) magnitude at cutoff frequency (available for 2nd order filters)
#	gina_pkg:     logscale (dB) magnitude as peaking filter at cutoff frequency (available for 2nd order filters)

#	freq_a:       cutoff frequency as analog filter
#	freq_d:       cutoff frequency as digital filter
#	freq1_d:      lower frequency of shelving filter as digital filter
#	freq2_d:      higher frequency of shelving filter as digital filter
#	q_a           Q as analog filter
#	q_d           Q as digital filter

#	pole1         1st pole
#	pole2         2nd pole (available for 2nd order filters)
#	zero1         1st zero
#	zero2         2nd zero (available for 2nd order filters)

#	type:         filter types
#					THRU, HPF1, LPF1, HI_BOOST1, HI_CUT1, APF1, HPF2, LPF2, BPF2, BEF2, HI_BOOST2, HI_CUT2, PKG2, APF2, COMPLEX, ERROR

#	memo:         memo

sub CeqAnalyzeCore {
	my ($a0, $a1, $a2, $b1, $b2, $Fs) = @_;

	my %result;

	#	print "plot 20*log10(abs(H_EA_N(z(x), $a0, $a1, $a2) / H_EA_D(z(x), $b1, $b2)))\n";
	#	print "plot sin(t), cos(t) notitle with lines, \\\nreal(pole1($b1,$b2)), imag(pole1($b1,$b2)), \\\nreal(pole2($b1,$b2)), imag(pole2($b1,$b2)), \\\n";
	#	print "real(zero1($a0,$a1,$a2)), imag(zero1($a0,$a1,$a2)), \\\nreal(zero2($a0,$a1,$a2)), imag(zero2($a0,$a1,$a2))\n";


	$result{ mag_dc }      = (1 - $b1 - $b2 != 0) ? (($a0 + $a1 + $a2) / (1 - $b1 - $b2)) : "infinite";
	$result{ mag_nyquist } = (1 + $b1 - $b2 != 0) ? ($a0 - $a1 + $a2) / (1 + $b1 - $b2) : "infinite";
	$result{ mag_cutoff }  = (1 + $b2 != 0) ? ($a0 - $a2) / (1 + $b2) : "infinite";
	$result{ gain_dc }      = ($result{ mag_dc }      eq "infinite") ? "infinite" : ($result{ mag_dc }      != 0) ? 20 * log(abs($result{ mag_dc })) / log(10) : "-infinite";
	$result{ gain_nyquist } = ($result{ mag_nyquist } eq "infinite") ? "infinite" : ($result{ mag_nyquist } != 0) ? 20 * log(abs($result{ mag_nyquist })) / log(10) : "-infinite";
	$result{ gain_cutoff }  = ($result{ mag_cutoff }  eq "infinite") ? "infinite" : ($result{ mag_cutoff }  != 0) ? 20 * log(abs($result{ mag_cutoff })) / log(10) : "-infinite";

	my $w0_2 = (1 + $b1 - $b2 != 0) ? (1 - $b1 - $b2) / (1 + $b1 - $b2) : 0;
	my $w0;

	if ((abs(1 + $b1 - $b2) < $ERROR_LEVEL) && (abs($a0 - $a1 + $a2) < $ERROR_LEVEL)) {	# s^2 become zero
		$result{ memo } = "this filter is actually 1st order filter";
		my $a0_FA = (-3 * $a0 - $a1 + $a2) / (-3 + $b1 - $b2);
		my $a1_FA = ($a0 - $a1 - 3 * $a2) / (-3 + $b1 - $b2);
		my $b1_FA = -(1 + $b1 + 3 * $b2) / (-3 + $b1 - $b2);
		CeqFA_Analyze($a0_FA, $a1_FA, $b1_FA, $Fs, \%result);
		return \%result;
	} elsif ((abs(1 - $b1 - $b2) < $ERROR_LEVEL) && (abs($a0 + $a1 + $a2) < $ERROR_LEVEL)) {	# s^0 become zero
		$result{ memo } = "this filter is actually 1st order filter";
		my $a0_FA = (3 * $a0 - $a1 - $a2) / (3 + $b1 + $b2);
		my $a1_FA = ($a0 + $a1 - 3 * $a2) / (3 + $b1 + $b2);
		my $b1_FA = -(1 - $b1 + 3 * $b2) / (3 + $b1 + $b2);
		CeqFA_Analyze($a0_FA, $a1_FA, $b1_FA, $Fs, \%result);
		return \%result;
	} elsif ($w0_2 > 0) {
		$w0 = sqrt($w0_2);
	} else {
		$result{ memo } = "cannot proceed calculation, this coefficients were not calculated with \"bilinear arithmetic\"";
		$result{ type } = "ERROR";
		return \%result;
	}

	$result{ pole1 } = ($b1 + sqrt(cplx($b1**2 + 4*$b2, 0))) / 2;
	$result{ pole2 } = ($b1 - sqrt(cplx($b1**2 + 4*$b2, 0))) / 2;
	$result{ zero1 } = ($a0 != 0) ? (-$a1 + sqrt(cplx($a1**2 - 4*$a0*$a2, 0))) / (2*$a0) : (($a1 != 0) ? (-$a2 / $a1) : 0);
	$result{ zero2 } = ($a0 != 0) ? (-$a1 - sqrt(cplx($a1**2 - 4*$a0*$a2, 0))) / (2*$a0) : (($a1 != 0) ? (-$a2 / $a1) : 0);

	#	printf "pole1 (%8.5f, %8.5f), pole2 (%8.5f, %8.5f) / zero1 (%8.5f, %8.5f), zero2 (%8.5f, %8.5f)\n",
	#		Re($result{ pole1 }), Im($result{ pole1 }), Re($result{ pole2 }), Im($result{ pole2 }), Re($result{ zero1 }), Im($result{ zero1 }), Re($result{ zero2 }), Im($result{ zero2 });

	#	printf "pole1 arg(%7.2f, %9.2fHz), pole2 (%7.2f, %9.2fHz) / zero1 (%7.2f, %9.2fHz), zero2 (%7.2f, %9.2fHz)\n",
	#		180 * arg($result{ pole1 }) / pi(), $Fs / 2 * arg($result{ pole1 }) / pi(), 180 * arg($result{ pole2 }) / pi(), $Fs / 2 * arg($result{ pole2 }) / pi(),
	#		180 * arg($result{ zero1 }) / pi(), $Fs / 2 * arg($result{ zero1 }) / pi(), 180 * arg($result{ zero2 }) / pi(), $Fs / 2 * arg($result{ zero2 }) / pi();


	if ((abs($result{ pole1 }) > 1) || (abs($result{ pole2 }) > 1)) {
		$result{ memo } = "pole is out of unit circle. this filter will be unstable!";
		$result{ type } = "ERROR";
		return \%result;
	}

	$result{ freq_a } = $w0 * $Fs / pi();
	$result{ freq_d } = (1 + $b1 - $b2 != 0) ? atan2($w0, 1) / pi() * $Fs : $Fs;	# reverse prewarping

	$result{ q_a } = ($b2 != -1) ? sqrt((1 - $b1 - $b2) * (1 + $b1 - $b2)) / (2 * $b2 + 2) : "infinite";
	#$result{ q_a } = -$b1 * $w0 / ((2 + $b1) * $w0_2 - 2 + $b1);
	$result{ q_d } = (($result{ freq_d } != 0) && ($result{ q_a } ne "infinite")) ? $result{ q_a } * $Fs * tan(pi() * $result{ freq_d } / $Fs) / ($result{ freq_d } * pi()) : "infinite";	# reverse prewarping


	if      (($a0 == $a2) && (abs(2 * $a0 + $a1) < $ERROR_LEVEL)) {	# HPF
		$result{ type } = "HPF2";
		return \%result;
	} elsif (($a0 == $a2) && (abs(2 * $a0 - $a1) < $ERROR_LEVEL)) {	# LPF
		$result{ type } = "LPF2";
		return \%result;
	} elsif ((abs($b1 * $a0 + $a1 - $a1 * $b2 + $b1 * $a2) < $ERROR_LEVEL) && ($b1 != 0)) {	# PKG | BPF | APF
		if ($a1 == 0) {
			$result{ type } = "BPF2";
			return \%result;
		} elsif ($a0 == $a2) {	# BEF
			$result{ type } = "BEF2";
			return \%result;
		} elsif (abs($a1 * (1 + $b2) -$b1 * ($a0 - $a2)) < $ERROR_LEVEL) {
			$result{ type } = "APF2";
			return \%result;
		} elsif ((abs($a0 + $a1 + $a2) > $ERROR_LEVEL) && (abs($a0 - $a1 + $a2) > $ERROR_LEVEL)) {
			$result{ type } = "PKG2";
			$result{ mag_pkg }  = -$b1 / $a1 * $result{ mag_cutoff };
			$result{ gain_pkg } = ($result{ mag_pkg } != 0) ? 20 * log(abs($result{ mag_pkg })) / log(10) : "-infinite";

#			if ($result{ mag_pkg } == 0) {
			if (abs($result{ mag_pkg }) == 1) {
				$result{ type } = "THRU";
			} elsif (abs($result{ mag_pkg }) < 1) {
				$result{ q_d } = $result{ q_d } / abs($result{ mag_pkg });
				$result{ q_a } = $result{ q_a } / abs($result{ mag_pkg });
			}

			return \%result;
		}
	} elsif ((abs($result{ mag_dc } * $result{ mag_nyquist } - $result{ mag_cutoff } ** 2) < $ERROR_LEVEL) && ($b1 != 0)) {
		my $w0_svf1 = ($result{ mag_cutoff } > 0) ? $w0 / $result{ mag_cutoff } : $w0 * $result{ mag_cutoff };
		my $w0_svf2 = ($result{ mag_cutoff } < 0) ? $w0 / $result{ mag_cutoff } : $w0 * $result{ mag_cutoff };
		$result{ freq1_d } = (1 + $b1 - $b2 != 0) ? atan2($w0_svf1, 1) / pi() * $Fs : $Fs;	# reverse prewarping
		$result{ freq2_d } = (1 + $b1 - $b2 != 0) ? atan2($w0_svf2, 1) / pi() * $Fs : $Fs;	# reverse prewarping

		if ($result{ mag_nyquist } >= $result{ mag_dc }) {
			$result{ type } = "HI_BOOST2";
		} else {
			$result{ type } = "HI_CUT2";
		}
		return \%result;
	} elsif (($a2 == 0) && ($b2 == 0)) {	# 1st order filter
		CeqFA_Analyze($a0, $a1, $b1, $Fs, \%result);
		return \%result;
	}


	#
	# not a regular filters (complexed)
	#
	$result{ type } = "COMPLEX";
	if (($result{ mag_cutoff } != 0) && ($a1 * (1 + $b2) != 0)) {
		$result{ mag_pkg }  = -$b1 / $a1 * $result{ mag_cutoff };
		$result{ gain_pkg } = ($result{ mag_pkg } != 0) ? 20 * log(abs($result{ mag_pkg })) / log(10) : "-infinite";
	}
	return \%result;
}

1;
