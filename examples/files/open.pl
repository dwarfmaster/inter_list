#!/usr/bin/perl

use strict;
use warnings;
use Cwd 'abs_path';

my $path = abs_path($ARGV[0]);

if(-d $path) {
    print "top $path\n";
    print "bot [%i/%I] : %t\n";
    print "feed perl examples/files/feed.pl $path\n";
} else {
    print "term vim $path\n";
}

