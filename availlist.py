#!/usr/bin/env python
# Tested with python2 and python3

import os

def run_git_notes_list():
    return os.popen( 'git notes list' ).readlines()

def run_git_show( commit, include_notes ):
    if include_notes:
        return os.popen( 'git show --name-only ' + commit ).readlines()
    else:
        return os.popen( 'git show --no-notes --name-only ' + commit ).readlines()

# Returns True if this is a PENDING note.
def is_relevant_note( note_lines ):
    for note_line in note_lines:
        if 'PENDING' in note_line:
            return True
    return False

# Parses "git notes list" output and returns a list of commits with notes
def get_objects_with_notes():
    git_notes_output = run_git_notes_list()

    result = []
    for line in git_notes_output:
        line = line.strip() #remove '\n'
        splited_line = line.split()
        if len(splited_line) == 2:
            result.append( splited_line[1] )
        else:
            print( "Invalid line: " + line )

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
            result = result + run_git_show( commit, False )
            result.append( separator )
    return result

# Prints the avail list
def print_avail_list():
    lines = get_avail_list()
    for line in lines:
        print( line.strip() )


print_avail_list()