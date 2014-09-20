#!/usr/bin/perl

use strict;
use warnings;

my $path = ".";
$path = $ARGV[0] if scalar(@ARGV) > 0;

print "spawn source examples/global/inc.sh\n";
print "map [return] spawn perl examples/files/open.pl %n\n";
print "map o<Open : > spawn perl examples/files/open.pl %s\n";

print "spawn perl examples/files/open.pl $path\n";
