#!/usr/bin/perl
#
# Copyright (c) 2006 Volker Krause <vkrause@kde.org>
# based on kmail-3.3-move-identities.pl by David Faure <faure@kde.org>
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
        if ( $currentGroup =~ /^Transport/ ) {
            print "# DELETEGROUP [$currentGroup]\n";
            print "[$currentGroup]\n";
        }
        next;
    };
    # Move over keys from the transport groups
    if ( $currentGroup =~ /^Transport/ ) {
        print;
    }
    # Move over the key for the default transport
    elsif ( $currentGroup eq 'Composer' ) {
        ($key,$value) = split /=/;
        chomp $value;
        if ( $key eq 'default-transport' ) {
            print "[General]\n$key=$value\n";
            #print "# DELETE [$currentGroup]$key\n";
        }
    }
}
