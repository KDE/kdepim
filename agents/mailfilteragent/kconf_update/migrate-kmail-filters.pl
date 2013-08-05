#!/usr/bin/perl
#
# Copyright (c) 2011 Tobias Koenig <tokoe@kde.org>
# based on migrate-kmail-filters.pl by Volker Krause <vkrause@kde.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
#

$currentGroup = "";

while (<STDIN>) {
    next if /^$/;
    # recognize groups:
    if ( /^\[(.+)\]$/ ) {
        $currentGroup = $1;
        if ( $currentGroup =~ /^Filter/ ) {
            print "# DELETEGROUP [$currentGroup]\n";
            print "[$currentGroup]\n";
        }
        next;
    };

    ($key,$value) = split /=/;
    chomp $value;

    # Move over keys from the transport groups
    if ( $currentGroup =~ /^Filter/ ) {
        print "$key=$value\n";
    }

    # Move over the key for the default transport
    elsif ( $currentGroup eq 'General' ) {
        if ( $key eq 'filters' ) {
            print "[General]\n$key=$value\n";
            print "# DELETE [$currentGroup]$key\n";
        }
    }
}
