#!/bin/bash
COPYTO=$1

if [ "x$COPYTO" = "x" ] || [ ! -d "$COPYTO" ]
then
  echo "$(basename $0) <directory to merge to>"
  exit 1
fi

STARTDIR=$(pwd)

COPYFILES=""
DELFILES=""
NEWFILES=""

function isversioned {
  F=$1
  svn info $F 2>&1 | grep Checksum > /dev/null
  rc=$?
  if [ $rc -eq 0 ]
  then
    echo 1
  else
    echo 0
  fi
}

function checkCopy {

echo "[-] Checking for files that need to be copied to: [$COPYTO]..."
echo "-----------------------------------------------------"

# first pass is to copy all files we care about from current to destination
for f in $(find . | egrep -vi \
"aap|build-|\.svn|\.libs|Makefile$|Makefile.in|~$|\.la$")
do
if [ -f $f ]; then
  F=$(printf "%-60s\n" $f)
  C=$(sum -r $f)
  O=$(sum -r $COPYTO/$f 2>/dev/null)
  if [ "$C" = "$O" ] ; then
    S="SAME"
  elif [ ! -f $COPYTO/$f ]; then
    S="NEW"
  else
    S="DIFF"
  fi

  if [ "$S" != "SAME" ]; then
    echo "file: [$F], status: [$S]"
    if [ "$S" = "DIFF" ]; then
      COPY="y"
    elif [ "$S" = "NEW" ] ; then
      V=$(isversioned $f)
      if [ "$V" -eq 0 ]; then
        echo " - new file, but not versioned, so ignoring."
	COPY="n"
      elif [ "$V" -eq 1 ]; then
        echo -n " - new file.  versioned. copy this one? (Y/N) -> "
        read ANS
        ANS=$(echo $ANS | tr '[A-Z]' '[a-z]')
        if [ "$ANS" = "y" ]; then
          COPY="y"
	  NEWFILES="$NEWFILES $f"
        else
          COPY="n"
        fi
      fi
    fi
    if [ "$COPY" = "y" ]; then
      echo "  - copying this file..."
      COPYFILES="$COPYFILES $f"
    else 
      echo "  - not copying it..."
    fi
  fi
fi
done
}

function checkDelete {

echo "[-] Checking for files that should be deleted from: [$COPYTO] ..."
echo "-----------------------------------------------------"

# now see if there's anything that was in dest, but is not in new and
# remove it
cd $COPYTO
for f in $(find . | egrep -vi \
"aap|build|.svn|.libs|Makefile|~$|lib/pilot-link|.la$|.deps|.moc$|.lo$|\.o$")
do
if [ -f $f ]; then
  F=$(printf "%-60s\n" $f)
  if [ ! -f $STARTDIR/$f ] ; then
    V=$(isversioned $f)
    if [ "$V" -eq 1 ]; then
      echo -n " - file: [$F] looks like it's been deleted. should I remove it?  (Y/N) -> "
      read ANS
      ANS=$(echo $ANS | tr '[A-Z]' '[a-z]')
      if [ "$ANS" = "y" ]; then
        echo "  - okay, I'll remove this one..."
        DELFILES="$DELFILES $f"
      fi
    fi
  fi
fi
done
}

checkCopy
checkDelete

cd "$STARTDIR"

echo "okay, here are the files that I'll copy:"
for f in $(echo $COPYFILES)
do
  echo " - $f"
done

echo "and here are the files that I'll do an svn remove on:"
for f in $(echo $DELFILES)
do
  echo " - $f"
done

echo "and here are the files that I'll do an svn add on:"
for f in $(echo $NEWFILES)
do
  echo " - $f"
done

echo -n "Okay to proceed? (y/n) -> "
read ANS
ANS=$(echo $ANS | tr '[A-Z]' '[a-z]')
if [ "$ANS" != "y" ]; then
  echo "  - okay, stopping."
  exit
fi

cd "$STARTDIR"

echo "okay, copying..."
for f in $(echo $COPYFILES)
do
  cp --parents -v "$f" "$COPYTO"
done

cd "$COPYTO"

echo "doing svn remove..."
for f in $(echo $DELFILES)
do
  echo " - $f"
  svn remove "$f"
done

cd "$COPYTO"

echo "doing svn add..."
for f in $(echo $NEWFILES)
do
  echo " - $f"
  svn add "$f"
done
