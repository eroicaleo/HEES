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
my $numOfTasks = 1;
my $minPowerTasks = 1;
my $maxPowerTasks = 12;
my $harvestingPower = 2.3;
my $deadlineRatio = 1.0;

# Create a directory so the 
# log file does not mess up with files

# Get the current time up to minutes
my $nowString = strftime "%Y%m%d%H%M", localtime;
print "$nowString\n";
my $resultsDir = "results$nowString";
mkdir $resultsDir unless -d $resultsDir;

my @taskPowerArray = ($minPowerTasks..$maxPowerTasks);
@taskPowerArray = map { $_ * 0.1 } @taskPowerArray;
print "@taskPowerArray\n";

# Main loop
# Run the tests, process log file and archieve the results
foreach my $i (0..$#taskPowerArray) {
  my $power = $taskPowerArray[$i];
  my $format = sprintf "%04d", $i;
  my $logFile = "runinfo${format}.log";
  my $cmdstr = "./single --ratio_runtime_and_deadline $deadlineRatio --constant_power_value $harvestingPower ";
  $cmdstr .= "--number_of_tasks $numOfTasks --min_task_power $power --max_task_power $power > $logFile";
  print $cmdstr, "\n";
  system($cmdstr);
  move $logFile, $resultsDir;
  sleep 1;
}
