#! /bin/sh
##
## checkPlugin.sh
##
## Copyright (C) 2002 by Adriaan de Groot
##
## Distributed under the GNU General Public License (GPL) Version 2.
##

##
## Usage: checkPlugin.sh <app-path> <plugin-path> [<extra-lib> ...]
##
## <app-path> :    path to the application that will be loading the
##                 plugin. This is used to get the list of library
##                 dependencies.
## <plugin-path> : path to the plugin (.so) that will be loaded.
## <extra-lib> :   paths to additional libraries to get defined symbols from.
##

USAGE="Usage: checkPlugin.sh <app-path> <plugin-path> [<extra-lib> ...]"

UNDEF_RE="^ *U "
DEF_RE="^[0-9a-fA-F]* [TdWBVDR] "
TMP="/tmp/$$"

APP_PATH="$1"
PLUGIN_PATH="$2"

test -z "$APP_PATH" && echo "$USAGE"
test -z "$APP_PATH" && exit 1
test -f "$APP_PATH" || echo "$USAGE"
test -f "$APP_PATH" || exit 1

test -z "$PLUGIN_PATH" && echo "$USAGE"
test -z "$PLUGIN_PATH" && exit 1
test -f "$PLUGIN_PATH" || echo "$USAGE"
test -f "$PLUGIN_PATH" || exit 1

shift 2

if nm --demangle "$PLUGIN_PATH" > "$TMP-1" ; then
	echo `wc -l < "$TMP-1"` "symbols in $PLUGIN_PATH"
else
	echo "nm failed on $PLUGIN_PATH"
	exit 1
fi

cat "$TMP-1" | grep "$UNDEF_RE" | sed "s/$UNDEF_RE//" | sort > "$TMP-undef"

T=`ldd "$APP_PATH" | grep -v "$APP_PATH" | grep -v "not found" | sed -e 's/.*=> //' -e 's/ (.*) *$//' | sort | uniq`

for LIBF in $T $* ; do
	test -f "$LIBF" || echo "$LIBF: Not found"
	test -f "$LIBF" || exit 1

	if nm --demangle "$LIBF" > "$TMP-2" 2> /dev/null ; then
		nm --demangle --dynamic "$LIBF" >> "$TMP-2" 2> /dev/null
		# echo `wc -l < "$TMP-2"` "symbols defined in $LIBF"
	else
		echo "nm failed on $LIBF"
		exit 1
	fi

	cat "$TMP-2" | grep "$DEF_RE" | sed "s/$DEF_RE//" | sort | uniq > "$TMP-def"
	cat "$TMP-undef" "$TMP-def" | sort | uniq -d > "$TMP-now-defined"
	cat "$TMP-undef" "$TMP-now-defined" | sort | uniq -u > "$TMP-still"

	echo `wc -l < "$TMP-now-defined"` "symbols resolved by $LIBF"
	
	cat "$TMP-still" > "$TMP-undef"
done

echo `wc -l < "$TMP-undef"` "undefined symbols remain"

cat "$TMP-undef"

rm -f "$TMP" "$TMP-1" "$TMP-2" "$TMP-undef" "$TMP-def" "$TMP-now-defined" "$TMP-still"
