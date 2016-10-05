#!/usr/bin/env perl

use FindBin;
use strict;

require $FindBin::RealBin . "/smf_util.pm";

my $file = shift;
my %info;
my @track;
my @LastNoteOnTime;
my @noteOnDurationHist;


sub IsNoteOn_Off {
	my ($dataRef) = @_;
	my $command = $$dataRef[0] & 0xf0;
	my $val2 = $$dataRef[2];

	if ($command == 0x90) {
		if ($val2 != 0) {
			return 1;
		} else {
			return 2;
		}
	} elsif ($command == 0x80) {
		return 2;
	} else {
		return 0;
	}
}

sub GetNoteInfo {
	my ($dataRef) = @_;
	my $command = $$dataRef[0] & 0xf0;
	my $channel = $$dataRef[0] & 0x0f;
	my $val1 = $$dataRef[1];	# note
	my $val2 = $$dataRef[2];	# velocity

	return [$channel, $val1, $val2];
}

{
	&smf_util::ReadAndParse($file, \%info, \@track);

	my ($infoHashRef, $trackArrayRef) = (\%info, \@track);

	&smf_util::Update($infoHashRef, $trackArrayRef);

	my $cur_track = 1;
	my $real_track_end;
	foreach my $track (@$trackArrayRef) {
#		print "next track ($cur_track) ...\n";
		foreach my $event (@$track) {
			my ($tickTime, $isRunningStatus, @data) = @{ $event };

			if ($data[0] == 0xff) {
#				&smf_util::ParseMetaEvent($tickTime, @data);
				if ($data[1] == 0x2f) {	# track end
#					printf("track end\n");
					if ($real_track_end < $tickTime) {
						$real_track_end = $tickTime;
					}
				}
			} else {
#				&smf_util::PrintEventTime($tickTime);
#				printf "%s", ($isRunningStatus) ? "!" : " ";
				if (IsNoteOn_Off(\@data) == 1) {		# Note On
					my ($channel, $note, $velo) = @{ GetNoteInfo(\@data) };
					if (($channel != 0) && ($channel != 2) && ($channel != 3)) { next; }
					push @{ $LastNoteOnTime[$channel][$note] }, $tickTime;
#					for (my $i = 21; $i <= 108; $i++) {
#						if ($#{ $LastNoteOnTime[$channel][$i] } >= 0) {
#							print "*";
#						} else {
#							print " ";
#						}
#					}
#					print "\n";
				} elsif (IsNoteOn_Off(\@data) == 2) {	# Note Off
					my ($channel, $note, $velo) = @{ GetNoteInfo(\@data) };
					if (($channel != 0) && ($channel != 2) && ($channel != 3)) { next; }
					if ($#{ $LastNoteOnTime[$channel][$note] } >= 0) {
						my $lastNoteOnTime = shift @{ $LastNoteOnTime[$channel][$note] };
						my $noteOnTime = &smf_util::Tick2RealElapseTime($lastNoteOnTime);
						my $noteOffTime = &smf_util::Tick2RealElapseTime($tickTime);
						my $duration = ($noteOffTime - $noteOnTime);
						push @{ $noteOnDurationHist[$note][$channel] }, $duration;
#						print "channel: $channel note: $note duration: $duration\n";
					}

#					for (my $i = 21; $i <= 108; $i++) {
#						if ($#{ $LastNoteOnTime[$channel][$i] } >= 0) {
#							print "*";
#						} else {
#							print " ";
#						}
#					}
#					print "\n";
				}
#				foreach my $val (@data) {
#					printf("%02x ", $val);
#				}
#				print "\n";
			}
		}
		$cur_track++;
	}

	my $end_of_song_time = smf_util::Tick2RealElapseTime($real_track_end) / 1000000;
#	print "end_of_song: $end_of_song_time\n";
	print "$file, end, $end_of_song_time, notes, ";
	for (my $note = 0; $note < 128; $note++) {
		my $total_duration = 0;
		my $total_note_count = 0;
		for (my $channel = 0; $channel < 16; $channel++) {
			if (defined($noteOnDurationHist[$note][$channel])) {
				my @durlist = @{ $noteOnDurationHist[$note][$channel] };
				foreach my $duration (@durlist) {
#					print "channel: $channel, note: $note, duration: $duration\n";
					$total_duration += $duration;
					$total_note_count++;
				}
			}
		}
#		if ($total_duration > 0) {
			my $duration_sec = $total_duration / 1000000;
#			print "note: $note count: $total_note_count, total_duration: $duration_sec\n";
#			print "$note, $total_note_count, $duration_sec\n";
			print "$duration_sec, ";
#		}
	}
	print "\n";

}

