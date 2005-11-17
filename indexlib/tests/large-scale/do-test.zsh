#!/usr/bin/env zsh

# SET INPUT FILE BELOW
inputfile=$1
inputfile=ulyss12.txt

indexlibadmin=../../indexlibadmin
index=index

rm -rf index
mkdir index

if test -z $inputfile; then
	cat <<-END 1>&2
	This test needs a large input file as a seed.

	You might consider using http://www.gutenberg.org/ as a starting point to get a file.

	Please edit this script ($0) to set the input file.
END
	exit 1
fi

rm -rf output
mkdir output/

rm -rf tmp
mkdir tmp/

python generate.py < $inputfile

$indexlibadmin remove $index
for t in output/text_*; do 
	$indexlibadmin add $index $t
done


for w in output/words_*.list; do
	$indexlibadmin search $index "`cat $w`" >tmp/got 2>/dev/null
	source output/`basename $w list`script
	if ! diff -q tmp/got tmp/expected; then
		cat <<-END
			Pattern $w was wrong!

			Diff:
		END
		diff -u tmp/got tmp/expected
		echo "End of Diff."
		exit 1
	fi
done

rm -f tmp/got tmp/expected tmp/pat
rmdir tmp

