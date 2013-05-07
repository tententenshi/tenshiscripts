use strict;


sub WriteHeader
{
	my ($fh, $infoHashRef, $num_of_track) = @_;

	my $buf;
	$buf = pack "a4", "MThd";
	print $fh $buf;

	$buf = pack "N", 6;
	print $fh $buf;		# size of header

	my $format = $$infoHashRef{ "format" };
	if ($num_of_track > 1) {
		$format = 1;
	}
	$buf = pack "n", $format;
	print $fh $buf;		# format

	$buf = pack "n", $num_of_track;
	print $fh $buf;		# num of track

	$buf = pack "n", $$infoHashRef{ "TPQN" };
	print $fh $buf;		# TPQN
}


sub WriteTrack
{
	my ($fh, $trackRef) = @_;

### count size
	my $prevTime = 0;
	my $prevCommand;
	my $size = 0;
	foreach my $event (@$trackRef) {
		my ($elapseTime, $isRunningStatus, @data) = @{ $event };
		my $deltaTime = $elapseTime - $prevTime;

		if ($deltaTime < 0) {
			die("delta time with event @data is not proper.\n");
		}

		if ($deltaTime >= (1 << 21)) { $size += 4; }
		elsif ($deltaTime >= (1 << 14)) { $size += 3; }
		elsif ($deltaTime >= (1 <<7)) { $size += 2; }
		else { $size += 1; }

		my $command = $data[0];
		if ((($command & 0xf0) != 0xf0) && ($command == $prevCommand) && ($isRunningStatus)) {
			$size += @data - 1;
		} else {
			$size += @data;
		}
		$prevTime = $elapseTime;
		$prevCommand = $command;
	}

###
###
	my $buf;
	$buf = pack "a4", "MTrk";
	print $fh $buf;
	$buf = pack "N", $size;
	print $fh $buf;		# size of track

	my $prevTime = 0;
	my $prevCommand = 0;
	foreach my $event (@$trackRef) {
		my ($elapseTime, $isRunningStatus, @data) = @{ $event };
		my $deltaTime = $elapseTime - $prevTime;
		my @deltaBuf;
		if ($deltaTime >= (1 << 21)) { push @deltaBuf, ((($deltaTime >> 21) & 0x7f) | 0x80); }
		if ($deltaTime >= (1 << 14)) { push @deltaBuf, ((($deltaTime >> 14) & 0x7f) | 0x80); }
		if ($deltaTime >= (1 <<  7)) { push @deltaBuf, ((($deltaTime >>  7) & 0x7f) | 0x80); }
		                             { push @deltaBuf, ($deltaTime & 0x7f); }
		my $buf = pack("C*", @deltaBuf);
		print $fh $buf;
		my $command = $data[0];
		if ((($command & 0xf0) != 0xf0) && ($command == $prevCommand) && ($isRunningStatus)) {
			shift @data;
			my $buf = pack("C*", @data);
			print $fh $buf;
		} else {
			my $buf = pack("C*", @data);
			print $fh $buf;
		}
		$prevTime = $elapseTime;
		$prevCommand = $command;
	}
}


sub Write
{
	my ($smfFileName, $infoHashRef, $trackArrayRef) = @_;

	my $fh;
	open ($fh, "> $smfFileName") or die "cannnot open $smfFileName\n";

	my $num_of_track = @$trackArrayRef;
	WriteHeader($fh, $infoHashRef, $num_of_track);

	foreach my $trackRef (@$trackArrayRef) {
		WriteTrack($fh, $trackRef);
	}

	close ($fh);
}

1;
