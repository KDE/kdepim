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

$source = $ARGV[0];

%knode_key_map = (
  "server", "host",
  "needsLogon", "auth",
  "timeout", "",
  "holdTime", ""
);

while (<STDIN>) {
    next if /^$/;
    # recognize groups:
    if ( /^\[(.+)\]$/ ) {
        $currentGroup = $1;
        if ( $source eq "kmail" && $currentGroup =~ /^Transport/ ) {
            print "# DELETEGROUP [$currentGroup]\n";
            $groupid = $currentGroup;
            $groupid =~ s/^Transport //;
            print "[Transport kmail-$groupid]\n";
        }
        elsif ( $source eq "knode" && $currentGroup eq "MAILSERVER" ) {
            print "# DELETEGROUP [$currentGroup]\n";
            print "[Transport knode-0]\n";
            print "name=KNode Mail Transport\n";
        }
        next;
    };

    ($key,$value) = split /=/;
    chomp $value;

    # Move over keys from the transport groups
    if ( $source eq "kmail" && $currentGroup =~ /^Transport/ ) {
        if ( $key eq "authtype" ) {
            $value =~ s/-/_/g;
        }
        print "$key=$value\n";
    }
    elsif ( $source eq "knode" && $currentGroup eq "MAILSERVER" ) {
        $key = $knode_key_map{$key} if exists $knode_key_map{$key};
        next if $key eq "";
        print "$key=$value\n";
    }
    # Move over the key for the default transport
    elsif ( $source eq "kmail" && $currentGroup eq 'Composer' ) {
        if ( $key eq 'default-transport' ) {
            print "[General]\n$key=$value\n";
            #print "# DELETE [$currentGroup]$key\n";
        }
    }
}
