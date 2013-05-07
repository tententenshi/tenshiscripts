use strict;


sub ProcessHeader
{
	my ($fh, $infoHashRef) = @_;

	my $buf;
	read $fh, $buf, 4;	# MThd
	my $str = unpack "a4", $buf;
	if ($str ne "MThd") {
		die "not proper format (MThd not found)\n";
	}

	read $fh, $buf, 4;	# size of header
	my $size = unpack "N", $buf;
	read $fh, $buf, 2;	# format
	$$infoHashRef{ "format" } = unpack "n", $buf;
	read $fh, $buf, 2;	# num of track
	$$infoHashRef{ "num_of_track" } = unpack "n", $buf;
	read $fh, $buf, 2;	# TPQN
	$$infoHashRef{ "TPQN" } = unpack "n", $buf;

	read $fh, $buf, $size - 6;	# skip remaining

#	print "format: ". $$infoHashRef{"format"} . " num_of_track: " . $$infoHashRef{"num_of_track"} . " TPQN: ". $$infoHashRef{"TPQN"} . "  \n";
}

sub ParseDeltaTime
{
	my ($fh, $sizeRef) = @_;

	my $buf;
	read $fh, $buf, 1;
	$buf = unpack("C", $buf);
	my $val = $buf & 0x7f;
	$$sizeRef--;
	while ($buf & 0x80) {
		read $fh, $buf, 1;
		$buf = unpack("C", $buf);
		$val = ($val << 7) | ($buf & 0x7f);
		$$sizeRef--;
	}
	return $val;
}


sub ReadVariableLengthValue
{
	my ($fh, $sizeRef, $dataArrayRef) = @_;

	my $buf = $$dataArrayRef[-1];
	my $val = $buf & 0x7f;
	while ($buf & 0x80) {
		read $fh, $buf, 1;
		$buf = unpack("C", $buf);
		push @$dataArrayRef, $buf;
		$val = ($val << 7) | ($buf & 0x7f);
		$$sizeRef--;
	}
	return $val;
}

my $prevCommand;
sub ParseData
{
	my ($fh, $sizeRef) = @_;

	my @data = ();
	my $buf;
	read $fh, $buf, 1; $buf = unpack("C", $buf); $$sizeRef--;

	my $command;
	my $isRunningStatus = 0;
	if (($buf & 0xf0) < 0x80) {		# running status
		$command = $prevCommand;
		push @data, $command;
		push @data, $buf;
		$isRunningStatus = 1;
	} else {
		$prevCommand = $command = $buf;
		push @data, $buf;
		read $fh, $buf, 1; $buf = unpack("C", $buf); $$sizeRef--;
		push @data, $buf;
	}

	my $packetsize;
	if      (($command & 0xf0) == 0x80) {	# note on
		$packetsize = 3 - 2;
	} elsif (($command & 0xf0) == 0x90) {	# note off
		$packetsize = 3 - 2;
	} elsif (($command & 0xf0) == 0xa0) {	# polyphonic key pressure
		$packetsize = 3 - 2;
	} elsif (($command & 0xf0) == 0xb0) {	# control change
		$packetsize = 3 - 2;
	} elsif (($command & 0xf0) == 0xc0) {	# program change
		$packetsize = 2 - 2;
	} elsif (($command & 0xf0) == 0xd0) {	# channel pressure
		$packetsize = 2 - 2;
	} elsif (($command & 0xf0) == 0xe0) {	# pitch bend
		$packetsize = 3 - 2;
	} elsif (($command & 0xf0) == 0xf0) {	# 
		if ($command == 0xf0) {
			$packetsize = ReadVariableLengthValue($fh, $sizeRef, \@data);
		} elsif ($command == 0xf7) {
			$packetsize = ReadVariableLengthValue($fh, $sizeRef, \@data);
		} elsif ($command == 0xff) {
			my $metaevent = $buf;
			read $fh, $buf, 1; $buf = unpack("C", $buf); $$sizeRef--;
			push @data, $buf;
			$packetsize = ReadVariableLengthValue($fh, $sizeRef, \@data);
		} else {
			printf ("unknown command 0x%x found \n", $command);
		}
	}
	while ($packetsize > 0) {
		read $fh, $buf, 1; $buf = unpack("C", $buf); $$sizeRef--;
		push @data, $buf;
		$packetsize--;
	}

	return [$isRunningStatus, @data];
}


sub ProcessTrack
{
	my ($fh) = @_;

	my @track;

	my $buf;
	read $fh, $buf, 4;	# MTrk
	my $str = unpack "a4", $buf;
	if ($str ne "MTrk") {
		die "not proper format (MTrk not found)\n";
	}
	read $fh, $buf, 4;	# size of header
	my $size = unpack "N", $buf;

	my $elapseTime = 0;

	while ($size > 0) {
		my $deltaTime = ParseDeltaTime($fh, \$size);
		my ($isRunningStatus, @data)  = @{ ParseData($fh, \$size) };
		$elapseTime += $deltaTime;
		push @track, [$elapseTime, $isRunningStatus, @data];
	}

	return @track;
}

sub ReadAndParse
{
	my ($smfFileName, $infoHashRef, $trackArrayRef) = @_;

	my $fh;
	open ($fh, "$smfFileName") or die "cannnot open $smfFileName\n";

	ProcessHeader($fh, $infoHashRef);

	my $cur_track = 1;

	while ($cur_track <= $$infoHashRef{ "num_of_track" }) {
#		print "next track ($cur_track) ...\n";
		my @track = ProcessTrack($fh);
		push @$trackArrayRef, \@track;
		$cur_track++;
	}

	close ($fh);
}


1;
