#!/usr/bin/perl -w
#
# eproof - run E and translate its output into TSTP format
#
# This script is a port of a Bash script with the same name coming with
# E 1.0-004 (written by Stephan Schulz).
#
# Contributed by Sascha Boehme (TUM)
#

use File::Basename qw/ dirname /;
use File::Temp qw/ tempfile /;
use English;


# E executables

my $edir = dirname($0);
my $eprover = "$edir/eprover";
my $epclextract = "$edir/epclextract";


# build E command from given commandline arguments

my $format = "";
my $timelimit = 2000000000;   # effectively unlimited

my $eprover_cmd = "'$eprover'";
foreach (@ARGV) {
  if (m/--cpu-limit=([0-9]+)/) {
    $timelimit = $1;
  }

  if (m/--tstp-out/) {
    $format = $_;
  }
  else {
    $eprover_cmd = "$eprover_cmd '$_'";
  }
}
$eprover_cmd = "$eprover_cmd -l4 -R -o- --pcl-terms-compressed --pcl-compact";


# run E, redirecting output into a temporary file

my ($fh, $filename) = tempfile(UNLINK => 1);
my $r = system "$eprover_cmd > '$filename'";
exit ($r >> 8) if $r != 0;


# analyze E output

my @lines = <$fh>;
my $content = join "", @lines[-60 .. -1];
  # Note: Like the original eproof, we only look at the last 60 lines.

if ($content =~ m/Total time\s*:\s*([0-9]+\.[0-9]+)/) {
  $timelimit = int($timelimit - $1 - 1);

  if ($content =~ m/No proof found!/) {
    print "# Problem is satisfiable (or invalid), " .
      "generating saturation derivation\n";
  }
  elsif ($content =~ m/Proof found!/) {
    print "# Problem is unsatisfiable (or provable), " .
      "constructing proof object\n";
  }
  elsif ($content =~ m/Watchlist is empty!/) {
    print "# All watchlist clauses generated, constructing derivation\n";
  }
  else {
    print "# Cannot determine problem status\n";
    exit $r;
  }
}
else {
  print "# Cannot determine problem status within resource limit\n";
  exit $r;
}


# translate E output

foreach (@lines) {
  print if (m/# SZS status/ or m/"# Failure"/);
}
$r = system ("exec bash -c \"ulimit -S -t $timelimit; " .
  "'$epclextract' $format -f --competition-framing '$filename'\"");
  # Note: Setting the user time is not supported on Cygwin, i.e., ulimit fails
  # and prints and error message. How could we then limit the execution time?
exit ($r >> 8);

