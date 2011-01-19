# -*-python-*-
# vim: ts=4

import os, re

red = "^[[2;31m"
green = "[2;32m"
blue = "[34m"
yellow = "[1;33m"
cyan = "[36m"
normal = "[0m"
progress_color="[2;35m"
bold = "[0m"
compile_text = green + "compiling" + bold
creating_text = blue + "creating" + bold
linking_text = yellow + "linking" + bold
installing_text = cyan + "installing" + bold
uninstalling_text = cyan + "uninstalling" + bold

def clearAllColors():
	global compile_text,creating_text,linking_text,installing_text,uninstalling_text,progress_color,normal
	compile_text = "compiling"
	creating_text = "creating"
	linking_text = "linking"
	installing_text = "installing"
	uninstalling_text = "uninstalling"
	progress_color = ""
	normal = ""

variablere = re.compile('\$[{(]([^({})]*)[})]$')
variablesre = re.compile('(.*?)\$[{(]([^({})]*)[})](.*)')
autoconfre = re.compile('(.*?)@([^@]*)@(.*)')
extre = re.compile('(.*)(\.[^.]*)$')
definere = re.compile('\s*([^+=\s]*)\s*(\+?)=(.*)$')
cppext = [".cpp", ".cc", ".C", ".cxx", ".c++"]
hext = [".h", ".H", ".hh", ".hxx", ".hpp", ".h++"]

_topsrcdir = None
top_builddir = None
top_builddir_abs = None
subst_vars = {}
environment_vars = {}
true_conds = []
false_conds = []

configure_in = None
our_path = None

__pychecker__ = 'unusednames=_topsrcdir'

def list_source_files(dirname):
	cppfiles = []
	hfiles = []
	for myfile in os.listdir(dirname):
		match = extre.match(myfile)
		if not match:
			continue
		base = match.group(1)
		ext = match.group(2)

		if not ext in cppext and not ext in hext:
			continue
		if base.endswith('.moc') or len(base) == 0 or base[0] == '.':
			continue
		found = 0
		for cpp_ext in cppext:
			all_cpp = ".all_%s.%s" % (cpp_ext, cpp_ext)
			if myfile.endswith(all_cpp):
				found = 1
		if found:
			continue
		if ext in cppext:
			cppfiles.append((base, ext))
		else:
			hfiles.append((base, ext))

	return (cppfiles, hfiles)

default_force = 1

def write_if_changed(filename, lines, force = 0):
	# I'm not convinced any longer we can make use of this
	force = default_force
	if not force and os.path.exists(filename):
		index = 0
		while index < len(lines):
			if string.find(lines[index], '\n', 0, -1) != -1:
				split = string.split(lines[index], '\n')
				if not len(split[-1]):
					split = split[:-1]
				for si in range(0, len(split)):
					split[si] = split[si] + '\n'
				lines[index:index+1] = split
				index = index + len(split)
			else:
				index = index + 1
		new_lines = open(filename).readlines()
		if new_lines == lines:
			return
		else:
			print filename, 'changed'
			pass
	
	open(filename, 'w').writelines(lines)

import string

def parse_autoconf(full=0):
	tracelines = []

	configure_file = "configure.in"
	if os.path.exists("configure.ac"):
		configure_file = "configure.ac"
	tracevalid = not full and os.path.exists(".autoconf_trace")
	if tracevalid and os.path.getmtime(configure_file) > \
	   os.path.getmtime(".autoconf_trace"):
		tracevalid = 0

	if tracevalid and os.path.getmtime("aclocal.m4") > \
	   os.path.getmtime(".autoconf_trace"):
		tracevalid = 0

	if tracevalid:
		tracelines = open( ".autoconf_trace" ).readlines()
			
	if not len(tracelines):
		autoconf = os.environ.get('AUTOCONF', 'autoconf')
		traces = autoconf + " -t AC_SUBST -t AC_CONFIG_FILES -t AC_CONFIG_AUX_DIR -t AC_CONFIG_HEADERS"
		tracelines = os.popen( traces ).readlines()
		if not len(tracelines):
			print 'couldn\'t call', traces
			sys.exit(1)
		open(".autoconf_trace", 'w').writelines(tracelines)
		
	subst = {}
	files = []
	headers = []
	auxdir = '.'
	for line in tracelines:
		splitted = string.split(string.strip(line), ':')
		if splitted[2] == 'AC_SUBST':
			subst[splitted[3]] = ""
		elif splitted[2] == 'AC_CONFIG_FILES':
			files.extend( string.split( splitted[3] ) )
		elif splitted[2] == 'AC_CONFIG_HEADERS':
			headers.extend( string.split( splitted[3] ) )
		elif splitted[2] == 'AC_CONFIG_AUX_DIR':
			auxdir = splitted[3]
		else:
			print splitted, 'doesn\'t contain the traced macros'

	if auxdir == '.':
		return subst, files, headers, '$(top_srcdir)'
	else:
		return subst, files, headers, '$(top_srcdir)/' + auxdir

def canon_name(name):
	return re.sub('[^a-zA-Z0-9_]', '_', name)

sys_exit_code = 0

import sys

def print_error(string):
	sys.stderr.write("ERROR:")
	sys.stderr.write(string)
	global sys_exit_code
	sys_exit_code = 1
	
