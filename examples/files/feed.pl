#!/usr/bin/perl

use strict;
use warnings;

my $pwd;
if(scalar(@ARGV) == 0) {
    $pwd = `pwd`;
    chomp $pwd;
} else {
    $pwd = $ARGV[0];
}

opendir(my $dir, $pwd) || die "Couldn't open $pwd.\n";
my @files = sort readdir($dir);
closedir $dir;

foreach my $f (@files) {
    next if $f eq ".";
    next if $f eq ".." and $pwd eq "/";
    print "$pwd/$f\t$f";
    print "/" if -d "$pwd/$f";
    print "\n";
}

