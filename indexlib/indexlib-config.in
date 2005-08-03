#!/bin/sh

usage()
{
	echo "usage: $0 [OPTIONS]"
cat << EOH

options:
	[--libs]
	[--cflags]
	[--version]
	[--prefix]
EOH
}

prefix=@prefix@
exec_prefix=@exec_prefix@
libdir=@libdir@
includedir=@includedir@

flags=""

if test $# -eq 0 ; then
  usage
  exit 0
fi

while test $# -gt 0
do
  case $1 in
    --libs)
	  flags="$flags -L$libdir -lindex"
	  ;;
    --cflags)
	  flags="$flags -I$includedir/index"
	  ;;
    --version)
	  echo 0.94
	  ;;
    --prefix)
	  echo $prefix
	  ;;
    --help)
          usage
	  exit 0
	  ;;
	*)
	  echo "$0: unknown option $1"
	  echo
	  usage
	  exit 1
	  ;;
  esac
  shift
done

if test -n "$flags"
then
  echo $flags
fi
