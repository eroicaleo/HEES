#!/usr/bin/env perl

use warnings;
use strict;

use autodie;
use File::Copy;
use POSIX qw(strftime);

#########################################
# Modify the parameters to run the test #
#########################################
my $numOfTestSets = 10;
my $numOfTasks = 3;
my $minPowerTasks = 0.6;
my $maxPowerTasks = 1.5;
my $harvestingPower = 2.2;
my $deadlineRatio = 1.05;

# Create a directory so the 
# log file does not mess up with files

# Get the current time up to minutes
my $nowString = strftime "%Y%m%d%H%M", localtime;
print "$nowString\n";
my $resultsDir = "results$nowString";
mkdir $resultsDir unless -d $resultsDir;

# Main loop
# Run the tests, process log file and archieve the results
foreach my $i (0..($numOfTestSets-1)) {
  my $format = sprintf "%04d", $i;
  my $logFile = "runinfo${format}.log";
  my $cmdstr = "./Scheduler --ratio_runtime_and_deadline $deadlineRatio --constant_power_value $harvestingPower > $logFile";
  print $cmdstr, "\n";
  system($cmdstr);
  &processFile($logFile);
  move $logFile, $resultsDir;
  move "TasksOrig.txt", "$resultsDir/TasksOrig${format}.txt";
  move "TasksDP.txt", "$resultsDir/TasksDP${format}.txt";
  move "TasksCATSFixed.txt", "$resultsDir/TasksCATSFixed${format}.txt";
  move "TasksCATS.txt", "$resultsDir/TasksCATS${format}.txt";
  sleep 1;
}
move "Summary.log", $resultsDir;

sub processFile {
  my $filename = $_[0];
  my @energyArray; 
  my @runtimeArray;
  my $line;
  open my $fh, "<", "$filename";
  while ($line = <$fh>) {
    if ($line =~ m/Final Energy: ([0-9.-]+) Finish Time: ([0-9.-]+)/) {
      print $line;
      push @energyArray, $1;
      push @runtimeArray, $2;
    }
  }
  close $fh;
  my $impr = 0;
  $impr = ($energyArray[0]-$energyArray[1])/$energyArray[1] unless ($energyArray[1] == 0);
  printf "DP: $energyArray[0] CatsFixed: $energyArray[1] RuntimeDiff: %d Improve: %.2f\n", $runtimeArray[0]-$runtimeArray[1], $impr;
  open $fh, ">>", "Summary.log";
  printf $fh "DP: $energyArray[0] CatsFixed: $energyArray[1] RuntimeDiff: %d Improve: %.2f\n", $runtimeArray[0]-$runtimeArray[1], $impr;
  close $fh;
}
