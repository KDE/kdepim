import types, string, re, os, glob
import utilities

just_print = False
num_todo = 0
print_progress = False
precise_print_progress = False

class PhonyConflict(Exception):
	pass

class DefValue:
	def __init__(self, value):
		#assert(type(value) == types.StringType)
		self.value = value
		
	def __repr__(self):
		return "-" + self.value + "-"
	
class Target:
	def __init__(self, target, deps, rules, user_specified, phony):
		self.target = target
		if type(deps) == types.StringType:
			deps = string.split(deps)
			self.deps = []
			for dep in deps:
				self.deps.append(DefValue(dep))
		else:
			assert(type(deps) == types.ListType)
			self.deps = []
			for dep in deps:
				if isinstance(dep, DefValue):
					self.deps.append(dep)
				else:
					self.deps.append(DefValue(dep))

		if type(rules) == types.StringType:
			self._rules = [ rules ]
		elif user_specified:
			self._rules = []
			for rule in rules:
				striped = string.strip(rule)
				if len(striped) and not striped[0] == '#':
					if not rule[0] == '\t':
						if user_specified:
							raise 'rule %s doesn\'t start with <tab>' % target
						self._rules.append(string.rstrip(rule))
					else:
						self._rules.append(string.rstrip(rule[1:]))
		else:
			self._rules = rules[:]
		self.user_specified = user_specified
		self.is_phony = phony
		self.changed = False
		self.expanded = False

	def __repr__(self):
		return self.target + ":"
	
		ret = self.target + ": " + self.deps_string()
		for rule in self._rules:
			ret += "\n\t" + rule
		if self._rules:
			ret += "\n"
		return ret
	
	def has_rules(self):
		return len(self._rules) != 0
	
	def merge(self, targ):
		assert(self.target == targ.target)
		if not self.is_phony == targ.is_phony:
			raise PhonyConflict

		self.expanded = self.expanded and targ.expanded
		self.compile_target = self.compile_target or targ.compile_target

		for dep in targ.deps:
			self.deps.append(dep)

		if self._rules == targ._rules:
			return
		elif len(self._rules) and len(targ._rules):
			if not self.user_specified or not targ.user_specified:
				print self._rules
				print targ._rules
				raise ("two targets named '%s' define rules!" % self.target)
		else:
			if targ.user_specified and len(targ._rules):
				self.user_specified = 1
			self._rules.extend(targ._rules)

	def deps_string(self):
		output = ""
		for dep in self.deps:
			if isinstance(dep, DefValue):
				output += " " + dep.value
			else:
				output += " " + dep
		if len(output):
			return output[1:]
		return output
		
	def print_out(self, output):
		output.append('%s: %s\n' % (self.target, self.deps_string()))
		for rule in self._rules:
			if rule[0] == '\002':
				output.append('%s\n' % string.replace(rule[1:], '\001', '\\\n'))
			else:
				output.append('\t%s\n' % string.replace(rule, '\001', '\\\n'))
		if len(self._rules):
			output.append("\n")

	def _used_defines_str(self, str):
		match = utilities.variablesre.match(str)
		if not match:
			return []
		if not len(match.group(3)):
			return [match.group(2)]
		else:
			return [match.group(2)] + self._used_defines_str(match.group(3))
		
	def used_defines(self):
		res = []
		for dep in self.deps:
			if type(dep) == types.StringType:
				res.append(dep)
			else:
				res.extend(self._used_defines_str(dep.value))
			
		for rule in self._rules:
			strings = string.split(rule)
			for _str in strings:
				res.extend(self._used_defines_str(_str))
		return res

	def expand_target(self, makefile):
		ntarg = makefile.expand(self.target)
		if ntarg == self.target:
			return
		try:
			del makefile.targets[self.target]
		except KeyError:
			pass

		if not len(ntarg):
			return

		ntargs = string.split(ntarg)

		if len(ntargs) == 1:
			self.target = ntarg
			makefile.addTarget(self)
		else:
			self.target = ntargs[0]
			makefile.addTarget(self)
			for index in range(1, len(ntargs)):
				nt = Target(ntargs[index], self.deps, self._rules,
							phony=self.is_phony,
							user_specified=0)
				nt.user_specified = self.user_specified
				nt.compile_target = self.compile_target
				makefile.addTarget(nt)

	def expand_rules(self, makefile, replace=1):
		rlen = len(self._rules)
		for index in range(rlen):
			line = makefile.expand(self._rules[index])
			if replace:
				line = makefile.replace_autoconf(line)
				line = string.replace(line, '$@', self.target)
				line = string.replace(line, '$<', self.deps_string())
			self._rules[index] = line

	def expand_deps(self, makefile):
		if not self.user_specified:
			ndeps = self.deps
			self.deps = []
			for dep in ndeps:
				if type(dep) == types.StringType:
					self.deps.append(dep)
				else:
					deps = string.split(makefile.expand(dep.value))
					for dep2 in deps:
						self.deps.append(dep2)
			return
		deps = self.deps
		self.deps = []
		for dep in deps:
			if type(dep) == types.StringType:
				edeps = [dep]
			else:
				edeps = string.split(makefile.expand(dep.value))
			for dep2 in edeps:
				gdeps = glob.glob(dep2)
				if len(gdeps):
					for dep3 in gdeps:
						self.deps.append(dep3)
				else:
					self.deps.append(dep2)
			
	def expand(self, makefile):
		if self.expanded:
			return
		self.expand_target(makefile)
		self.expand_rules(makefile, replace=0)
		self.expand_deps(makefile)
		self.expanded = True

	def _flags(self, current, ignore_exit=False, echo_line=True):
		if current[0] == '@':
			return self._flags(current[1:], ignore_exit, False)

		if current[0] == '-':
			return self._flags(current[1:], True, echo_line)

		return (current, ignore_exit, echo_line)
	
	def call_command(self, makefile):
		self.changed = 1
		endswith = 0
		current = ''
		targetre = re.compile('^(.*[^$])\$@(.*)')
		depsre = re.compile('^(.*[^$])\$\?(.*)$')
		fdepre = re.compile('^(.*[^$])\$<(.*)$')
		self.expand_rules(makefile, replace=0)
		linere = re.compile('\s*\001\s+')
		first_line = True
		
		for line in self._rules:

			if line[0] == '\002':
				line = linere.sub(' ', line[1:])
			else:
				line = linere.sub(' ', line)

			line = string.lstrip(line)
			if endswith:
				current += line
			else:
				current = line
			endswith = current[-1] == '\\'
			if endswith:
				current = current[:-1]
				continue
			
			current, ignore_exit, echo_line = self._flags(current)
			
			while True:
				match = targetre.match(current)
				if not match:
					break
				current = match.group(1) + self.target + match.group(2)
			while True:
				match = depsre.match(current)
				if not match:
					break
				current = match.group(1) + self.deps_string() + match.group(2)
			while True:
				match = fdepre.match(current)
				if not match:
					break
				current = match.group(1) + self.deps[0] + match.group(2)
						   
			current = string.replace(current, "$$", "$")
			
			if echo_line:
				if print_progress and not first_line:
					print "    " + current
				elif precise_print_progress and not first_line:
					print "       " + current
				else:
					print current
					first_line = False

			if just_print:
				print current
				continue
			
			pid = os.fork()
			if not pid:
				if self.user_specified:
					os.chdir(makefile.asubdir)
				os._exit(os.execl("/bin/sh", "/bin/sh", "-c", current))
			else:
				(pid, exit_code) = os.waitpid(pid, 0)
			if exit_code and not ignore_exit:
				if os.WIFEXITED(exit_code):
					return os.WEXITSTATUS(exit_code)
				return exit_code

		return 0

