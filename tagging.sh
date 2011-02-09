#! /bin/bash

function check_environment_for_tagging_setup()
{
  if [ ! -n "$E3_BASE_SOURCE_DIR" ]; then
    echo "E3_BASE_SOURCE_DIR is not set, aborting."
    return 1;
  fi
  if [ ! -n "$E5_BASE_SOURCE_DIR" ]; then
    echo "E5_BASE_SOURCE_DIR is not set, aborting."
    return 1;
  fi
  if [ ! -n "$SVNBASE" ]; then
    echo "SVNBASE is not set, aborting."
    return 1;
  fi
  if [ ! -n "$GITBASE" ]; then
    echo "GITBASE is not set, aborting."
    return 1;
  fi
  if [ ! -n "$EMERGEBASEDIR" ]; then
    echo "EMERGEBASEDIR is not set, aborting."
    return 1;
  fi

  return 0
}

# $1: old file name
# $2: current SVN revision
# $3: current date
function change_file_version {
  local NEWFILE=`cat $1 | sed s/20[0-9][0-9][0-9][0-9][0-9][0-9]/$3/g | sed s/1[0-9][0-9][0-9][0-9][0-9][0-9]/$2/g`
  echo "Changing version in file $1."
  echo "$NEWFILE" > $1
}

# $1: current SVN revision
# $2: current date
function change_version_numbers {

  local CURDIR_FOR_VERSION_NUMBERS_CHANGE=`pwd | sed 's/\// /g' | awk '{print $NF}'`
  if [ "$CURDIR_FOR_VERSION_NUMBERS_CHANGE" != "kdepim" ]; then
    echo "Wrong working directory, the current directory needs to be the kdepim source directory."
    return 1
  fi

  local CURDIFF=`svn diff`
  if [ -n "$CURDIFF" ]; then
    echo "$PWD is not clean!"
    echo "Cannot proceed with the tagging. Press <enter> to continue"
    read
    return 1
  fi

  change_file_version "kmail/kmversion.h" $1 $2
  change_file_version "kontact/src/main.cpp" $1 $2
  change_file_version "korganizer/version.h" $1 $2

  svn diff
  echo "Going to check in the above diff, press enter to continue or CTRL+C to abort."
  read
  svn ci -m "SVN_SILENT Update version numbers for today's release."
  return 0
}

function get_revision {
  svn up $1 >/dev/null 2>&1
  local GET_REVISION_REV=$(svn info $1 | grep "Revision:" | awk '{print $2}')
  echo "$GET_REVISION_REV"
}

function get_date {
  echo `date +%Y%m%d`
}

function get_time {
  echo `date --utc +%k%M%S`
}

# $1: Date, as in 20101027
# $2: New revision, as in 1023428
# $3: Directory to change, e.g. akonadi-e5
# $4: Path to change, e.g. kdesupport\\/akonadi
# $5: Tagname, e.g. enterprise5.0
#
# This function needs to be in the directory where trunk/kdesupport/emerge/portage/enterprise5 is located.
function addTarget
{
  cd $3
  local FILETOCHANGE=`ls --time-style=long-iso -l | sed '1d' | awk '{ print $8 }' | grep $3`
  #cat $FILETOCHANGE | sed 's/^\( \)*self.svnTargets\['\''[0-9]\{8,8\}'\''\] = '\''tags\/kdepim\/'$5'.[0-9]\{8,8\}.[0-9]\{7,7\}\/'$4''\''$/&\n        self.svnTargets['\'$1\''] = '\''tags\/kdepim\/'$5'.'$1'.'$2'\/'$4''\''/' | sed 's/self.defaultTarget = '\''[0-9]\{8,8\}'\''/self.defaultTarget = '\'$1\''/' > $FILETOCHANGE.tmp
  cat $FILETOCHANGE | sed 's/self.defaultTarget = '\''[0-9]\{8,8\}'\''/self.svnTargets['\'$1\''] = '\''tags\/kdepim\/'$5'.'$1'.'$2'\/'$4''\''\n        self.defaultTarget = '\'$1\''/' > $FILETOCHANGE.tmp
  echo "Adding new target $5.$1.$2 to file $3/$3.py"
  mv $FILETOCHANGE.tmp $FILETOCHANGE
  svn mv $FILETOCHANGE $3-$1.py
  cd ..
}

# $1: Current date as is 20101224
# $2: Tag base name e.g. enterprise5.0
# $3: remote repository e.g. git@git.kde.org
# $4: target name e.g. akonadi
# $5: working directory e.g. /tmp
function addGitTag
{
  echo "Tagging $4 from $3:$4"
    cd $5
    if [ ! -d $4 ]; then
        git clone $3:$4
        cd $4
    else
        cd $4
        git pull
    fi
    git tag -m "Tag $4 $1" $2-$1
    git push --dry-run --tags
    echo "Going to push the above, press enter to continue or CTRL+C to abort."
    read
    git push --tags
}

# $1: Current date
# $2: Current revision number
# $3: Tag base name, e.g. "enterprise5.0
function addAllTargets
{
  cd $EMERGEBASEDIR
  svn up
  echo "Adding targets to the emerge scripts..."
  #addTarget $1 $2 akonadi-e5 kdesupport\\/akonadi $3
  addTarget $1 $2 automoc-e5 kdesupport\\/automoc $3
  addTarget $1 $2 kdelibs-e5 kdelibs $3
  addTarget $1 $2 kdepimlibs-e5 kdepimlibs $3
  addTarget $1 $2 l10n-kde4-e5 l10n-kde4 $3
  addTarget $1 $2 qimageblitz-e5 kdesupport\\/qimageblitz $3
  addTarget $1 $2 strigi-e5 kdesupport\\/strigi $3
  addTarget $1 $2 attica-e5 kdesupport\\/attica $3
  addTarget $1 $2 kdebase-runtime-e5 runtime $3
  addTarget $1 $2 kdepim-e5 kdepim $3
  #addTarget $1 $2 kdewin-e5 kdesupport\\/kdewin $3
  #addTarget $1 $2 phonon-e5 kdesupport\\/phonon $3
  addTarget $1 $2 soprano-e5 kdesupport\\/soprano $3

  svn status
  echo "Going to check in the above changes, press enter to continue or CTRL+C to abort."
  read
  svn ci -m "SVN_SILENT Add new targets for tag $3.$1.$2"
}

# $1: base tag name, e.g. enterprise35.0 or enterprise4.0
# $2: SVN branch name, e.g. "/branches/kdepim/enterprise/" or "/branches/kdepim/enterprise4/".
#     Will be used as the base for the branch names passed in in $3.
#     Name must include slashes
# $3: list of branch names to tag, e.g. "kdepim kdepimlibs"
# $4: list of extra branches to tag. These are branches that don't live in the current
#     SVN branch, i.e. $2. Rather, they live outside that branch.
#     Example: "/trunk/KDE/kdesupport/ /trunk/KDE/kdelibs/".
#     Names must include slashes.
# $5: Should be "yes" for e5: Calls addTargets and manually tags kdesupport, which comes
#     from multiple sources there. Also some other differences. Should be "no" for e35.
#
# This function expects that we are in the base source dir, with kdepim as the subdir.
function tag_general
{
  check_environment_for_tagging_setup
  if [ $? -eq 1 ]; then
    return
  fi

  local BASETAGNAME=$1
  local SVNBRANCHNAME=$2
  local BRANCHLIST=$3
  local EXTRABRANCHES=$4

  #local REV=$(get_revision kdepim)
  local TODAY=`get_date`
  local ROOTDIR=$PWD
  local TIME=`get_time`
  #local TAGNAME="$BASETAGNAME.$TODAY.$REV"
  # use format time instead of rev since kdepim is in git
  local TAGNAME="$BASETAGNAME.$TODAY.$TIME"

  echo "Going to tag $TAGNAME"
  if [[ "$5" == "no" ]] ; then
    echo "Did you update translations?"
    echo "Did you update the changelog?"
  fi
  echo "Press enter to continue or CTRL+C to abort."
  read

  # Go to the kdepim subdir and change the version numbers there.
  if [[ "$5" == "no" ]] ; then
    cd kdepim
    change_version_numbers $REV $TODAY
    if [ $? -eq 1 ]
    then
      return 1
    fi
    cd ..
  fi

  # Checkout the tag dir from SVN and create the dir for the current tag
  echo "Creating tag base dir..."
  rm -rf /tmp/tags-$1
  svn co -q -N $SVNBASE/tags/kdepim /tmp/tags-$1
  cd /tmp/tags-$1
  svn mkdir $TAGNAME
  cd $TAGNAME
  
  # This is a bit of a hack: Create a "kdesupport" subdir when we're tagging e5
  if [[ "$5" == "yes" ]] ; then
    svn mkdir kdesupport
  fi

  svn status
  echo "Going to check in the above diff, press enter to continue or CTRL+C to abort."
  read
  svn ci -m "SVN_SILENT Create tagging dir for today's $BASETAGNAME tags."

  cd $ROOTDIR
  rm -rf /tmp/tags-$1

  echo "Now creating actual tag directories, press enter to continue or CTRL+C to abort."
  read
  local TAGTARGET=$SVNBASE/tags/kdepim/$TAGNAME
  for CURBRANCH in $BRANCHLIST;
  do
    echo "Tagging in-source branch $CURBRANCH..."
    svn cp $SVNBASE$SVNBRANCHNAME$CURBRANCH/ $TAGTARGET -m "SVN_SILENT Tag $CURBRANCH."
  done;

  for EXTRABRANCH in $EXTRABRANCHES;
  do
    echo "Tagging out-of-source branch $EXTRABRANCH..."
    svn cp $SVNBASE$EXTRABRANCH $TAGTARGET -m "SVN_SILENT Tag $EXTRABRANCH."
  done;

  if [[ "$5" == "yes" ]] ; then
    echo "Tagging kdepim and kdepim-runtime from git"
    addGitTag $TODAY $BASETAGNAME $GITBASE kdepim $E5_BASE_SOURCE_DIR
    addGitTag $TODAY $BASETAGNAME $GITBASE kdepim-runtime $E5_BASE_SOURCE_DIR

    echo "Tagging the kdesupport modules..."
    # We always need Akonadi trunk
    #svn cp $SVNBASE/trunk/kdesupport/akonadi $TAGTARGET/kdesupport -m "SVN_SILENT Tag akonadi"

    # But the rest we can take from kdesupport-for-4.5
    svn cp $SVNBASE/tags/soprano/2.5.2 $TAGTARGET/kdesupport/soprano -m "SVN_SILENT Tag soprano"
    svn cp $SVNBASE/tags/kdesupport-for-4.5/attica $TAGTARGET/kdesupport -m "SVN_SILENT Tag attica"
    svn cp $SVNBASE/tags/kdesupport-for-4.5/automoc $TAGTARGET/kdesupport -m "SVN_SILENT Tag automoc"
    svn cp $SVNBASE/tags/kdesupport-for-4.5/oxygen-icons $TAGTARGET/kdesupport -m "SVN_SILENT Tag oxygen-icons"
    #svn cp $SVNBASE/tags/kdesupport-for-4.5/strigi $TAGTARGET/kdesupport -m "SVN_SILENT Tag strigi"
    svn cp $SVNBASE/tags/kdesupport-for-4.5/qimageblitz $TAGTARGET/kdesupport -m "SVN_SILENT Tag qimageblitz"
    #svn cp $SVNBASE/trunk/kdesupport/strigi $TAGTARGET/kdesupport -m "SVN_SILENT Tag strigi"
    svn cp  $SVNBASE/branches/work/komo/strigi $TAGTARGET/kdesupport -m "SVN_SILENT Tag strigi"

    addGitTag $TODAY $BASETAGNAME $GITBASE akonadi $E5_BASE_SOURCE_DIR

    addAllTargets $TODAY $REV $1

    echo "Tagging Maemo packaging files..."
    svn cp $SVNBASE/branches/work/komo/packaging/maemo/ $TAGTARGET/packaging-maemo -m "SVN_SILENT tag maemo packaging files"
    echo "Tagging Desktop packaging files..."
    svn cp $SVNBASE/branches/kdepim/enterprise/packaging/debian/sid $TAGTARGET/packaging -m "SVN_SILENT tag desktop packaging files"

  fi

  echo "OK, created tag $TAGNAME. Don't forget to mail KKTI."

# echo a sample email message
  d="`date '+%d %B %Y'`"
  subject="This Week's E35 Tag ($d)"
  cat <<EOF
To: kolab-konsortium-tech-intern@kolab-konsortium.de
Subject: $subject
Howdy,

We created the following kdepim tags:

enterprise35:  $TAGNAME

The NewsLog.txt has been updated in the e35 branch.
Updated German translations are also included.

EOF

  return 0
}

function tag_e3
{
  cd $E3_BASE_SOURCE_DIR
  tag_general "enterprise35.0" "/branches/kdepim/enterprise/" "kde-l10n kdepim kdelibs" "" "no"
}

function tag_e5
{
  cd $E5_BASE_SOURCE_DIR
  tag_general "enterprise5.0" "/branches/work/komo/" "kdelibs kdebase/runtime" "/trunk/KDE/kdepimlibs /trunk/l10n-kde4/" "yes"
}
