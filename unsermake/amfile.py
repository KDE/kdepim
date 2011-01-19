
import utilities
import re, os, types, string, sys
import handlerbase, program
from target import Target, PhonyConflict, DefValue

__pychecker__ = 'no-shadowbuiltin'

maybe_missing =  {}
find_missing_deps = False
found_missing_deps = {}

ignore_errors = False

class AMFile:
	def __init__(self, filename, ignore_comments = True, create_empty = False):

		self.targets = {}
		self.defs = {}
		self.printed_defs = {}
		self.conds = {}
		self.dep_files = []
		self.cached_defs = {}
		
		if utilities._topsrcdir and filename.startswith(utilities._topsrcdir):
			self.filename = filename[ len(utilities._topsrcdir) : ]
		elif utilities.top_builddir_abs and filename.startswith(utilities.top_builddir_abs):
			self.filename = filename[ len(utilities.top_builddir_abs) + 1: ]
		else:
			self.filename = filename

		self.configure_files = []

		# this array collects defines that were changed after initial parsing
		self.overwrites = []

		self.commentre = re.compile('\s*#.*')
		self.to_handle_targets = []
		self.am_includes = []
		self.binaries = {}
		self.mansectre = re.compile('.*\.([0-9nl])[a-z]*')
		self.mansections = ['0','1','2','3','4','5','6','7','8','9','n','l']
		self.first_target = None

		try:
			self.parse(filename, ignore_comments, create_empty)
		except IOError, e:
			print filename, "caused error:", e
			sys.exit(1)

		self.overwrites = []
		self.isempty = create_empty

	def __repr__(self):
		return self.subdir
		
	def addTarget(self, target):
		if self.targets.has_key(target.target):
			# throws if both have rules
			try:
				self.targets[target.target].merge(target)
			except PhonyConflict:
				utilities.print_error("%s tries to overwrite the PHONY status of %s\n" % (self.filename, target))
		else:
			self.targets[target.target] = target

	def insertTarget(self, target, deps, rules = [], user_specified=0,
					 phony=0, compile_target=0):
		if not len(rules) and self.targets.has_key(target):
			target = self.targets[target]
			if type(deps) == types.StringType:
				deps = string.split(deps)
				for dep in deps:
					target.deps.append(DefValue(dep))
			else:
				for dep in deps:
					if type(dep) == types.StringType:
						# we can not assume it's an expanded value
						target.deps.append(DefValue(dep))
					else:
						target.deps.append(dep)

			return
			
		# throws error if a line doesn't start with <tab>
		t = Target(target, deps, rules, user_specified, phony)
		t.compile_target = compile_target
		self.addTarget(t)

	def print_out_definition(self, output, definition):
		if self.printed_defs.has_key(definition):
			return
		self.set_def_printed(definition)
			
		if not self.is_defined(definition):
			# print '%s: %s is not defined but used' % (self.filename, definition)
			return

		if self.defs.has_key(definition):
			for var in self.defs[definition]:
				match = utilities.variablere.match(var)
				if match:
					self.print_out_definition(output, match.group(1))
			rhs = string.join(self.defs[definition])
		
			output.append('%s = %s\n' % (definition, rhs))
		else:
			cond = self.conds[definition]
			for var in cond[1] + cond[2]:
				match = utilities.variablere.match(var)
				if match:
					self.print_out_definition(output, match.group(1))

			output.append('if %s\n' % cond[0])
			output.append('%s = %s\n' % (definition, string.join(cond[1])))
			output.append('else\n')
			output.append('%s = %s\n' % (definition, string.join(cond[2])))
			output.append('endif\n')

	def print_out_target(self, output, target):
		defs = self.targets[target].used_defines()
		for definition in defs:
			self.print_out_definition(output, definition)
		self.targets[target].print_out(output)

	def target(self, target):
		if self.targets.has_key(target):
			return self.targets[target]
		return None

	def defines(self):
		return self.defs.keys() + self.conds.keys()
	
	def is_target(self, target):
		return self.targets.has_key(target)
	
	def is_defined(self, variable):
		return self.defs.has_key(variable) or self.conds.has_key(variable)

	def del_define(self, variable):
		#return self.defs.remove(variable)
		if self.defs.has_key(variable):
			del self.defs[variable]

	def add_define(self, variable, value, cond = "", cond_true=1):

		if type(value) == types.StringType:
			value = string.split(string.strip(string.replace(value, '\001', ' ')))

		if not len(cond):
			if not variable in self.overwrites:
				self.overwrites.append(variable)
			if self.defs.has_key(variable):
				self.defs[variable].extend(value)
			else:
				self.defs[variable] = value
		else:
			if cond_true:
				if self.conds.has_key(variable):
					print '%s: %s is redefined in automake conditional' % (self.filename,
																		   variable)
				self.conds[variable] = (cond, value, [])
			else:
				if not self.conds.has_key(variable):
					self.conds[variable] = (cond, [], value)
				if not self.conds[variable][0] == cond:
					utilities.print_error('%s: %s defined in two different conditionals\n' % (self.filename,
																						 variable))
					return
				cond = self.conds[variable]
				self.conds[variable] = (cond[0], cond[1], value)

	def add_prefixed_variable(self, var, replace_srcdir=0):
		if not self.defs.has_key(var):
			# not defined here (may be subst_vars)
			return

		if var in self.overwrites:
			# already defined
			return
		if self.conds.has_key(var):
			list1 = self.replace_srcdir(self.conds[var][1])
			list2 = self.replace_srcdir(self.conds[var][2])

			self.conds[var] = (self.conds[var][0], list1, list2)
			self.overwrites.append(var)
			return
		orig = self.definition(var)
		rec = self.definition_rec(var)
		if replace_srcdir:
			rec = self.replace_srcdir(rec)
		if orig != rec:
			self.del_define(var)
			self.add_define(var, rec)

	def _var_rec_var(self, str):

		if not len(str):
			return ['']
		
		val = utilities.variablesre.match(str)
		if val:
			if not len(val.group(1)) and not len(val.group(3)):
				return self._var_rec_priv_(val.group(2))
			else:
				ret = self._var_rec_priv_(val.group(2))
				if len(ret) > 1:
					utilities.print_error('%s: variable \'%s\' is used in another variable, but contains a list\n' % (self.filename, val.group(2)))
					return [str]
				if not len(ret):
					return [str]

				suffix = self._var_rec_var(val.group(3))
				prefix = self._var_rec_var(val.group(1))
				assert(len(prefix) == 1)
				assert(len(suffix) == 1)
				retstr = prefix[0] + ret[0] + suffix[0]

				if utilities.subst_vars.has_key(val.group(2)):
					return [retstr]
				else:
					return self._var_rec_var(retstr)
		else:
			if str[0] == '@' and str[-1] == '@':
				str = str[1:-1]
				str = utilities.subst_vars[str]
				return [str]
		return [str]
					
	def _var_rec_priv_(self, variable):
		var_list = self.definition(variable)
		if not len(var_list):
			if variable in utilities.subst_vars.keys() or self.conds.has_key(variable):
				return ["$(%s)" % variable]
			else:
				return []

		return_list = []
		
		if self.circle.has_key(variable):
			sys.stderr.write("Warning: %s produces a circle.\n" % variable)
			return return_list

		self.circle[variable] = 1
	
		for entry in var_list:
			return_list.extend(self._var_rec_var(entry))

		del self.circle[variable]
		return return_list

	def definition(self, variable):
		if self.defs.has_key(variable):
			return self.defs[variable]
		else:
			return []
		
	def value_of(self, variable):
		if self.defs.has_key(variable):
			return string.join(self.defs[variable])
		return None

	def value_list(self, variable):
		if self.defs.has_key(variable):
			return self.definition_rec(variable)
		return []

	def definition_rec(self, variable):
		self.circle = {}
		ret = self._var_rec_priv_(variable)
		del self.circle
		return ret

	def add_makefile_rules(self):
		if self.subdir == '.':
			prefix = ""
		else:
			prefix = self.subdir + "/"

		self.insertTarget(self.build + 'Makefile',
						  ['$(top_builddir)/config.status', self.source + "Makefile.in"],
						  ["cd $(top_builddir) && " +
						   "$(SHELL) ./config.status " + prefix + "Makefile"])
		self.insertTarget("all",  self.build + "Makefile", phony=1)

		
	def set_configure_files(self, files):
		for _file in files:
			if not _file in self.configure_files:
				self.configure_files.append(_file)
			if self.subdir == '.':
				pfile = _file
			else:
				pfile = self.subdir + "/" + _file
			self.insertTarget(self.build + _file,
							  ['$(top_builddir)/config.status',
							   '$(srcdir)/' + _file + '.in'],
							  ["\tcd $(top_builddir) && " +
							   "$(SHELL) ./config.status " + pfile])
		self.add_makefile_rules()

	def set_configure_headers(self, files, stamp_counter):
		cleanrules = []
		for file in files:
			stamp = 'stamp-h%d' % stamp_counter

			self.insertTarget(self.build + file, self.build + stamp,
									['@if test ! -f %s; then \\' % (self.build + file),
									 '   rm -f %s; \\' % (self.build + stamp),
									 '   cd %s && $(MAKE) %s; \\' % (self.build, stamp),
									 'else :; fi'])

			config_status_argument = '%s/%s' % (self.subdir, file)
			if self.subdir == '.':
				config_status_argument = file
				
			self.insertTarget(self.build + stamp,
									[self.source + file + '.in',
									 '$(top_builddir)/config.status'],
									['@rm -f %s' % (self.build + stamp),
									 'cd $(top_builddir) && $(SHELL) ./config.status %s' % config_status_argument])

			if stamp_counter == 1:
				self.insertTarget(self.source + file + '.in', [ utilities.configure_in ],
								  ['cd $(top_srcdir) && $(AUTOHEADER)',
								   'rm -f ' + self.build + stamp,
								   'touch ' + self.source + file + '.in'])

			cleanrules.append('-rm -f ' + self.build + file + ' ' + self.build + stamp)
			stamp_counter += 1
		if cleanrules:
			self.insertTarget('distclean-hdr-%s' % self.canon_subdir, [],
					  cleanrules, phony=1)
			self.insertTarget('distclean', 'distclean-hdr-%s' % self.canon_subdir, phony=1)
			
		
		return stamp_counter
			
	def final_reorder(self):
		self.insertTarget('.SUFFIXES', '')
		for target in self.targets.values():
			if target.is_phony and not target.target == '.PHONY':
				self.insertTarget('.PHONY', target.target, phony=1)

	def cache_def(self, var, value):
		value = self.expand(value)
		self.cached_defs[var] = value
		return value

	def cache_def_list(self, var, list):
		ret = ""
		for item in list:
			ret += self.expand(item) + " "
		if ret and ret[-1] == ' ':
			ret = ret[:-1]
		self.cached_defs[var] = ret
		return ret
		
	def expand(self, file):
		match = utilities.variablesre.match(file)
		if not match:
			return file
		var = match.group(2)
		begin = match.group(1)
		if len(begin) and begin[-1] == '$':
			# if it ends with a $, then the variable is escaped
			return file[:match.end(2)] + self.expand(file[match.end(2):])
		# TODO: while this cache saves us at least 30% recursions to expand
		# the check also costs considerably time. But we can't put the expanded
		# values into .defs that easily as INSTALL_DATA should not be collected
		# as primary and we can't put cached subst_vars back in subst_vars
		# as the variables _could_ expand depending on the Makefile's .defs
		if self.cached_defs.has_key(var):
			return begin + self.cached_defs[var] + self.expand(match.group(3))
		if self.defs.has_key(var):
			return begin + self.cache_def_list(var, self.defs[var]) + self.expand(match.group(3))
		if utilities.subst_vars.has_key(var):
			return begin + self.cache_def(var, utilities.subst_vars[var]) + self.expand(match.group(3))
		if os.environ.has_key(var):
			return begin + self.cache_def(var, os.environ[var]) + self.expand(match.group(3))
		# print "DEBUG:", self.filename, "variable", var, "not found."
		return begin + self.expand(match.group(3))

	def read_in(self, filename, ignore_comments):
		# read the file
		try:
			lines = open(filename, 'r').readlines()
		except IOError, e:
			print "No such file: ", filename
			raise e
			sys.exit(1)	
		ret = []
		for line in lines:
			if not len(line):
				continue
			if ignore_comments and line[0] != '\t':
				try:
					index = string.index(line, '#')
					if index == 0:
						continue
					line = line[:index]
				except ValueError:
					pass
			if line.startswith('include '):
				line = string.strip(line[8:])
				if line.startswith('$(top_srcdir)'):
					file = os.path.join(utilities._topsrcdir, line[14:])
				else:
					file = os.path.join(os.path.dirname(filename), line)
				ret.extend(self.read_in(file, ignore_comments))
				self.am_includes.append(line)
			else:
				ret.append(line)
		return ret
		
	def parse(self, filename, ignore_comments, create_empty):
		if len(filename) and not create_empty:	
			self.lines = self.read_in(filename, ignore_comments)
		else:
			self.lines = []
		self.dirname = os.path.dirname(filename)
		self.subdir = os.path.dirname(self.filename)

		if utilities.top_builddir:
			self.asubdir = os.path.abspath(utilities.top_builddir + "/" + self.subdir) 
		else:
			self.asubdir = "."
	
		if not len(self.subdir):
			self.subdir = '.'
			self.canon_subdir = 'top'
		else:
			self.canon_subdir = 'top_' + utilities.canon_name(self.subdir)
			
		# sharing some variables
		self.source = '$(top_srcdir)/'
		self.build = '$(top_builddir)/'
		if self.subdir != '.':
			self.source += self.subdir + "/"
			self.build += self.subdir + "/"
		
		# add a final line feed to be sure we have it
		if len(self.lines) == 0:
			self.lines = ["\n"]
		if not self.lines[-1][-1] == '\n':
			self.lines.append('\n')

		# concatinate lines with backslash (replaced with \001 for later
		# reference). Automake comments are skiped
		index = 0
		ebre = re.compile('\\\[ \t]*\n')
		while index < len(self.lines):
			if self.lines[index].startswith('##'):
				del self.lines[index]
				continue
			if ebre.search(self.lines[index]):
				if index + 1 < len(self.lines):
					self.lines[index:index+2] = [ ebre.sub('\001', self.lines[index], 1) + self.lines[index + 1] ] 
				else:
					del self.lines[index]
			else:
				index = index + 1

		self.find_targets()
		self.find_defines()
		self.find_opts()
		
		for line in self.lines:
			if len(string.strip(line)) and not self.commentre.match(line):
				sys.stderr.write("%s: rest %s\n" % (self.filename, line))

		# the targets allowed to be defined not-PHONY
		for target in ['install-exec', 'install-data', 'all', "uninstall", "check"]:
			targ = self.target(target + '-hook')
			if targ:
				targ.is_phony = 1
			targ = self.target(target + '-local')
			if targ:
				targ.is_phony = 1

		if self.targets.has_key(".FORWARDS"):
			targets = self.targets[".FORWARDS"].deps
			del self.targets[".FORWARDS"]
			for target in targets:
				# assuming forwards do not contain variables
				del self.targets[target.value]

		for handler in handlerbase.handlers:
			handler.parse(self)
			
	def replace_autoconf(self, line):
		match = utilities.autoconfre.match(line)
		if match:
			if utilities.subst_vars.has_key(match.group(2)):
				return match.group(1) + utilities.subst_vars[match.group(2)] + self.replace_autoconf(match.group(3))
			else:
				return match.group(1) + '@' + match.group(2) + self.replace_autoconf('@' + match.group(3))

		return line
	
	def find_defines(self):

		current_cond = ""
		current_cond_true = 1
		index = 0
		while index < len(self.lines):
			line = self.lines[index]
			if self.commentre.match(line):
				index = index + 1
				continue
			define = utilities.definere.match(line)
			if define:
				varname = define.group(1)
				value = define.group(3)
				if string.find(value, '@') != -1:
					value = self.replace_autoconf(value)
				if define.group(2) == '+':
					if not self.is_defined(varname):
						utilities.print_error("%s: %s += used before assigned value to the variable\n" % (self.filename, varname))
					elif current_cond:
						if current_cond in utilities.true_conds:
							if current_cond_true:
								self.add_define(varname, value)
						elif current_cond in utilities.false_conds:
							if  not current_cond_true:
								self.add_define(varname, value)
						else:
							self.add_define(varname, value)
					else:
						self.add_define(varname, value)
				else:
					if current_cond:
						if current_cond in utilities.true_conds:
							if current_cond_true:
								self.add_define(varname, value)
							else:
								self.add_define(varname, [])
						elif current_cond in utilities.false_conds:
							if not current_cond_true:
								self.add_define(varname, value)
							else:
								self.add_define(varname, [])
						else:
							self.add_define(varname, value, current_cond, current_cond_true)
					else:
						if self.is_defined(varname):
							utilities.print_error("%s: %s defined twice\n" % (self.filename, varname))
						self.add_define(varname, value, current_cond, current_cond_true)
						
				self.lines[index:index+1] = []
			elif line.startswith('if '):
				if len(current_cond):
					utilities.print_error('%s: nested automake conditionals '
								'(%s vs. %s)\n' % (self.filename,
												   current_cond,
												   string.strip(line[3:])))
					return
				current_cond = string.strip(line[3:])
				current_cond_true = 1
				self.lines[index:index+1] = []
			elif line.startswith('else'):
				current_cond_true = 0
				self.lines[index:index+1] = []
			elif line.startswith('endif'):
				current_cond = ""
				self.lines[index:index+1] = []
			else:
				index = index + 1

	def find_targets(self):
		targetre = re.compile(' *([^:]*)\s*:\s*(.*)')

		index = 0
		while index < len(self.lines):
			line = self.lines[index]
			if self.commentre.match(line):
				index = index + 1
				continue
			# it's a croax - defines can contain : chars too
			if utilities.definere.match(line):
				index = index + 1
				continue
			targmatch = targetre.match(line)
			if targmatch:
				targets = string.split(targmatch.group(1))
				deps = re.sub('#.*', '',  targmatch.group(2))
				deps = string.replace(deps, '\001', '')
				newindex = index + 1
				while newindex < len(self.lines):
					if self.lines[newindex][0] == '\t':
						newindex = newindex + 1
						continue
					# white space only lines count as comments too
					if not len(string.strip(self.lines[newindex])) or self.commentre.match(self.lines[newindex]):
						newindex = newindex + 1
						continue
					break
				for target in targets:
					if not self.first_target:
						self.first_target = target
					self.insertTarget(target, deps, self.lines[index+1:newindex], 1)
				self.lines[index:newindex] = []
			else:
				index = index + 1
		target = self.target('.PHONY')
		if target:
			target.is_phony = 1
			for targ in target.deps:
				if isinstance(targ, DefValue):
					targ = self.expand(targ.value)
				otarg = self.target(targ)
				if not otarg:
					utilities.print_error("%s: target '%s' marked PHONY but isn't present\n" % (self.filename,
																					  targ))
				else:
					otarg.is_phony = 1

	# does some basic checking for things that often break
	def check(self):
		for define in self.defines():
			if define.endswith('_LDFLAGS') or define == 'LDFLAGS':
				list = self.definition_rec(define)
				for str in list:
					if str.endswith('.la') or str.endswith('.lo'):
						utilities.print_error('%s: LDFLAGS contains libtool\n'
									'\tfile %s. These belong in LIBADD\n'
									'\t(for libs) or LDADD (for programs)\n' %
									(self.filename, str))
						continue
					if str.startswith('-L .') or str.startswith('-L.'):
						utilities.print_error('%s: seen -L. in your LDFLAGS. You shouldn\'t\n'
									'use -L relative paths but use relative paths to\n'
									'.la files in LDADD/LIBADD\n' % (self.filename))
						continue
			if define.endswith('_LIBADD') or define.endswith('_LDADD'):
				list = self.definition_rec(define)
				for str in list:
					if str.endswith('.la') and not os.path.basename(str).startswith('lib'):
						print('%s: %s contains a module. This is not portable!\n'
							  '     You can only link against libraries.' % (self.filename, define))
						continue
						

	def replace_srcdir(self, list):
		res = []
		Ire = re.compile('^-I([^/\$].*)')
		index = 0
		while index < len(list):
			l = list[index]
			if l == '-I':
				print('%s: found single -I argument. This is not portable.' % self.filename)
				list[index+1] = l + list[index+1]
				del list[index]
				continue
			l = re.sub('^-I\$\(srcdir\)', '-I' + self.source[:-1], l)
			match = Ire.match(l)
			if match:
				# strip the latest / - it looks ugly, no other reason
				if match.group(1) == '.':
					l = '-I' + self.build[:-1]
				else:
					l = '-I' + self.build + match.group(1)
			else:
				match = utilities.variablesre.match(l)
				if match and self.conds.has_key(match.group(2)):
					var = match.group(2)
					if not var in self.overwrites:
						list1 = self.replace_srcdir(self.conds[var][1])
						list2 = self.replace_srcdir(self.conds[var][2])

						self.conds[var] = (self.conds[var][0], list1, list2)
						l = '$(%s)%s' % (var, match.group(3))
					else:
						l = '$(%s)%s' % (var, match.group(3))
					self.overwrites.append(var)
						
			res.append(l)
			index = index + 1
		return res
	
	def create(self):

		self.default_includes = string.join(self.replace_srcdir(['$(DEFS)', '-I.', '-I$(srcdir)', '-I$(top_builddir)']))
		self.add_define(self.canon_subdir + '_srcdir', '$(srcdir)')
		self.add_define(self.canon_subdir + '_builddir', '.')
		utilities.subst_vars[self.canon_subdir + '_srcdir'] = ''
		utilities.subst_vars[self.canon_subdir + '_builddir'] = ''

		self.find_binaries()
		
		for handler in handlerbase.handlers:
			handler.create(self)

		finals = []
		for prog in self.binaries.values():
			prog.create_variables()
			
			sources = self.definition_rec( prog.canon_name + '_SOURCES' )
			index = 0
			while index < len(sources):
				source = sources[index]
				match = utilities.variablere.match(source)
				if match and self.conds.has_key(match.group(1)):
					cond = self.conds[match.group(1)]
					# if we don't know the value, we take both 
					sources.extend(cond[1])
					sources.extend(cond[2])
					del sources[index]
				else:
					index += 1
			
			for source in sources:
				match = utilities.extre.match(source)
				if not match:
					match = utilities.variablere.match(source)
					print match
					if match and self.conds.has_key(match.group(1)):
						print match.group(1)
					utilities.print_error('%s: "%s" doesnt match extre\n' % (self.filename, source))
					continue
				base = match.group(1)
				ext = match.group(2)

				if ext in utilities.hext:
					continue
				
				if handlerbase.ext_dict.has_key(ext):
					handlerbase.ext_dict[ext].create_source(prog, base, ext)
				elif prog.is_cpp(ext) or ext == '.c':
					pass
				else:
					utilities.print_error('%s: unknown source extension %s for %s\n' % (self.filename, ext, prog.name))
			finals.append(prog.name)

		if len(finals):
			self.insertTarget("final", [],
							  utilities.our_path +
							  " -C $(top_builddir)/%s UNSERMAKE_FORCE_FINAL=1 " % self.subdir +
							  string.join(finals), phony=1)
		else:
			self.insertTarget("final", [],
							  ["@echo no programs available to recreate as final. Call in subdirs"],
							  phony=1)
	
	def set_def_printed(self, definition):
		self.printed_defs[definition] = 1

	def in_srcdir(self, file):
		return os.path.exists(os.path.join(self.dirname, file))
	
	def in_builddir(self, file):
		if utilities.top_builddir:
			path = os.path.abspath(utilities.top_builddir + "/" + self.subdir)
		else:
			path = "./" + self.subdir
		return os.path.exists(os.path.join(path, file))
	
	def userdirs(self):
		try:
			return self.userdirs
		except AttributeError:
			self.userdirs = []
			for key in self.defines():
				if key.endswith('dir'):
					self.userdirs.append(key[:-3])
			return self.userdirs


	def dirprefixes(self):
		# These are the directory names, which can be written
		# as variable prefixes (e.g. xxx_SOURCES, with xxx being one of the
		# dirs)
		# it's a subset of automake's recognized directories
		list = ["bin", "sbin", "libexec", "data", "sysconf", "sharedstate",
				"localstate", "lib", "info", "include", "man",
				"pkglib", "pkginclude", "pkgdata"]
		list.extend( self.userdirs() )
		return list

	def primaries(self):
		return ["PROGRAMS", "LIBRARIES", "SCRIPTS", "DATA", "HEADERS", "MANS",
				"TEXINFOS", "LTLIBRARIES"]

	def instance_binary(self, entry, prefix, type):
		prog = program.Program(self, entry, prefix, type)
		self.binaries[prog.name] = prog
						  
	# This finds all xxx_{PROGRAMS,{LT}LIBRARIES} and their sources
	def find_binaries(self):
	
		progre = re.compile('(.*)_(PROGRAMS|(LT)?LIBRARIES)')
		
		for key in self.defines():
			match = progre.match(key)
			if not match:
				continue

			prefix = match.group(1)
			suffix = match.group(2)

			type = program.program_type(suffix)

			for entry in self.definition_rec(key):
				match = utilities.variablere.match(entry)
				if not match:
					self.instance_binary(entry, prefix, type)
				else:
					var = match.group(1)
					if not self.conds.has_key(var):
						utilities.print_error('%s: variable %s is not automake conditional, but can\'t be expanded\n' %
									(self.filename, var))
						continue
					for l in self.conds[var][1]:
						self.instance_binary(l, prefix, type)
					for l in self.conds[var][2]:
						self.instance_binary(l, prefix, type)

	def find_opts(self):
		self.options = {}
		self.options["foreign"] = 0
		self.options["qtonly"] = 0
		self.options["noautodist"] = 0
		self.options["foreign-libtool"] = 0
		self.options["nofinal"] = 0
		self.options["doxygen"] = 0
		for option in self.definition("KDE_OPTIONS"):
			self.options[option] = 1
	
	def get_opt(self, opt):
		try:
			return self.options[opt]
		except:
			utilities.print_error('No such option named: %s\n' % opt)
			return 0
			
	def print_out(self, force):
		
		self.printed_defs = {}
	
		output = []
		
		output.append("# Makefile.in generated by unsermake\n")
		output.append("####################################\n")
		output.append("\n")
		
		if self.subdir != '.':
			builddir = '..'
			slashes = string.count(self.subdir, '/')
			for dummy in range(slashes):
				builddir = builddir + '/..'
			output.append('top_builddir = %s\n' % builddir)
		else:
			output.append('top_builddir = .\n')
			
		self.printed_defs["subdir"] = 1
		self.printed_defs["srcdir"] = 1
		self.printed_defs["top_builddir"] = 1
		self.printed_defs["distdir"] = 1
		self.printed_defs["top_distdir"] = 1
		self.printed_defs["INSTALL"] = 1
		
		if self.canon_subdir != "top":
			output.append("top_srcdir = @top_srcdir@\n")
		output.append("srcdir = @srcdir@\n")
		self.printed_defs[self.canon_subdir + "_srcdir"] = 1
		self.printed_defs[self.canon_subdir + "_builddir"] = 1
		self.printed_defs["top_srcdir"] = 1

		# these are the variables the toplevel Makefile _has_ to write
		self.printed_defs[self.canon_subdir + "_srcdir"] = 1
		self.printed_defs[self.canon_subdir + "_builddir"] = 1

		for var in utilities.subst_vars.keys() + self.defines():
			if not var in self.overwrites:
				self.printed_defs[var] = 1

		if self.subdir == '.':
			keys = utilities.subst_vars.keys()
			keys.sort()
			for var in keys:
				if var != 'top_builddir':
					if len(utilities.subst_vars[var]):
						output.append('%s = %s\n' % (var, utilities.subst_vars[var]))
					else:
						output.append('%s = @%s@\n' % (var, var))
			output.append("transform = @program_transform_name@\n")
			output.append("INSTALL = @INSTALL@\n")
			
		phonies = []
		keys = self.targets.keys()
		keys.sort()
		forwards = []
		for key in ['install', 'install-data', 'install-exec', 'clean', 'check', 'force-reedit']:
			if not key in keys:
				forwards.append(key)

		targets_output = []
		
		for key in keys:
			if key in [".PHONY"]:
				continue
			target = self.targets[key]
			if not target.user_specified:
				self.print_out_target(targets_output, key)
				if target.is_phony:
					phonies.append(key)
			else:
				if target.is_phony:
					forwards.append(key)

		forwards[:0] = ['all']
		
		output.append(".FORWARDS: " + string.join(forwards) + "\n\n")
		for key in forwards:
			output.append("%s:\n\t@echo 'WARNING: use unsermake instead of make or use a wrapper script, e.g. makeobj!!!'\n\t%s %s\n\n" %
						  (key, utilities.our_path, key))

		output += targets_output
		
		keys = self.defines()
		keys.sort()
		for definition in keys:
			self.print_out_definition(output, definition)
			#if not self.printed_defs.has_key(definition):
			#	print '%s = %s' % (definition, string.join(self.defs[definition]))

		if len(phonies):
			output.append(".PHONY: " + string.join(phonies) + "\n")
		utilities.write_if_changed(re.sub('.am$', '.in', self.filename), output, force)

	def translate_target(self, target):
		targ = self.target(target)
		
		if not targ:
			targ = self.target('$(srcdir)/' + target)
			lfs = self.source + target
		else:
			lfs = self.build + targ.target
			
		if targ and targ.user_specified:
			self.to_handle_targets.append(targ)

			for dep in targ.deps:
				if type(dep) == types.StringType:
					self.translate_target(dep)
				else:
					self.translate_target(dep.value)

			self.insertTarget(lfs, [])
			
	def replace_builddir(self, deps, leaveout_vars=1):
		newdeps = deps
		deps = []
		for dep in newdeps:
			if isinstance(dep, DefValue):
				dep = dep.value
			if not len(dep):
				continue
			elif dep.startswith('./'):
				dep = self.build + dep[2:]
			elif dep[0] == '/': # absolute paths are rather seldom
				pass
			elif dep.startswith('../'):
				dep = os.path.normpath('$(top_builddir)/' +
									   self.subdir + '/' + dep)
			elif dep.startswith('$(srcdir)'):
				if dep[9] == '/':
					dep = self.source + dep[10:]
				else:
					dep = self.source + dep[9:]
			elif dep.startswith('$(top_srcdir)'):
				pass
			elif dep[0] == '$':
				if re.match('^\$\(top_builddir\)', dep):
					deps.append(dep)
					continue
				match = utilities.variablesre.match(dep)
				if not match or len(match.group(1)):
					utilities.print_error('%s: "%s" appears to be a variable, but is none. Assertion hit!\n' % (self.filename, dep))
					continue
				var = match.group(2)
				pvar = '%s_%s' % (self.canon_subdir, var)
				if leaveout_vars:
					pvar = pvar + '_dep'
				if self.conds.has_key(var):
					if not self.conds.has_key(pvar):
						list1 = self.replace_builddir(self.conds[var][1], leaveout_vars)
						list2 = self.replace_builddir(self.conds[var][2], leaveout_vars)
						if list1 == list2:
							deps.extend(list1)
						else:
							self.conds[pvar] = (self.conds[var][0], list1, list2)
							deps.append('$(%s)%s' % (pvar, match.group(3)))
					else:
						deps.append('$(%s)%s' % (pvar, match.group(3)))
						
					continue
				elif utilities.subst_vars.has_key(var) and len(utilities.subst_vars[var]):
					deps.extend(self.replace_builddir(string.split(utilities.subst_vars[var] + match.group(3)), leaveout_vars))
					continue
				elif leaveout_vars:
					continue
			elif dep[0] == '-':
				if leaveout_vars:
					continue
			else:
				target = self.target(dep)
				if not target:
					if self.in_srcdir(dep) and not self.in_builddir(dep):
						dep = "$(srcdir)/" + dep
					else:
						dep = self.build + dep
				elif not target.is_phony:
					dep = self.build + dep
			
			deps.append(dep)
		return deps
	
	def rewrite_list(self, list):
		files = []
		for file in list:
			self.translate_target(file)
			match = utilities.variablere.match(file)
			if match:
				self.rewrite_cond(match.group(1))
				files.append('$(%s_%s)' % (self.canon_subdir, match.group(1)))
			else:
				dir = self.source
				if self.target(file) or self.target(self.build + file):
					dir = self.build
				elif file.startswith(self.source):
					files.append(file)
					continue
				files.append(dir + file)
		return files
	
	def rewrite_cond(self, var):
		if not self.conds.has_key(var):
			print self.filename, var
		assert(self.conds.has_key(var))
		cond = self.conds[var]
		new_tuple = (cond[0], self.rewrite_list(cond[1]),
					 self.rewrite_list(cond[2]))
		self.conds[self.canon_subdir + '_' + var] = new_tuple

	def add_install_target(self, install_rules, dirprefix, file, primary):
		my_install_rules = []
		basename = os.path.basename(file)
		basenamenoext = basename
		if basenamenoext.rfind('.') != -1:
			basenamenoext = basenamenoext[:basenamenoext.rfind('.')]
		mansect = ""

		dirname = '$(DESTDIR)$(%sdir)' % dirprefix
		if primary == 'MANS':
			match = self.mansectre.match(file)
			if not match:
				utilities.print_error('%s is an illegal manpage filename.\n' % file)
			else:
				mansect = match.group(1)
				dirname += "/man" + mansect
				assert(mansect in self.mansections)

		for rule in install_rules:
			rule = string.replace(rule, '@file@', file)
			rule = string.replace(rule, '@basename@', basename)
			rule = string.replace(rule, '@basenamenoext@', basenamenoext)
			rule = string.replace(rule, '@mansect@', mansect)
			my_install_rules.append(rule)

		target = dirname + ('/%s' % basename)
		deps = [file, '$(UNSERMAKE_FORCE_INSTALL)']

		self.insertTarget(target, deps,
						  ['@test -d %s || $(mkinstalldirs) %s' % (dirname, dirname)]
						  + my_install_rules )
		return target
		
	def collect_primaries(self):
		primaries = self.primaries()
		primaryre = re.compile('(.*)_([^_]*)')
		for key in self.defines():
			match = primaryre.match(key)
			if not match or not match.group(2) in primaries:
				continue
			primary = match.group(2)
			if primary == 'MANS' and match.group(1) == 'KDE':
				continue
			if self.conds.has_key(key):
				utilities.print_error('%s: found primary %s in conditional.\n' % (self.filename, key))
				continue
			files = self.rewrite_list(self.definition_rec(key))

			self.add_define("%s_%s" % (self.canon_subdir, key), files)
			if match.group(1) == 'check':
				self.insertTarget("check-am", "$(%s_%s)" % (self.canon_subdir, key), phony=1)
				self.insertTarget("check", "check-am", phony=1)
			elif match.group(1) != 'EXTRA':
				self.insertTarget("all-%s" % (self.canon_subdir),
								  "$(%s_%s)" % (self.canon_subdir, key),
								  phony=1)
			if match.group(1) in ['EXTRA', 'noinst', 'check']:
				continue

			varname = '%sdir' % match.group(1)
			if not utilities.subst_vars.has_key(varname):
				if not self.is_defined('%s_%sdir' % (self.canon_subdir, match.group(1))):
					if primary == 'MANS':
						self.add_define('%s_%sdir' % (self.canon_subdir, match.group(1)),
										'$(mandir)/man' + match.group(1)[-1])
					else:
						dirkey = '%sdir' % match.group(1)
						ndirkey = '%s_%sdir' % (self.canon_subdir, match.group(1))
						if not self.conds.has_key(dirkey):
							self.add_define(ndirkey, self.definition_rec(dirkey))
						else:
							self.conds[ndirkey] = self.conds[dirkey]
						
				dirprefix = '%s_%s' % (self.canon_subdir, match.group(1))
			else:
				dirprefix = match.group(1)

			install_kind = 'exec'
			
			install_rules = ['@$(V_ECHO) "%s @file@ %s"; \\' % (utilities.installing_text, utilities.normal)]
			uninstall_rules = ['$(V_ECHO) "%s $$p %s"; \\' % (utilities.uninstalling_text, utilities.normal)]
			if primary == 'LTLIBRARIES':
				install_rules += ['$(LIBTOOL) --quiet --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) @file@ $(DESTDIR)$(%sdir) > /dev/null' % dirprefix]
				uninstall_rules += ['f="`echo $$p | sed -e \'s|^.*/||\'`"; \\',
									'$(LIBTOOL) --quiet --mode=uninstall rm -f $(DESTDIR)$(%sdir)/$$f; \\' % dirprefix]
			elif primary == 'LIBRARIES':
				install_rules += ['$(INSTALL_DATA) @file@ $(DESTDIR)$(%sdir)/@basename@' % dirprefix,
								 '$(RANLIB) $(DESTDIR)$(%sdir)/@basename@' % dirprefix]
				uninstall_rules += ['f="`echo $$p | sed -e \'s|^.*/||\'`"; \\',
								   'rm -f $(DESTDIR)$(%sdir)/$$f; \\' % dirprefix]
			elif primary in ['DATA', 'HEADERS']:
				install_rules += ['$(INSTALL_DATA) @file@ $(DESTDIR)$(%sdir)/@basename@' % dirprefix]
				uninstall_rules += ['f="`echo $$p | sed -e \'s|^.*/||\'`"; \\',
								   'rm -f $(DESTDIR)$(%sdir)/$$f; \\' % dirprefix]
				install_kind = 'data'
			elif primary == 'PROGRAMS':
				if sys.platform == 'cygwin':
					install_rules += ["EXEXT='.exe'; \\"]
				install_rules += ["p1=`echo @file@ |sed 's/$(EXEEXT)$$//'`; \\",                                               
								 'if test -f @file@ || test -f $$p1 ; then \\',                                                
								 "f=`echo $$p1|sed '$(transform);s|^.*/||;s/$$/$(EXEEXT)/'`; \\",                              
								 '$(INSTALL_PROGRAM_ENV) $(LIBTOOL) --quiet --mode=install '                                   
								 '$(INSTALL_PROGRAM) @file@$(EXEEXT) $(DESTDIR)$(%sdir)/$$f; else :;\\' % dirprefix,           
								 'fi']                                                                                         
				uninstall_rules += ['f=`echo $$p|sed \'s/$(EXEEXT)$$//;$(transform);s|^.*/||;s/$$/$(EXEEXT)/\'`; \\',
								   'rm -f $(DESTDIR)$(%sdir)/$$f; \\' % dirprefix]
			elif primary == 'SCRIPTS':
				install_rules += ['f="`echo @file@ |sed \'$(transform)\'| sed \'s|^.*/||\'`"; \\',
								 '$(INSTALL_SCRIPT) @file@ $(DESTDIR)$(%sdir)/$$f' % dirprefix]
				uninstall_rules += ['f="`echo $$p|sed \'$(transform)\' | sed \'s|^.*/||\'`"; \\',
								   'rm -f $(DESTDIR)$(%sdir)/$$f; \\' % dirprefix]
			elif primary == 'MANS':
				installdir = '$(DESTDIR)$(%sdir)' % dirprefix
				manfilename = ''
				if match.group(1) == 'man':
					installdir += '/man@mansect@'
					manfilename = '@basename@'
				else:
					assert(match.group(1)[-1] in self.mansections)
					manfilename = '@basenamenoext@.%s' % match.group(1)[-1]
				install_rules += ['$(INSTALL_DATA) @file@ %s/%s' % (installdir, manfilename)]

				uninstalldir = '$(DESTDIR)$(%sdir)' % dirprefix
				if match.group(1) == "man":
					manfilename = '$$f'
					uninstall_rules.append('s="`echo $$p | sed -e \'s|^.*\\.\\([0-9a-z]\\)[a-z]*$$|\\1|\'`"; \\')
					uninstalldir += "/man$$s";
				else:
					manfilename = '"`echo $$f | sed -e \'s|[^\\.]*$$|%s|\'`"' % match.group(1)[-1]

				uninstall_rules += ['f="`echo $$p | sed -e \'s|^.*/||\'`"; \\',
									'echo " rm -f %s/%s"; \\' % (uninstalldir, manfilename),
									'rm -f %s/%s; \\' % (uninstalldir, manfilename)]
				install_kind = 'data'
			else:
				print('%s: install for %s unimplemented\n' % (self.filename, primary))
				install_rules = []
				uninstall_rules = []

			for file in files:
				vmatch = utilities.variablere.match(file)
				if vmatch:
					(cond, list_true, list_false) = self.conds[vmatch.group(1)]
					varname = 'installed-%s%s-%s' % (dirprefix, primary, vmatch.group(1))
					new_list_true = []
					new_list_false = []
					for file in list_true:
						target = self.add_install_target(install_rules, dirprefix, file, primary)
						new_list_true.append(target)
					for file in list_false:
						target = self.add_install_target(install_rules, dirprefix, file, primary)
						new_list_false.append(target)
					self.conds[varname] = (cond, new_list_true, new_list_false)
					self.insertTarget('install-%s-%s%s' % (self.canon_subdir, dirprefix, primary),
									  '$(%s)' % varname, phony=1)
				else:
					target = self.add_install_target(install_rules, dirprefix, file, primary)
					self.insertTarget('install-%s-%s%s' % (self.canon_subdir, dirprefix, primary),
									  target, phony=1)

			if len(files):
				self.insertTarget('uninstall-%s-%s%s' %	(self.canon_subdir, dirprefix, primary), [],
								  ['@list=\'$(%s_%s)\'; for p in $$list; do \\' % (self.canon_subdir, key)]
								  + uninstall_rules +
								  ['done'], phony=1)
				self.insertTarget('install-%s-%s' % (install_kind, self.canon_subdir),
								  'install-%s-%s%s' %	(self.canon_subdir, dirprefix, primary), phony=1)
				self.insertTarget('uninstall', 'uninstall-%s-%s%s' %
								  (self.canon_subdir, dirprefix, primary), phony=1)
			
		local_deps = []
		for targ in ["install-exec", "install-data"]:
			if self.is_target(targ +  '-%s' % self.canon_subdir) and self.is_target(targ + "-hook"):
				self.insertTarget(targ, targ +  '-%s' % self.canon_subdir, phony=1)
				self.insertTarget(targ +  '-%s' % self.canon_subdir, [],
								  "cd " + self.build + " && $(MAKE) " + targ + "-hook", phony=1)
				local_deps.append(targ + '-%s' % (self.canon_subdir))
			elif self.is_target(targ +  '-%s' % self.canon_subdir):
				self.insertTarget(targ, targ +  '-%s' % self.canon_subdir, phony=1)
				local_deps.append(targ + '-%s' % (self.canon_subdir))
			elif self.is_target(targ + "-hook"):
				utilities.print_error("%s: Hook for target %s installed without this target being used\n" % (self.filename, targ))
				return

		self.insertTarget('install-%s' % (self.canon_subdir), local_deps, phony=1)
		
		# adding the default install targets
		self.insertTarget('install', ["install-data",  "install-exec"], phony=1)
		self.insertTarget('install-exec', '', phony=1)
		self.insertTarget('install-data', '', phony=1)
			
	def collect(self):

		# adding default targets
		
		self.default_includes = string.join(self.replace_srcdir(['$(DEFS)', '-I.', '-I$(srcdir)', '-I$(top_builddir)']))
		
		self.insertTarget("all-%s" % (self.canon_subdir), [], phony=1)
		self.insertTarget("all-recursive", "all-%s" % (self.canon_subdir), phony=1)
		self.insertTarget("all", "all-recursive", phony=1)
		self.insertTarget("install", "install-%s" % (self.canon_subdir), phony=1)
		
		self.insertTarget("compile", "", phony=1)
		self.insertTarget("clean", "", phony=1)

		self.find_binaries()

		for handler in handlerbase.handlers:
			handler.shuffle_binaries(self)
			
		for prog in self.binaries.values():
			prog.handle_sources( self.definition_rec( prog.canon_name + '_SOURCES' ) )
			prog.add_targets()

		for bin in self.binaries.values():
			list = self.value_list(bin.canon_name + '_COMPILE_FIRST')
			for imp in list:
				self.translate_target(imp)
			for imp in list:
				for obj in bin.objs:
					self.insertTarget(self.build + obj, self.replace_builddir([imp]))
			bin.add_final_target()
			
		for handler in handlerbase.handlers:
			handler.collect(self)
		
		for targ in ['install-exec', 'install-data', 'all', "uninstall", "check", "clean"]:
			target = self.target("%s-local" % targ)
			if target:
				self.insertTarget(targ, target.target, phony=1)
			
		for target in self.targets.values():
			if target.user_specified:
				ndeps = []
				for dep in target.deps:
					if isinstance(dep, DefValue):
						ndeps.append(dep.value)
					else:
						ndeps.append(dep)
				ndeps = self.replace_builddir(ndeps)
				target.deps = []
				for dep in ndeps:
					target.deps.append(DefValue(dep))
				ntargs = self.replace_builddir([target.target])
				if not len(ntargs):
					del self.targets[target.target]
					continue
				# TODO: if it builds then remove the loop
				assert(len(ntargs) == 1)
				if ntargs[0] != target.target:
					del self.targets[target.target]
					target.target = ntargs[0]
					self.addTarget(target)

		self.collect_primaries()
		if self.is_defined("CLEANFILES"):
			self.insertTarget("clean-%s-CLEANFILES" % self.canon_subdir,
									[], "rm -f " + string.join(self.replace_builddir(self.definition_rec("CLEANFILES"))), phony=1)
			self.insertTarget("clean", "clean-%s-CLEANFILES" % self.canon_subdir, phony=1)
			
		if self.is_defined("DISTCLEANFILES"):
			self.insertTarget("clean-%s-DISTCLEANFILES" % self.canon_subdir,
							  [], "rm -f " + string.join(self.replace_builddir(self.definition_rec("DISTCLEANFILES"))), phony=1)
			self.insertTarget("clean", "clean-%s-DISTCLEANFILES" % self.canon_subdir, phony=1)

		for targ in self.to_handle_targets:
			dir = self.build
			if not self.target(targ.target) and not self.target(dir + targ.target):
				dir = self.source

			if targ.target.startswith('$(top_builddir)'):
				continue
			if targ.target.startswith('$(top_srcdir)'):
				continue
			
			deps = []
			for dep in targ.deps:
				if isinstance(dep, DefValue):
					dep = dep.value
				if len(os.path.dirname(dep)):
					deps.append(dep)
				else:
					if self.is_target(self.build + dep):
						deps.append(self.build + dep)
					else:
						deps.append(self.source + dep)

			lfs = dir + targ.target
			if targ.target.startswith('$(srcdir)'):
				lfs = self.source + targ.target[10:]

			if not len(targ._rules): # that's the easy case
				self.insertTarget(lfs, self.replace_builddir(deps, 0))
				if self.targets.has_key(targ.target):
					del self.targets[targ.target]
			else:
				if targ.target.startswith('$(srcdir)'):
					self.insertTarget(lfs,
									  self.replace_builddir(deps, 0),
									  "cd " + self.build
									  + " && $(MAKE) '" + self.source +
									  targ.target[10:] + "'")
				else:
					del self.targets[targ.target]
					targ.target = lfs
					self.addTarget(targ)

		for bin in self.binaries.values():
			bin.collect_final_dependencies()
		
		deps = []
		for file in ['Makefile.am'] + self.am_includes:
			if file.startswith('$(top_srcdir)'):
				deps.append(file)
			else:
				deps.append(self.source + file)

		if not self.subdir == '.':
			rule = "cd $(top_srcdir) && %s -c %s/Makefile" % (utilities.our_path, self.subdir)
			self.insertTarget(self.source + "Makefile.in", deps, rule)
			
			if 0: # TODO: check if still necessary
				for target in self.targets.values():
					if target.user_specified and target.has_rules() and not target.is_phony:
						self.insertTarget('$(top_builddir)/%s/%s' %
										  (self.subdir, target.target),
										  target.target)

				# for subdirs we need to generate aliases for ../<subdir>/target
				targets = self.targets.values()
				prefixlen = len(self.build)
				for target in targets:
					if target.target[:prefixlen] == self.build and not target.is_phony:
						targ = target.target[prefixlen:]
						# don't add rules the other way around than the above
						rhs = '$(top_builddir)/%s/%s' % (self.subdir, targ)
						if not self.is_target(rhs):
							# note, that we insert an empty command (the " ;" at the end
							# of the prerequisites), to avoid invoking any default commands
							self.insertTarget(targ, [rhs, ";"])
						elif not self.target(rhs).has_rules():
							self.insertTarget(targ, ";")

		else:
			deps.append(utilities.configure_in)
			deps.append('$(top_srcdir)/aclocal.m4')
			rule = "cd $(top_srcdir) && %s -c" % utilities.our_path
			self.insertTarget(self.source + "Makefile.in", deps, rule)
	
	def read_deps(self):
		if not len(self.dep_files):
			return

		depdir_value = utilities.subst_vars["DEPDIR"]
		depdir = utilities.top_builddir + "/" + self.subdir + "/" + depdir_value + "/"
		if not os.path.exists(depdir):
			try:
				os.mkdir(depdir)
			except OSError, e:
				print "failed to create %s:" % depdir_value, e
				return

		for dep in self.dep_files:
			dep = depdir + dep
			if os.path.exists(dep):
				dep_lines = open(dep).readlines()
				if not len(dep_lines):
					continue
				if dep_lines[0].startswith("builddir="):
					builddir = dep_lines[0][len("builddir="):-1]
					topbuilddir = dep_lines[1][len("top_builddir="):-1]
					otargetfile = targetfile = dep_lines[2][len("targetfile="):-1]
					if targetfile.startswith('$(top_builddir)'):
						targetfile = utilities.top_builddir + targetfile[len('$(top_builddir)'):]
					if topbuilddir == ".":
						tops = []
					else:
						tops = string.split(topbuilddir, '/')
					builds = string.split(builddir, '/')
					mytopbuilddir = dir = os.path.abspath(utilities.top_builddir) + "/"
					for index in range(0, len(tops)):
						dir += "/%s" % builds[index+1]
					dir += '/'

					target_deps = []
					self.insertTarget(targetfile, [])
					target = self.targets[targetfile]
					
					for dep in dep_lines[3:]:

						dep = dep[:-1]
						if dep[0] != '/':
							dep = os.path.normpath(dir + dep)

							if dep.startswith(mytopbuilddir):
								if find_missing_deps:
									dep2 = '$(top_builddir)/' + dep[len(mytopbuilddir):]
									found=False
									for dep3 in self.targets[otargetfile].deps:
										if type(dep3) == types.StringType:
											print dep2, dep3
											assert(False)
										else:
											if dep2 == dep3.value:
												found=True
												
									if not found:
										if found_missing_deps.has_key(otargetfile):
											found_missing_deps[otargetfile].append(dep2)
										else:
											found_missing_deps[otargetfile] = [dep2]

								else:
									dep = utilities.top_builddir + dep[len(mytopbuilddir) - 1:]

						if find_missing_deps: continue

						target.deps.append(dep)
						maybe_missing[dep] = True
						
					self.insertTarget(targetfile, target_deps)
					
				else:
					deps_am = AMFile(dep)
					for targ in deps_am.targets.values():
						targ.user_specified = 0
						self.addTarget(targ)

		self.dep_files = []
