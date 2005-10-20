#!/usr/bin/env zsh

index=delete-me
files=(
	one ' On October 11th 2005, the KDE Project released KOffice 1.4.2. KOffice is a free light-weight yet feature rich office solution that integrates with KDE, supports the OASIS OpenDocument file format as does OpenOffice.org 2 and provides filters for other office suites such as Microsoft Office. Read the KOffice 1.4.2 Release Notes.'
	two 'KDE is a powerful Free Software graphical desktop environment for Linux and Unix workstations. It combines ease of use, contemporary functionality, and outstanding graphical design with the technological superiority of the Unix operating system. More... '
	three 'The YaKuake Package for Debian sarge and sid.
 Yet Another Kuake aka YaKuake VERSION 2.6
 http://www.kde-look.org/content/show.php?content=29153
 
 have fun!
 
 Thx OldKid for compile on debian amd64.
 
 
'
	numbers '123456789'
)
expected=( \
	kde "onetwothree"
	noshow "Empty results"
	poWeRFuL 'two'
	'kde BUT debian' "onetwo"
	debian 'three'
	'12345678' 'numbers'
	)
driver=./indexlibadmin
unittests=./testdriver

$unittests

echo "Running tests on the command line..."

mkdir $index
for name data in $files; do
	$driver add $index $name - <<<$data
done

for q res in $expected ; do
	$driver search $index $q | tr -d '\n' | read got
	if test $res != $got; then 
		echo "ERROR in test '$q'"
		echo "EXPECTED:"
		echo -$res-
		echo "GOT:"
		echo -$got-
	fi
done

rm -rf $index

echo "done."
