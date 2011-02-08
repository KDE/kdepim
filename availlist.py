#!/usr/bin/env python
# Tested with python2 and python3

import os

COMMITS_IN_CURRENT_BRANCH = []

# prints debug info about this commit
#COMMIT_TO_DEBUG = 'bf77b149ca72d1591cd4e054dca4571d81a52f74'
COMMIT_TO_DEBUG = []

def run_git_branch_contains( commit ):
    return os.popen( 'git branch --contains ' + commit ).readlines()

def run_git_notes_list():
    return os.popen( 'git notes list' ).readlines()

def run_git_show( commit, include_notes ):
    if include_notes:
        return os.popen( 'git show --name-only ' + commit ).readlines()
    else:
        return os.popen( 'git show --no-notes --name-only ' + commit ).readlines()

def run_git_branch():
    return os.popen( 'git branch' ).readlines()

def run_git_log( branch ):
    return os.popen( 'git log --format="%H" ' + branch ).readlines()

# returns current branch
def current_branch():
    lines = run_git_branch()
    for line in lines:
        if line.startswith( '* ' ):
            branch = line.strip( '* ' ).strip()
            # print( "Current branch is " + branch )
            return branch
    return ""

# Returns True if this is a PENDING note.
def is_relevant_note( note_lines ):
    for note_line in note_lines:
        if 'PENDING' in note_line:
            return True
    return False

# This method is expensive. git branch --contains is slow. Using git-log instead.
# returns True if branch contains commit
#def branch_contains( branch, commit ):
#    lines = run_git_branch_contains( commit )
#    return ( "* " + branch + "\n" in lines ) or ( branch + "\n" in lines )

def branch_contains( branch, commit ):
    global COMMITS_IN_CURRENT_BRANCH
    if not COMMITS_IN_CURRENT_BRANCH:
        COMMITS_IN_CURRENT_BRANCH = run_git_log( branch )

    contains = commit + "\n" in COMMITS_IN_CURRENT_BRANCH

    if commit == COMMIT_TO_DEBUG:
        print( "DEBUG: branch_contains, " + commit + ', contains=' + str(contains) + ', branch=' + branch )

    return contains

# Parses "git notes list" output and returns a list of commits with notes
def get_objects_with_notes():
    git_notes_output = run_git_notes_list()

    result = []
    debug = 0
    for line in git_notes_output:
        debug = debug + 1
        line = line.strip() #remove '\n'
        splited_line = line.split()
        if len(splited_line) == 2:
            branch = current_branch()
            commit = splited_line[1]
            # print( "Checking " + str(debug) + "/" + str(len(git_notes_output)) + " " + commit )
            if branch_contains( branch, commit ):
                result.append( commit )
        else:
            print( "Invalid line: " + line )

    if COMMIT_TO_DEBUG in result:
        print( "DEBUG get_objects_with_notes has " + COMMIT_TO_DEBUG )

    return result

# Returns the avail list
def get_avail_list():
    commits = get_objects_with_notes()

    result = []
    separator = '---------------------------------------------------------------------'
    print( separator )
    for commit in commits:
        lines = run_git_show( commit, True )
        if is_relevant_note( lines ):
            if commit == COMMIT_TO_DEBUG:
                print( "DEBUG: get_avail_list: Found " + COMMIT_TO_DEBUG )
            result = result + run_git_show( commit, False )
            result.append( separator )
        else:
            if commit == COMMIT_TO_DEBUG:
                print( "DEBUG get_avail_list: Discarded " + COMMIT_TO_DEBUG )
    return result, len(commits)

# Prints the avail list
def print_avail_list():
    lines, commit_count = get_avail_list()
    for line in lines:
        print( line.strip() )
    print
    print( "There are " + str(commit_count) + " commits to forwardport. Good luck." )


print_avail_list()