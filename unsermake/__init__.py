import os
#os.environ['PYCHECKER'] = 'no-shadowbuiltin'
#import pychecker.checker

import string, re, os, types, stat

import utilities, amfile
import handlerbase
from target import Target, PhonyConflict
from sourcehash import SourceHash
import target
import program, time
import sys, getopt, os


verbose = False
ignore_most = False
max_children = 1
max_compile_jobs = 0

def create_makefiles(given_files):
	given_dirs = []
	for index in range(len(given_files)):
		filename = given_files[index]
		if not os.path.exists(filename + '.am'):
			sys.stderr.write('%s.am does not exist!\n' % filename)
			sys.exit(1)
		given_files[index] = filename
		filename = os.path.dirname(filename)
		if not len(filename):
			filename = '.'
		given_dirs.append(filename)

	makefiles = []
	utilities.subst_vars, files, headers, auxdir = utilities.parse_autoconf()

	# the remaining files, not beeing automake files
	# these need to be remade with config.status
	config_files = {}
	config_headers = {}

	for file in given_files:
		if not file in files:
			files.append(file)

	for file in files:
		dir = os.path.dirname(file)
		if not len(dir):
			dir = '.'

		if len(given_dirs) and not dir in given_dirs:
			continue

		if os.path.exists( file + '.am'):
			makefiles.append( file + '.am' )
		elif os.path.exists( file + '.in') or file == 'MakeVars':
			file = os.path.basename( file )
			if config_files.has_key(dir):
				config_files[dir].append( file )
			else:
				config_files[dir] = [ file ]
		else:
			utilities.print_error('%s appears in configure files, but has no .in file\n' % file)

	for file in headers:
		dir = os.path.dirname(file)
		if not len(dir):
			dir = '.'

		file = os.path.basename( file )
		if config_headers.has_key(dir):
			config_headers[dir].append( file )
		else:
			config_headers[dir] = [ file ]

	# the backslash is very bad to have as end of line ;/
	if utilities.subst_vars.has_key("AMDEPBACKSLASH"):
		del utilities.subst_vars["AMDEPBACKSLASH"]

	if not utilities.subst_vars.has_key("DEPDIR"):
		utilities.subst_vars['DEPDIR'] = '.deps'

	utilities.subst_vars['depcomp'] = '$(SHELL) %s/depcomp\n' % auxdir
	utilities.subst_vars['mkinstalldirs'] = '$(SHELL) %s/mkinstalldirs' % auxdir

	# the following is a several pass thing. First we have to parse
	# all, then we have to do overall checks and inserting of targets
	# and then we can print the makefiles out
	amfiles = []

	stamp_cnt = 1
	for file in makefiles:
		makefile = amfile.AMFile(os.path.normcase(os.path.abspath(file)))
		if config_files.has_key(makefile.subdir):
			makefile.set_configure_files( config_files[ makefile.subdir] )
		else:
			makefile.set_configure_files( [] )
		if config_headers.has_key(makefile.subdir):
			stamp_cnt = makefile.set_configure_headers( config_headers[ makefile.subdir], stamp_cnt )

		amfiles.append(makefile)

		for targ in makefile.targets.values():
			for line in targ._rules:
				if string.find(line, '$<') >= 0:
					utilities.print_error("%s: Do not use $< rules. They are not portable (%s).\n" %
								(makefile.filename, targ.target))
					
	for makefile in amfiles:
		makefile.create()
		makefile.print_out(makefile.subdir in given_dirs)

def overwrite_makefile(dest):
	for var, define in utilities.environment_vars.items():
		dest.del_define(var)
		dest.add_define(var, define)

def merged_makefiles(builddir, srcdir):

	path = os.path.abspath(builddir + "/Makefile")
	if not os.path.exists(path):
		if os.path.exists(srcdir + "/Makefile.am"):
			src = amfile.AMFile(path, create_empty=True)
			# ignore configure content
			src.add_makefile_rules()
			if getmtime(srcdir + "/Makefile.am") > getmtime(utilities._topsrcdir + "/configure.in"):
				raise OldConfigureIn(srcdir)
		else:
			if utilities.top_builddir_abs and builddir.startswith(utilities.top_builddir_abs):
				path = builddir[ len(utilities.top_builddir_abs) + 1 : ]
			utilities.print_error("No Makefile present in: %s!\n" % path)
			return None
	else:
		src = amfile.AMFile(path)
		src.del_define("top_srcdir")
	
	if not os.path.exists(srcdir + "/Makefile.am"):
		overwrite_makefile(src)
		return src
	
	dest = amfile.AMFile(os.path.abspath(srcdir + "/Makefile.am"))
	dest.defs["subdir"] = [dest.subdir]
	dest.defs["srcdir"] = ['$(top_srcdir)/' + dest.subdir]
	
	for var, define in src.defs.items():
		if var in ['subdir', 'srcdir']: # the better paths are in srcdir
			continue
		if var in utilities.environment_vars.keys():
			continue
		dest.del_define(var)
		if len(define):
			dest.add_define(var, define)

	overwrite_makefile(dest)

	for targ in dest.targets.values():
		# expand the variables to the local scope
		targ.expand_target(dest)
		targ.expand_deps(dest)
		targ.expand_rules(dest, replace=1)

	for targ in src.targets.values():
		targ.user_specified = 0
		dest.addTarget(targ)

	# we need to delete the cache as the context changes
	dest.cached_defs = {}
	return dest

def append_all_defines(all_defines, makefile):
	values = makefile.targets.values()
	for targ in values:
		if not targ.expanded:
			targ.expand_target(makefile)

	# expand_target moves/merges targets
	values = makefile.targets.values()
	for targ in values:
		try:
			all_defines[targ.target].append(makefile)
		except KeyError:
			all_defines[targ.target] = [makefile]

	return all_defines

def remove_makefile(all_defines, makefile):
	values = makefile.targets.values()
	for targ in values:
		all_defines[targ.target].remove(makefile)
		if not len(all_defines[targ.target]):
			del all_defines[targ.target]
	return all_defines
	
class NoSuchTarget(Exception):
	def __init__(self, targ, orig = "", filename = ""):
		Exception.__init__(self)
		self.target = targ
		self.orig = orig
		self.filename = filename

class MakefileChanged(Exception):
	def __init__(self, filename):
		Exception.__init__(self)
		self.filename = filename

class OldConfigureIn(Exception):
	def __init__(self, filename):
		Exception.__init__(self)
		self.filename = filename

def read_subdirs(makefile, all_defines, src_prefix, prefix, ignore_makefile_updates=False):

	makefile.check()
	makefile.collect()
	makefile.read_deps()

	all_defines = append_all_defines(all_defines, makefile)

	makefile_filename = makefile.expand(makefile.build + "Makefile")
	if makefile.targets.has_key(makefile_filename):
		try:
			if finish(all_defines, makefile_filename) and not ignore_makefile_updates:
				raise MakefileChanged(makefile_filename)
		except NoSuchTarget, t:
			print "No such target", t.target
			if not ignore_most:
				utilities.sys_exit_code = 1
				return all_defines
		
	if not makefile.is_defined("SUBDIRS"):
		return all_defines

	subdirs = []
	
	for subdir in makefile.definition_rec("SUBDIRS"):
		subdir = makefile.expand(subdir)
		if subdir == '.':
			continue
		subdirs.extend(string.split(subdir))

	for subdir in subdirs:
		
		nprefix = prefix + subdir + "/"
		nsrc_prefix = src_prefix + subdir + "/"
		submakefile = merged_makefiles(nprefix, nsrc_prefix)
		if not submakefile:
			continue
		submakefile.del_define("top_srcdir")
		submakefile.del_define("top_builddir")
		try:
			all_defines = read_subdirs(submakefile, all_defines, nsrc_prefix, nprefix)
		except MakefileChanged, m:
			all_defines = remove_makefile(all_defines, submakefile)
			submakefile = merged_makefiles(nprefix, nsrc_prefix)
			submakefile.del_define("top_srcdir")
			submakefile.del_define("top_builddir")
			all_defines = read_subdirs(submakefile, all_defines, nsrc_prefix,
									   nprefix, ignore_makefile_updates=True)
	return all_defines

stat_cache = {}

def call(obj, makefile, fork):
	if verbose:
		print "updating", obj.target

	if fork:
		pid = os.fork()
		if pid:
			return pid

	try:
		ret = obj.call_command(makefile)
	except KeyboardInterrupt, ki:
		if not fork:
			raise ki
		os._exit(130)
		
	if ret:
		sys.stderr.write("Error creating %s. Exit status %d.\n" % (obj.target, ret))
		if not obj.is_phony:
			try:
				os.unlink(obj.target)
			except OSError:
				pass
	if amfile.ignore_errors:
		utilities.sys_exit_code = ret
		ret = 0
	if fork:
		os._exit(ret)
	else:
		return ret

sourcehash = None
def getsourcemtime(file, real):
	if sourcehash is None:
		return real
	for ending in ['.h','.cpp','.cc','.c']:
		if file.endswith(ending):
			return sourcehash.mtime(file,real)
	return real

def getrealmtime(file, force=0):
	if not force and stat_cache.has_key(file):
		m = stat_cache[file]
		return m

	try:
		m = os.stat(file)[stat.ST_MTIME]
	except OSError, e:
		m = 0
		
	stat_cache[file] = m
	return m

def getmtime(file, force = 0):
	return getsourcemtime(file, getrealmtime(file, force))

def exists(file):
	if stat_cache.has_key(file):
		m = stat_cache[file]
		return m != 0

	return getmtime(file) != 0

def add_subst_var(var, define):
	if not utilities.environment_vars.has_key(var):
		utilities.subst_vars[var] = define

def export_var(var):
	if utilities.environment_vars.has_key(var):
		os.environ[var] = utilities.environment_vars[var]
	elif utilities.subst_vars.has_key(var):
		os.environ[var] = utilities.subst_vars[var]
	else:
		print 'Variable %s does not exist.' % var
	
def two_spaces_per(depth):
	ret = ""
	for i in range(0, depth):
		ret += "  "
	return ret

def fix_target_depth(targ, depth):
	if targ.depth > depth:
		return
	targ.depth = depth + 1
	for dep in targ.dep_objs:
		if type(dep) != types.StringType:
			fix_target_depth(dep, targ.depth)

class PrioQueue:
	def __init__(self):
		self.data = {}
		self.prios = []
		self.length = 0

	def add(self, item):
		if item.depth in self.prios:
			self.data[item.depth].append(item)
		else:
			self.data[item.depth] = [item]
			self.prios.append(item.depth)
			self.prios.sort()
			self.prios.reverse()
		self.length += 1

	def first(self):
		self.length -= 1
		depth = self.prios[0]
		obj = self.data[depth].pop(0)
		if not len(self.data[depth]):
			# clean up
			self.prios.pop(0)
		if verbose:
			print "first", obj.target, depth, len(self.data[depth])
		return obj

	def size(self):
		return self.length

	def first_prio(self):
		return self.prios[0]
	
	def fix_items(self):
		prios = self.prios[:]
		prios.reverse() # assuming the depth go only up
		for depth in prios:
			items = self.data[depth]
			index = 0
			while index < len(items):
				if items[index].depth != depth:
					self.add(items[index])
					del items[index]
					self.length -= 1
				else:
					index += 1
			if len(items):
				self.data[depth] = items
			else:
				del self.data[depth]
		self.prios = self.data.keys()
		self.prios.sort()
		self.prios.reverse()

def find_todo(targ, all_defines, todo_list, com_todo, non_todo, depth):
 
	if verbose:
		print two_spaces_per(depth), "find_todo", targ

	if todo_list.has_key(targ):
		return todo_list[targ]
	
	try:
		makefiles = all_defines[targ]
	except KeyError:
		# if the target is not defined, then we take the string
		# as target and are done
		todo_list[targ] = targ
		return targ

	rule=0
	first_tar = None

	# first we collect the target in all makefiles it's in
	for makefile in makefiles:
		tar = makefile.targets[targ]
		if not first_tar:
			tar.makefile = makefile
			first_tar = tar
			if not first_tar.expanded:
				first_tar.expand_deps(makefile)
			continue
		if tar.user_specified:
			# BTW: already expanded deps+rules (esp. the $@)
			del makefile.targets[targ]
			tar.target = targ + "_" + makefile.canon_subdir
			makefile.addTarget(tar)
			first_tar.deps.append(tar.target)
			assert(not all_defines.has_key(tar.target))
			all_defines[tar.target] = [makefile]
			continue
		if not tar.expanded:
			tar.expand_deps(makefile)
		if tar.has_rules():
			if first_tar.has_rules():
				continue # ignoring
			for dep in first_tar.deps:
				# TODO: profile
				if not dep in tar.deps:
					tar.deps.append(dep)
			tar.makefile = makefile
			first_tar = tar
		else:
			for dep in tar.deps:
				if not dep in first_tar.deps:
					first_tar.deps.append(dep)

	all_defines[targ] = [first_tar.makefile]
	
	# now some caching
	todo_list[targ] = first_tar

	# it might be changed by former finish runs. Then it's
	# uptodate and we can return the file name
	if first_tar.changed:
		todo_list[first_tar.target] = first_tar.target
		return first_tar.target
	
	if first_tar.is_phony:
		first_tar.mtime = 0
	else:
		first_tar.mtime = getmtime(first_tar.target)

	deps = 0
	first_tar.dep_objs = []
	first_tar.needs = []
	first_tar.deps_count = 0
	first_tar.is_todo = False
	first_tar.depth = depth

	for dep in first_tar.deps:
		if todo_list.has_key(dep):
			dep_obj = todo_list[dep]
		else:
			dep_obj = find_todo( dep, all_defines,
								 todo_list, com_todo,
								 non_todo, depth + 1)

		if type(dep_obj) == types.StringType:
			dep_mtime = getmtime(dep_obj)
			dep_str = dep_obj + "(str)"
			
			if not dep_mtime:
				if dep_obj in amfile.maybe_missing:
					continue
				if not ignore_most:
					raise NoSuchTarget(dep_obj, first_tar.target, first_tar.makefile.filename)
				else:
					utilities.sys_exit_code = 1
				continue
			if first_tar.mtime >= dep_mtime:
				continue
		else:
			dep_mtime = dep_obj.mtime
			dep_str = dep_obj.target
			if not dep_obj.mtime: # propagate
				first_tar.mtime = 0

		if verbose:
			print two_spaces_per(depth), "first_tar", first_tar.target, first_tar.mtime, dep_str, dep_mtime

		deps += 1
		if type(dep_obj) != types.StringType:
			dep_obj.needs.append(first_tar)
			if first_tar.depth >= dep_obj.depth:
				fix_target_depth(dep_obj, first_tar.depth)
			first_tar.deps_count += 1
		first_tar.dep_objs.append(dep_obj)
					
	# if the target is uptodate, just return the path
	if first_tar.mtime and not deps:
		# if we ignore this target, don't take the full
		# effort in future calls
		first_tar.changed = True
		todo_list[targ] = targ
		return targ

	if not first_tar.is_phony:
		target.num_todo += 1
	
	if not first_tar.deps_count:
		if first_tar.compile_target:
			com_todo.add(first_tar)
		else:
			non_todo.add(first_tar)

	return first_tar

def print_dfa(targ, depth = 0):
	if type(targ) == types.StringType:
		print two_spaces_per(depth) + targ
		return

	try:
		if target.printed:
			print targ.depth, depth, two_spaces_per(depth) + "[" + targ.target + "]"
			return
	except:
		pass
	print targ.depth, depth, two_spaces_per(depth) + targ.target, targ.mtime
	targ.printed = 1
	for dep in targ.dep_objs:
		print_dfa(dep, depth + 1)

def finish(all_defines, targ):
	if not all_defines.has_key(targ):
		raise NoSuchTarget(targ)

	children = {}
	try:
		return finish_internal(all_defines, targ, children)
	except KeyboardInterrupt, ki:
		for obj in children.values():
			#Note we use ansi color call to set the color to nothing - just incase we were half way through displaying a color
			sys.stderr.write("[0minterrupted call, removing %s\n" % obj.target)
			try:
				os.unlink(obj.target)
			except OSError:
				pass		
		raise ki

def finish_internal(all_defines, targ, children):
	com_todo = PrioQueue()
	non_todo = PrioQueue()

	target.num_todo = 0
	target_obj = find_todo(targ, all_defines, {}, com_todo, non_todo, 0)
	target.max_todo = target.num_todo
	com_todo.fix_items()
	non_todo.fix_items()
	done_something = False
	
	if type(target_obj) == types.StringType:
		return done_something

	if verbose:
		print_dfa(target_obj)

	compile_jobs = 0
	
	while non_todo.size() or com_todo.size() or len(children):

		while non_todo.size() or com_todo.size():

			if verbose:
				print "todo", non_todo.size(), com_todo.size(), "%d(%d)" % (compile_jobs, max_compile_jobs), "%d(%d)" % (len(children), max_children)

			if not max_compile_jobs:
				if com_todo.size():
					if non_todo.size():
						if com_todo.first_prio() > non_todo.first_prio():
							obj = com_todo.first()
						else:
							obj = non_todo.first()
					else:
						obj = com_todo.first()
				elif non_todo.size():
					obj = non_todo.first()
				else:
					break
			else:
				if com_todo.size() and compile_jobs < max_compile_jobs:
					obj = com_todo.first()
				elif non_todo.size() and len(children) - compile_jobs < max_children - max_compile_jobs:
					obj = non_todo.first()
				elif com_todo.size() and len(children) < max_children:
					obj = com_todo.first()
				else:
					break

			assert(not obj.changed)
			assert(obj.deps_count == 0)

			if obj.mtime == -2:
				for targ in obj.needs:
					targ.mtime = -2
				obj.changed = True
				if not target.is_phony:
					target.num_todo -= 1
				continue
			
			if verbose:
				print "considering", obj.target

			assert(len(children) <= max_children)
			
			# this is a bit tree-state here. If the obj is in the
			# tree, then it needs to be updated. But if we go through
			# the list and find one younger, we need to update. The
			# older ones can be ignored
			all_younger = True
			one_younger = False

			if not obj.mtime and not len(obj.dep_objs) and not obj.is_phony:
				# special exception: if it didn't exist and has no dependencies,
				# we assume it had to be generated. So if it now exists, we can
				# ignore it (I hope)
				if verbose:
					print "Assuming", obj.target, "needs to be generated"
				all_younger = False
				
			obj.mtime = getmtime(obj.target, force=1)
			for dep in obj.dep_objs:
				if type(dep) == types.StringType:
					dep_mtime = getmtime(dep)
				else:
					dep_mtime = dep.mtime
					assert(dep_mtime != -1)

				if not dep_mtime:
					obj.mtime = 0
						
				if obj.mtime < dep_mtime:
					one_younger = True
				else:
					all_younger = False

			if all_younger:
				one_younger = True

			obj.changed = True
			if not obj.is_phony:
				target.num_todo -= 1

			if obj.mtime and not one_younger or not obj.has_rules():
				if verbose and not one_younger:
					print "no need to update", obj.target
				for targ in obj.needs:
					assert(targ.deps_count > 0)
					targ.deps_count -= 1
					if targ.deps_count == 0 and not targ.is_todo:
						if targ.compile_target:
							com_todo.add(targ)
						else:
							non_todo.add(targ)
				continue

			if target.print_progress and target.max_todo > 0:
				progress = 100 - int(round(100 * target.num_todo / target.max_todo))
				if progress == 100:
					# don't break the layout :)
					progress = 99
				sys.stdout.write("%s%02d%%%s " % (utilities.progress_color,progress,utilities.normal))
				sys.stdout.flush()
			elif target.precise_print_progress and target.max_todo > 0:
				progress = 100.0 - (100.0 * float(target.num_todo) / float(target.max_todo))
				if progress >= 99.99:
					# don't break the layout :)
					progress = 99.99
				if progress < 10.0:
					# Add a space to pad (%02 won't work for floats)
					sys.stdout.write("%s %.2f%%%s " % (utilities.progress_color,progress,utilities.normal))
				else:
					sys.stdout.write("%s%.2f%%%s " % (utilities.progress_color,progress,utilities.normal))
				sys.stdout.flush()
				
			if max_children > 1:
				pid = call(obj, obj.makefile, fork=1)
				if obj.compile_target:
					compile_jobs += 1
					
				children[pid] = obj
				obj.mtime = -1 # mark as in process
				if len(children) >= max_children:
					break
			else:
				try:
					ret = call(obj, obj.makefile, fork=0)
				except KeyboardInterrupt, ki:
					sys.stderr.write("interrupted call, removing %s\n" % obj.target)
					try:
						os.unlink(obj.target)
					except OSError:
						pass
					raise ki
				if ret:
					if ignore_most:
						obj.mtime = -2
						utilities.sys_exit_code = ret
					else:
						sys.exit(ret)
				children[0] = obj
				break

		if verbose:
			print "finished", len(children)
		
		if len(children):
			done_something = True
			exit_code = 0
			
			if max_children > 1:
				(pid, exit_code) = os.wait()

				if exit_code:
					if not ignore_most:
						if os.WIFEXITED(exit_code):
							sys.exit(os.WEXITSTATUS(exit_code))
						sys.exit(exit_code)
					else:
						utilities.sys_exit_code = exit_code
			else:
				pid = 0

			obj = children[pid]
			del children[pid]

			if max_children > 1 and obj.compile_target:
				compile_jobs -= 1
				
			if exit_code: # ignore_most
				obj.mtime = -2

			# the above is not the only way it can get -2 (-j1)
			if obj.mtime != -2:
				if obj.is_phony:
					new_mtime = 0
				else:
					new_mtime = getmtime(obj.target, force=1)

				obj.mtime = new_mtime

				for targ in obj.needs:
					assert(targ.deps_count > 0)
					targ.deps_count -= 1
					if targ.deps_count == 0 and not targ.is_todo:
						if targ.compile_target:
							com_todo.add(targ)
						else:
							non_todo.add(targ)
			
			else: # .mtime == -2
				# every target above in the tree is marked as failed too
				for targ in obj.needs:
					targ.mtime = -2
				
	return done_something

def setup_top_makefile( top_srcdir, subdir ):
	top_makefile = merged_makefiles(".", top_srcdir + "/" + subdir)

	if top_makefile.subdir == '.':
		for key in utilities.subst_vars.keys():
			top_makefile.del_define(key)

	bins = []
	bins.extend(top_makefile.binaries.values())
	if utilities.top_builddir == '.':
		top_makefile.insertTarget('$(top_srcdir)/configure', [utilities.configure_in,
															  '$(top_srcdir)/aclocal.m4'],
								  'cd $(top_srcdir) && $(AUTOCONF)')
		if os.path.exists(top_srcdir + "/acinclude.m4"):
			# TODO: check if it generated and trace dependencies
			top_makefile.insertTarget('$(top_srcdir)/aclocal.m4', [utilities.configure_in,
																   '$(top_srcdir)/acinclude.m4'],
									  'cd $(srcdir) && $(ACLOCAL) $(ACLOCAL_AMFLAGS)')
		top_makefile.insertTarget('$(top_builddir)/config.status', ['$(top_srcdir)/configure'],
								  '$(SHELL) ./config.status --recheck')

	top_makefile.insertTarget("force-reedit", [],
							  ["cd $(top_srcdir) && %s -c %s/Makefile" %
							   (utilities.our_path, top_makefile.subdir)], phony=1)
	top_makefile.insertTarget("force-install", [],
							  ['@$(MAKE) install UNSERMAKE_FORCE_INSTALL=FORCE'], phony=1)
	top_makefile.insertTarget("FORCE", [], phony=1)
	
	return top_makefile

def setup_top_makefile_wrapper( top_makefile, top_srcdir, subdir):
		
	try:
		all_defines = read_subdirs(top_makefile, {},
								   os.path.normpath(top_srcdir + "/" + subdir  ) + "/",
								   os.path.abspath(".") + "/")
	except MakefileChanged, m:
		top_makefile = setup_top_makefile( top_srcdir, subdir )
		# very possible that the second call tries again something
		# but we ignore the second time
		all_defines = read_subdirs(top_makefile, {},
								   os.path.normpath(top_srcdir + "/" + subdir  ) + "/",
								   os.path.abspath(".") + "/",
								   ignore_makefile_updates=True)
		
	return top_makefile, all_defines

def usage():
	print "Usage: " + thisProg + " [OPTION] ... [dir]..."
	print ""
	print "Some of the switches correspond to those in \"make\" or \"automake\". See the manpages for details."
	print ""
	print "Available options:"
	print ""
	print "      --add-missing     Add missing files to package (not implemented yet)."
	print "  -c, --create          Create Makefiles (automake mode)."
	print "  -C, --directory=dir   Change to directory dir before doing anything."
	print "      --color           Add color to the output (default)."
	print "      --compile-jobs=N  Limit compile jobs to N."
	print "  -e                    Environment variables override makefiles."
	print "  -i, --ignore-errors   Ignore errors from commands."
	print "  -j, --jobs=N          Allow N parallel jobs."
	print "  -k, --keep-going      Keep going when some targets can't be made."
	print "  -l N                  Don't start multiple jobs unless load is below N (not implemented yet)."
	print "      --missing-deps    Find missing dependencies."
	print "      --no-color        Do not colorize the output."
	print "      --no-real-compare Use only mtime and not file contents to determine if a file has changed  (default)."
	print "      --real-compare    Use not only mtime but file content to determine if a file has changed."
	print "      --random=N        Make random targets."
	print "  -n  --just-print      Only print out the commands to call."
	print "  -v, --verbose         Show verbose output."
	print "      --version         Show version information and copyright notice."
	print "  -p, --print-progress  Shows an estimated percent number before each output."
	sys.exit(0)
	
def unsermake_main():
	starttime = time.time()

	global thisProg, verbose
	thisProg = "unsermake"
	recurse_flags = []
	recurse_args = []
	after_dminus = False
	try:
		makeflags = os.environ['MAKEFLAGS']
		if len(makeflags):
			inquote = False
			last_arg = ''
			for arg in string.split(makeflags, ' '):
				last_arg += arg

				if string.count(arg, '\'') % 2:
					inquote = not inquote

				if not inquote:
					if len(last_arg) > 1 and last_arg[0] == '\'' and last_arg[-1] == '\'':
						last_arg = last_arg[1:-1]
					# special GNU make hack
					if not len(recurse_flags) and len(last_arg) and last_arg[0] != '-':
						if not string.count(last_arg, '='):
							last_arg = "-" + last_arg
					if last_arg == '--':
						after_dminus = True
					elif after_dminus:
						recurse_args.append(last_arg)
					else:
						recurse_flags.append(last_arg)
					last_arg = ''

	except KeyError:
		pass

	utilities.our_path = "unsermake"
	if len(recurse_args):
		recurse_args = ["--"] + recurse_args
	if os.environ.has_key('UNSERMAKE_OPTS'):
		recurse_flags = recurse_flags + string.split(os.environ['UNSERMAKE_OPTS'])
	recurse_flags = recurse_flags + sys.argv[1:] + recurse_args
	
	moduledirs = []

	added_delim = False

	commandline_vars = {}
	eqre = re.compile('^([^=]*)=(.*)$')

	optlist = []
	given_files = []

	while len(recurse_flags):
		try:	
			t_optlist, given_files = getopt.getopt(recurse_flags, 'cvhr:f:C:kij:el:nps', [
				'create', 'add-missing', 'version', 'verbose', 'help', 'random=', 'missing-deps', 
				'compile-jobs=', 'directory=', 'ignore-errors', 'keep-going', 'jobs=', 'just-print',
				'print-progress', 'real-compare', 'no-real-compare', 'modules=','color','no-color'])
			optlist.extend(t_optlist)
			recurse_flags = []
			if len(given_files):
				match = eqre.match(given_files[0])
				if match: # variable assignment on command line
					commandline_vars[match.group(1)] = match.group(2)
					recurse_flags.extend(given_files[1:])
		
		except getopt.GetoptError:
			print "Wrong parameters."
			usage();
			sys.exit(2);

	targets = []
	for arg in given_files:
		match = eqre.match(arg)
		if match: # variable assignment on command line
			commandline_vars[match.group(1)] = match.group(2)
		else:
			targets.append(arg)

	# print optlist, targets, commandline_vars.keys()
	
	# default values
	random_targets = 0
	create_mode = False
	print_progress = False
	precise_print_progress = False
	mtime_only = True
	color_mode = sys and sys.stderr.isatty() and sys.stdout.isatty() and "TERM" in os.environ and not os.environ["TERM"].upper() in ["DUMB"] and not "EMACS" in os.environ
	file_to_read = "Makefile"
	makeflags = ''

	global max_children

	for option, param in optlist:
		if option == '--version':
			print thisProg + " 0.4"
			print 'Written by Stephan Kulow using concepts of Michael Matz'
			print 'and Simon Hausmann. In parts based on concepts of "am_edit"'
			print 'and "GNU Automake".'
			print ''
			print 'This is free software; see the source for copying conditions.  There is NO'
			print 'warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.'
			sys.exit(0)
		elif option == '--verbose' or option == '-v':
			verbose = True
		elif option == '--help' or option == '-h':
			usage();
		elif option == '--random' or option == '-r':
			utilities.default_force = 0
			random_targets = string.atoi(param)
		elif option == '--create' or option == '-c':
			create_mode = True
		elif option == '--color':
			color_mode = True
		elif option == '--no-color':
			color_mode = False
		elif option == '-f':
			file_to_read = param
		elif option == '--directory' or option == '-C':
			os.chdir(param)
		elif option == '--missing-deps':
			amfile.find_missing_deps = True
		elif option == '--ignore-errors' or option == '-i':
			amfile.ignore_errors = True
			makeflags += "-i "
		elif option == '--keep-going' or option == '-k':
			global ignore_most
			ignore_most = True
			makeflags += "-k "
		elif option == '--jobs' or option == '-j':
			try:
				max_children = string.atoi(param)
			except:
				utilities.print_error("you have to pass an integer after -j\n")
				sys.exit(1)
			makeflags += "-j %d " % max_children
		elif option == '--just-print' or option == '-n':
			target.just_print = True
			makeflags += "-n "
		elif option == '-e':
			for var in os.environ.keys():
				utilities.environment_vars[var] = os.environ[var]
			makeflags += "-e "
		elif option == '-p' or option == '--print-progress':
			if print_progress or precise_print_progress:
                # This is not the first -p option, so set the "precise" progress
				precise_print_progress = True
				print_progress = False
			else:
				precise_print_progress = False
				print_progress = True
                # Set makeflags only at the first -p
				makeflags += "-p "
		elif option == '-l':
			print 'the option %s is ignored (for now)' % option
		elif option == '-s':
			continue # see above - but silently ;/
		elif option == '--compile-jobs':
			global max_compile_jobs
			try:
				max_compile_jobs = string.atoi(param)
			except:
				utilities.print_error("you have to pass an integer after --compile-jobs\n")
				sys.exit(1)
			makeflags += "--compile-jobs %s " % max_compile_jobs
		elif option == '--no-real-compare':
			mtime_only = True
			makeflags += "--no-real-compare "
		elif option == '--real-compare':
			mtime_only = False
			makeflags += "--real-compare "
		elif option == '--modules':
			moduledirs.append(param)
			
	max_children = max_children + max_compile_jobs
	
	# Register list of file handlers, as well as setting our_path to the
	# absolute path to unsermake for instances like sudo with --secure-path.
	# Assumes that one of the module dirs has the unsermake script.
	handlers = []
	for sourcedir in moduledirs:
		files = os.listdir(sourcedir)
		for l in files:
			if l.endswith('.um'):
				handlers.append(os.path.join(sourcedir, l))
			if l == 'unsermake':
				utilities.our_path = sourcedir + '/' + l

	handlers.sort()
	
	for file in handlers:
		execfile(file)
	if verbose:
		print "registered handlers:", handlerbase.handlers

	if not color_mode:
		utilities.clearAllColors()

	if create_mode:
		utilities.configure_in = "$(top_srcdir)/configure.in"
		if os.path.exists("configure.ac"):
			utilities.configure_in = "$(top_srcdir)/configure.ac"
		else:
			if not os.path.exists("configure.in"):
				print 'There must be a configure.in or configure.ac in the current directory for --create.'
				sys.exit(1)

		utilities._topsrcdir = os.path.abspath(os.curdir) + "/"
		create_makefiles(targets)
		return

	if not os.path.exists(file_to_read):
		print "no file", file_to_read, "present"
		sys.exit(1)

	added_delim = False

	for var, value in commandline_vars.items():
		utilities.environment_vars[var] = value
		os.environ[var] = value
		if not added_delim:
			makeflags += "-- "
			add_delim = True
		makeflags += "'%s=%s' " % (var,value)
	
	if makeflags and makeflags[-1] == ' ':
		makeflags = makeflags[:-1]

	utilities.subst_vars = {}
	add_subst_var('MAKE', utilities.our_path)
	export_var('MAKE')
	add_subst_var('MAKEFLAGS', makeflags)
	export_var('MAKEFLAGS')
	add_subst_var('SHELL', '/bin/sh')
	export_var('SHELL')
				
	# top_makefile is just of temporary value
	top_makefile = amfile.AMFile(file_to_read, ignore_comments=False)
	if not top_makefile.is_defined("top_srcdir"): # TODO: put some UNSERMAKE_was_here in there
		overwrite_makefile(top_makefile)
		all_defines = append_all_defines({}, top_makefile)
		try:
			if not len(targets):
				targets = [top_makefile.first_target]
			for targ in targets:
				finish(all_defines, targ)
		except NoSuchTarget, t:
			print "no rule to create target: %s" % t.target
			sys.exit(1)
		sys.exit(0) # if not exited before

	if not len(targets):
		targets = ["all"]
		
	top_srcdir = top_makefile.value_of("top_srcdir")
	utilities._topsrcdir = os.path.abspath(top_srcdir)
	if os.path.exists(utilities._topsrcdir + "/configure.ac"):
		utilities.configure_in = "$(top_srcdir)/configure.ac"
	else:
		utilities.configure_in = "$(top_srcdir)/configure.in"
	if not utilities._topsrcdir.endswith("/"):
		utilities._topsrcdir = utilities._topsrcdir + "/"
	utilities.top_builddir = top_makefile.value_of("top_builddir")
	if  utilities.top_builddir and utilities.top_builddir != '.':
		top_makefile = amfile.AMFile(utilities.top_builddir + "/Makefile", ignore_comments=False)
	else:
		utilities.top_builddir = '.'

	builddir = os.path.abspath(os.curdir)
	utilities.top_builddir_abs = os.path.abspath(os.curdir + "/" + utilities.top_builddir)
	subdir = builddir[len(utilities.top_builddir_abs)+1:]

	for cond in top_makefile.defines():
		if cond in ['subdir', 'top_builddir', 'srcdir', 'top_srcdir']:
			continue
		if cond.endswith("_FALSE"):
			continue
		if not cond.endswith("_TRUE"):
			if not utilities.environment_vars.has_key(cond):
				utilities.subst_vars[cond]=top_makefile.value_of(cond)
			top_makefile.del_define(cond)
			continue
		cond = cond[:-5]
		true = top_makefile.value_of(cond + '_TRUE')
		top_makefile.del_define(cond + '_TRUE')
		false = top_makefile.value_of(cond + '_FALSE')
		top_makefile.del_define(cond + '_FALSE')
		if true == '#' and not len(false):
			utilities.false_conds.append(cond)
			continue
		if false == '#' and not len(true):
			utilities.true_conds.append(cond)
			continue
		print 'unknown cond', cond

	add_subst_var('top_builddir', utilities.top_builddir)
	# we take the absolute form (without trailing /) so we avoid confusion with
	# relative build dirs
	# TODO: if someone is bored enough, he could put in here a @top_srcdir@ that
	# is then way later expanded back to the relative version. This only affects
	# user_specified rules as we got a race between variable expansion and build
	# dir expansion (amfile.replace_builddir)
	add_subst_var('top_srcdir', utilities._topsrcdir[:-1])
	add_subst_var('INSTALL_HEADER', '$(INSTALL_DATA)')
	add_subst_var('CXXLD', '$(CXX)')
	add_subst_var('CLD', '$(CC)')
	add_subst_var('AR', 'ar')
	add_subst_var('UNSERMAKE_FORCE_INSTALL', '')
	add_subst_var('INSTALL_STRIP_FLAG', '')

	# these variables are expected to used from the environment
	# and not from configure
	for var in ["DESTDIR", "INSTALL_STRIP_FLAG", "INSTALL_PROGRAM_ENV",
				"transform", "AM_MAKEFLAGS", "VERBOSE"]:
		add_subst_var(var, '')
		
	add_subst_var('V_ECHO', 'test -n "$(VERBOSE)" || echo')
	add_subst_var('DEFAULT_INCLUDES', '-I. -I$(srcdir) -I$(top_builddir)')
	add_subst_var('V_EXEC', "if test -n \"$(VERBOSE)\"; then "\
				  "echo \"$$@\"; \"$$@\"; "\
				  "	else "\
				  "	  rslt=$$(\"$$@\" 2>&1); stat=$$?; "\
				  "	  if test $$stat -ne 0; then "\
				  "	    echo \"$$@\"; echo \"$$rslt\" >&2; "\
				  "	  else "\
				  "	    test -n \"$$rslt\" && echo \"$$rslt\" >&2; "\
				  "	  fi; "\
				  "	  exit $$stat;"\
				  "	fi")
	add_subst_var('V_COMPILE', "if test -n \"$(VERBOSE)\"; then echo \"$$@\"; \"$$@\"; "\
				  "else rslt=$$(\"$$@\" 2>&1); fi; stat=$$?; "\
				  "	if test $$stat -ne 0; then "\
				  "	  if test -z \"$(VERBOSE)\"; then echo \"$$@\"; echo \"$$rslt\" >&2; fi; "\
				  "	else "\
				  "	  test -n \"$$rslt\" && echo \"$$rslt\" >&2; "\
				  "	  echo \"$$targetfile : \\\\\" > \"$$depfile.tmp\"; "\
				  "	  deps=`sed -e 's,.*:,,' $$tmpdepfile | sed -e 's,\\\\\\\\,,g'`; "\
				  "   echo 'builddir=$$(top_builddir)/$(subdir)' > $$depfile.tmp ;"\
				  "   echo 'top_builddir=$(top_builddir)' >> $$depfile.tmp ;"\
				  "   echo \"targetfile=$$targetfile\" >> $$depfile.tmp ;"\
				  "   for dep in $$deps ; do echo $$dep >> $$depfile.tmp ; done ;"\
				  "   mv $$depfile.tmp $$depfile; "\
				  "	fi; "\
				  "	rm -f \"$$tmpdepfile\"; exit $$stat")

	# not really sure who would use these
	package = utilities.subst_vars['PACKAGE']
	add_subst_var('pkgdatadir', '$(datadir)/' + package)
	add_subst_var('pkglibdir', '$(libdir)/' + package)
	add_subst_var('pkgincludedir', '$(includedir)/' + package)

	top_makefile = setup_top_makefile( top_srcdir, subdir )
	
	try:
		(top_makefile, all_defines) = setup_top_makefile_wrapper( top_makefile, top_srcdir, subdir)
	except OldConfigureIn, o:
		if top_makefile.targets.has_key(utilities._topsrcdir + "configure.in"):
			print "removing ",  utilities._topsrcdir + "configure.in"
			os.unlink( utilities._topsrcdir + "configure.in")
			(top_makefile, all_defines) = setup_top_makefile_wrapper( top_makefile, top_srcdir, subdir)
		else:
			utilities.print_error("there is no way to generate the Makefile for %s\n" %
								  o.filename)
			sys.exit(1)

	if amfile.find_missing_deps:
		if os.path.abspath(utilities.top_builddir) == os.path.abspath(top_srcdir):
			print "top_srcdir is the same as top_builddir. This doesn't work with --missing-deps"
			sys.exit(1)
		keys = amfile.found_missing_deps.keys()
		keys.sort()
		gwd = os.getcwd()
		os.chdir(top_srcdir)
		utilities.subst_vars, files, headers, auxdir = utilities.parse_autoconf()
		os.chdir(gwd)
		
		cheaders = []
		for header in headers + files:
			cheaders.append('$(top_builddir)/' + header)

		old_dir = None
		lines = ''

		print "\nFound Missing Dependencies:\n"
		
		for key in keys:
			custom_keys = []
			dir = os.path.dirname(key)
			if dir.startswith("$(top_builddir)/"):
				dir = dir[len("$(top_builddir)/"):]
			if dir != old_dir:
				if lines:
					print "%s/Makefile.am:" % old_dir
					print lines
					lines = ''
				old_dir = dir
				
			for dep2 in amfile.found_missing_deps[key]:
				if dep2 in cheaders:
					continue
				dep_dirs = string.split(dep2, '/')
				targetdirs = string.split(key, '/')
				index=0
				while index < min(len(targetdirs), len(dep_dirs)) - 1:
					if targetdirs[index] != dep_dirs[index]:
						break
					index += 1

				targetdirs = targetdirs[index:]
				dep_dirs = dep_dirs[index:]

				dep2 = ''
				for index in range(0, len(targetdirs) - 1):
					dep2 += "../"
				for index in range(0, len(dep_dirs) - 1):
					dep2 += dep_dirs[index] + "/"
				dep2 += dep_dirs[-1]
				if not dep2 in custom_keys:
					custom_keys.append(dep2)
			if len(custom_keys):
				lines += "%s: %s\n" % (os.path.basename(key),
									   string.join(custom_keys))
		if lines:
			print "%s/Makefile.am:" % old_dir
			print lines
				
		sys.exit(0)
	
	#for r in xrange(0, random_targets):
	#	b = random.choice(bins)
	#	b.add_random()
	#	bins.remove(b)

	top_makefile.final_reorder()
		
	if utilities.sys_exit_code:
		sys.exit(utilities.sys_exit_code)
	
	if verbose:
		top_makefile.filename = "Makefile.out"
		top_makefile.print_out(1)

	target.print_progress = print_progress
	target.precise_print_progress = precise_print_progress
	
	sourcehash_file = utilities.top_builddir+'/SourceHash'
	if not mtime_only:
		global sourcehash
		sourcehash = SourceHash(sourcehash_file)
	
	for targ in targets:
		try:
			finish(all_defines, targ)
		except NoSuchTarget, t:
			try:
				btargets = top_makefile.replace_builddir([targ])
				if len(btargets) == 1:
					btarget = top_makefile.expand(btargets[0])
					finish(all_defines, btarget)
			except NoSuchTarget, t2:
				filename = t2.filename
				orig = t2.orig
				targ = t2.target
				if not len(t2.filename):
					filename = t.filename
					orig = t.orig
					targ = t.target
				print "%s: no rule to create target: %s(%s)" % (filename, targ, orig)
				sys.exit(1)
	
	if sourcehash:
		sourcehash.save(sourcehash_file)

def main():
	if os.environ.has_key('PROFILE'):
		import profile
		profile.run("unsermake_main()", 'fooprof')
		
		import pstats
		p = pstats.Stats('fooprof')
		p.strip_dirs()
		p.sort_stats('cumulative').print_stats(10)
		p.sort_stats('time').print_stats(10)

	else:

		# If Psyco is installed, use it.
		# Psyco speeds dramatically up Python, see:
		# http://psyco.sourceforge.net/
		try:
			import psyco
			psyco.full()
		except ImportError:
			# Psyco is optional, silently accept its abscence
			pass
		
		try:
			unsermake_main()
		except KeyboardInterrupt, ki:
			sys.exit(130)
	
		sys.exit(utilities.sys_exit_code)

# vim: ts=4
