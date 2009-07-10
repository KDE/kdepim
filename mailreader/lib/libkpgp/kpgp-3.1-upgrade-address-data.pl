#!/usr/bin/perl

# Read all "AddressKeyEntry #" groups and the "EncryptionPreferences" group
# and store the information together in new "Address #" groups

my %data;

$currentGroup = "";
$address = "";

while(<>) {
  next if /^$/;
  # filter out groups:
  if( /^\[(.+)\]$/ ) {
    $currentGroup = $1;
    # delete the obsolete groups
    if( ( $currentGroup =~ /^AddressKeyEntry \d*/ ) ||
        ( $currentGroup eq "EncryptionPreferences" ) ) {
      print "# DELETEGROUP [$currentGroup]\n";
    }
    next;
  }
  # store the values of the old groups and delete them
  if( $currentGroup =~ /^AddressKeyEntry \d*/ ) {
    if( /^Address=(.*)$/ ) {
      $address = $1;
    }
    elsif( /^Key ID=(.*)$/ ) {
      $data{$address}{"keyid"} = $1;
    }
  }
  elsif( $currentGroup eq "EncryptionPreferences" ) {
    ($address,$encrpref) = split /=/;
    chomp $encrpref;
    $data{$address}{"encrpref"} = $encrpref;
  }
}

# write the new address groups
$n = 1;
foreach $address ( keys %data ) {
  %addressData = %{$data{$address}};
  print "[Address #$n]\n";
  print "Address=$address\n";
  if( exists $addressData{"encrpref"} ) {
    $encrpref = $addressData{"encrpref"};
    print "EncryptionPreference=$encrpref\n";
  }
  if( exists $addressData{"keyid"} ) {
    $keyid = $addressData{"keyid"};
    print "Key IDs=$keyid\n";
  }
  $n++;
}
$n--;

# write the number of address groups
print "# DELETE [General]addressKeyEntries\n";
print "# DELETE [General]addressEntries\n";
print "[General]\naddressEntries=$n\n";

