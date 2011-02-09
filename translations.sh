#! /bin/bash

function fn_exists()
{
    type $1 2>/dev/null | grep -q 'is a function'
}

function check_environment_for_translation_setup()
{
  if [ ! -n "$E3_BASE_SOURCE_DIR" ]; then
    echo "E3_BASE_SOURCE_DIR is not set, aborting."
    return 1;
  fi
  if [ ! -n "$E3_BASE_BUILD_DIR" ]; then
    echo "E3_BASE_BUILD_DIR is not set, aborting."
    return 1;
  fi
  if [ ! -n "$E4_BASE_SOURCE_DIR" ]; then
    echo "E4_BASE_SOURCE_DIR is not set, aborting."
    return 1;
  fi
  if [ ! -n "$KDEDIR" ]; then
    echo "KDEDIR is not set, aborting."
    return 1;
  fi
  if [ ! -n "$POEDIFF_SCRIPT" ]; then
    echo "POEDIFF_SCRIPT is not set, aborting."
    return 1;
  fi

  fn_exists cs
  if [ $? -eq 0 ]; then
    echo -n
  else
    echo "cs/cb functions are not defined, aborting."
    return 1
  fi

  return 0
}

function po_diff {
  $POEDIFF_SCRIPT -s -b -c svn .
}

function find_untranslated {
  echo "The following PO files need to be updated:"
  export LC_LANG=en
  for i in `find -name "*.po"`; do
    local stat=`msgfmt --statistics -o /dev/null $i 2>&1 | egrep "untranslated|fuzzy"`
    local POBLACKLIST="kalarm kcm_pci kpilot karm kitchensync kpilot_usage kpilot_configuration kpilot_sync plasma_applet_devicenotifier"
    local PONAME=`echo $i | sed 's/\// /g' | awk '{print $NF}' | sed 's/.po//'`
    if ! [ -z "$stat" ]; then
      if [[ $POBLACKLIST == *$PONAME* ]]; then
        echo -n # nop
        #echo "BLACKLISTED: $PONAME"
      else
        echo $i: $stat
      fi
    fi
  done
}

function update_e3_translations {
  check_environment_for_translation_setup
  if [ $? -eq 1 ]; then
    return
  fi

  export KDE3_PREFIX=`kde-config --prefix`
  cd $E3_BASE_SOURCE_DIR
  svn up kdepim kdelibs kde-l10n
  cd $E3_BASE_SOURCE_DIR/kde-l10n 
  export KDEDIR=$KDE3_PREFIX
  scripts/createdoctemplates.sh
  cd $E3_BASE_SOURCE_DIR/kdepim
  ./translate
  cd ..
  find_untranslated
  chmod -R 777 $E3_BASE_SOURCE_DIR/kde-l10n
}

function update_e4_translations {
  check_environment_for_translation_setup
  if [ $? -eq 1 ]; then
    return
  fi

  local TRANSDIR=$E4_BASE_SOURCE_DIR

  local CURDIFF=`svn diff $TRANSDIR/kdepim $TRANSDIR/kdepimlibs`
  if [ -n "$CURDIFF" ]; then
    echo "Working dir must be clean!"
    read
    return
  fi

  cd $TRANSDIR
  svn up kdebase-4.2-branch kdelibs-4.2-branch kdepim kdepimlibs l10n-kde4
  BASEDIR=$TRANSDIR branches/kdepim/enterprise4/l10n-kde4/scripts/update_translations
  find_untranslated
  chmod -R 777 $TRANSDIR/branches/kdepim/enterprise4/l10n-kde4
}

function install_e3_translations
{
  check_environment_for_translation_setup
  if [ $? -eq 1 ]; then
    return
  fi

  cd $E3_BASE_SOURCE_DIR/branches/kdepim/enterprise/kde-l10n
  scripts/autogen.sh de
  cd de
  ./configure --prefix=$KDEDIR
  make
  make install
}

function install_e4_translations
{
  check_environment_for_translation_setup
  if [ $? -eq 1 ]; then
    return
  fi

  cd $E4_BASE_SOURCE_DIR/branches/kdepim/enterprise4/l10n-kde4
  scripts/autogen.sh de
  cd de
  local SOURCE_FOLDER=$PWD
  cb
  cmake $SOURCE_FOLDER -DCMAKE_INSTALL_PREFIX=$KDEDIR
  make install
}
