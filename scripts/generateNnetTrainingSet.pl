#!/usr/bin/perl

use warnings;
use strict;

# usage:
# ./generateNnetTrainingSet.pl OverallProcess.txt [steps of start and end] [steps between this start and next start]

die "Please provide OverallProcess.txt file and a steps!" if @ARGV < 2;

my ($filename, $len, $step) = @ARGV;

if (!$step) {
  $step = $len;
}

open my $inputfh, "<", "$filename" or
  die "Can not open $filename! $!";

# filter out the first line
<$inputfh>;

my @energyArrary = ();
my $inputPower;
my $energy;

while (<$inputfh>) {
  my @line = split /\t/, $_;
  ($inputPower, $energy) = @line[7, 11];
  push @energyArrary, $energy;
}

my $i = 0;
for ($i = 0; $i < $#energyArrary; $i += $step) {
  if ($i + $len < $#energyArrary) {
    print "$inputPower\t$energyArrary[$i]\t$len\t$energyArrary[$i+$len]\n";
  }
}

close $inputfh;
