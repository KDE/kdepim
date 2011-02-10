#!/bin/sh

###########################################################################
# Copyright (c) 2010-2011 Klarälvdalens Datakonsult AB,                   #
#                                    a KDAB Group company <info@kdab.com> #
# Authors: Thomas McGuire <thomas.mcguire@kdab.com>                       #
#          Allen Winter <allen.winter@kdab.com>                           #
#                                                                         #
# This program is free software; you can redistribute it and/or modify    #
# it under the terms of the GNU General Public License as published by    #
# the Free Software Foundation; either version 2 of the License, or       #
#                                                                         #
# This program is distributed in the hope that it will be useful,         #
# but WITHOUT ANY WARRANTY; without even the implied warranty of          #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            #
# GNU General Public License for more details.                            #
#                                                                         #
# You should have received a copy of the GNU General Public License along #
# with this program; if not, write to the Free Software Foundation, Inc., #
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.            #
###########################################################################

#TODO:  variable for $HOME/projects/e3

baseName="enterprise35.0"

function check_environment_for_tagging_setup()
{
  if ( test ! -n "$SVNBASE" ) then
    echo "SVNBASE is not set, aborting."
    return 1
  fi
  if ( test ! -n "$E3_BASE_SOURCE_DIR" ) then
    echo "E3_BASE_SOURCE_DIR is not set, aborting."
    return 1
  fi

  return 0
}

function get_date
{
  DATE=`date +%Y%m%d`
}

function get_time
{
  TIME=`date --utc +%k%M%S`
}

function get_svn_revision
{
  SVN_REVISION=""

  savepath=`pwd`
  cd $1
  if ( test -d .svn) then
    svn up
    if ( test $? -eq 0 ) then
      SVN_REVISION=`svn info | grep "Revision:" | awk '{print $2}'`
    else
      echo "Problem encountered updating your $1 svn checkout"
    fi
  else
    echo "$1 is not an svn repo"
  fi
  cd $savepath
}

function get_git_revision
{
  GIT_REVISION=""

  savepath=`pwd`
  cd $1
  if ( test -d .git) then
    git pull --rebase
    if ( test $? -eq 0 ) then
      GIT_REVISION=`git rev-parse --short HEAD`
    else
      echo "Problem encountered updating your $1 git checkout"
    fi
  else
    echo "$1 is not a git repo"
  fi
  cd $savepath
}

# $1: old file name
# $2: current SVN or Git revision
# $3: current date
function change_file_version
{
  NEWFILE=`cat $1 | \
           sed s/20[0-9][0-9][0-9][0-9][0-9][0-9]/$3/g | \
           sed s/\.[0-9a-z][0-9a-z][0-9a-z][0-9a-z][0-9a-z][0-9a-z][0-9a-z]\)/\.$2\)/g`
  echo "Changing version in file $1."
  echo "$NEWFILE" > $1
}

# $1: current Git revision
# $2: current date
function change_version_numbers
{
  CURDIR_FOR_VERSION_NUMBERS_CHANGE=`pwd | awk -F/ '{print $NF}'`
  if ( test "$CURDIR_FOR_VERSION_NUMBERS_CHANGE" != "kdepim" ) then
    echo "Wrong working directory, the current directory needs to be the kdepim source directory."
    return 1
  fi

  CURDIFF=`git diff --name-only`
  if ( test -n "$CURDIFF" ) then
    echo "$PWD is not clean!"
    echo "Cannot proceed with the tagging. Press <enter> to continue"
    read
    return 1
  fi

  change_file_version "kmail/kmversion.h" $1 $2
  change_file_version "kontact/src/main.cpp" $1 $2
  change_file_version "korganizer/version.h" $1 $2

  PAGER=cat git diff
  echo "Going to check in the above diff, press enter to continue or CTRL+C to abort."
  read
  git commit --message "GIT_SILENT: Update version numbers for today's release."
  git push
  return 0
}

# $1: Repo location
# $2: Target name e.g. akonadi or kdepim
# $3: Tag name
function addGitTag
{
  echo "Tagging $2 as $3"
  if ( test -d $1 ) then
    cd $1
    if ( test -d $2 ) then
      cd $2
      git tag -m "Tag $2 $3" "$3"
      git push --dry-run --tags
      echo "Going to push the above, press enter to continue or CTRL+C to abort."
      read
      git push --tags
      return 1
    else
      echo "No such directory \"$1/$2\" for Git repo"
    fi
  else
    echo "No such directory \"$1\" for Git repo"
  fi
  return 0
}

# $1: base tag name, e.g. enterprise35.0 or enterprise4.0
# $2: SVN branch name, e.g. "/branches/kdepim/enterprise/" or "/branches/kdepim/enterprise4/".
#     Will be used as the base for the branch names passed in in $3.
#     Name must include slashes
# $3: list of branch names to tag, e.g. "kdepim kdepimlibs"
# $4: Tag name
function addSvnTag
{
  cd $E3_BASE_SOURCE_DIR
  savePath=$PWD

  # Checkout the tag dir from SVN and create the dir for the current tag
  echo "Creating tag base dir..."
  rm -rf /tmp/tags-$1
  svn co -q -N $SVNBASE/tags/kdepim /tmp/tags-$1
  cd /tmp/tags-$1
  svn mkdir $4
  cd $4
  
  svn status
  echo "Going to check in the above diff, press enter to continue or CTRL+C to abort."
  read
  svn ci -m "SVN_SILENT Create tagging dir for today's $baseName tags."

  cd $savePath
  rm -rf /tmp/tags-$1

  echo "Now creating actual tag directories, press enter to continue or CTRL+C to abort."
  read
  tagTarget=$SVNBASE/tags/kdepim/$4
  for cb in $3
  do
    echo "Tagging in-source branch $CURBRANCH..."
    svn cp $SVNBASE$2$cb/ $tagTarget -m "SVN_SILENT Tag $CURBRANCH."
  done
}

#1: Tag name
function mailNag
{
  echo "OK, created tag $1. Don't forget to mail KKTI."

# echo a sample email message
  d="`date '+%d %B %Y'`"
  subject="This Week's E35 Tag ($d)"
  cat <<EOF
To: kolab-konsortium-tech-intern@kolab-konsortium.de
Subject: $subject
Howdy,

We created the following tags for enterprise35:

  git@git.kde.org/kdelibs $1
  git@git.kde.org/kdepim $1
  svn.kde.org/home/kde/tags/kdepim/$1/kde-l10n

The NewsLog.txt has been updated in the e35 branch.
Updated German translations are also included.

EOF
}

##### End Functions #####

check_environment_for_tagging_setup
if ( test $? -eq 1 ) then
  exit
fi

# Start doing the work
get_date
get_time

tagName="$baseName.$DATE.$TIME"
echo "Going to tag $tagName"
echo
echo "Before continuing, please double-check:"
echo "- that it builds ok"
echo "- that the translations are up-to-date"
echo "- that the NewsLog.txt is up-to-dated"
echo "Press enter to continue or CTRL+C to abort."
read

get_git_revision $HOME/projects/e3/kdepim
KDEPIM_REVISION="$GIT_REVISION" #will be used in application version numbers

get_git_revision $HOME/projects/e3/kdelibs
KDELIBS_REVISION="$GIT_REVISION" #not used at this time

get_svn_revision $E3_BASE_SOURCE_DIR/kde-l10n
L10N_REVISION="$SVN_REVISION" #not used at this time

if ( test "$KDEPIM_REVISION" -a \
          "$KDELIBS_REVISION" -a \
          "$L10N_REVISION" ) then
  change_version_numbers $GIT_REVISION $DATE
  if ( test $? -eq 0 ) then
    addGitTag $HOME/projects/e3 kdelibs $tagName
    addGitTag $HOME/projects/e3 kdepim $tagName
    addSvnTag $baseName "/branches/kdepim/enterprise/" "kde-l10n" $tagName

    mailNag $tagName
  fi
fi
