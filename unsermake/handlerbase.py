class HandlerBase:

	def __init__(self):
		self.name = "Handler"
		
	def __repr__(self):
		return self.name
		
	# returns a couple of lists.
	# the first list is the list of handlers that have to run before this one
	# and the second list is the list of handlers that have to run after this one
	def requirements(self):
		return ([], [])
	
	# this is called to find out the file types the Handler works
	# for. So return here an array of extensions in the form
	# ['.s', '.S'] (for assembler)
	def extensions(self):
		return []

	# this is called for every source file in _SOURCES that fits
	# your given extensions.
	# The paramters are:
	# * program: is an instance of a Program target (can be a library
	#            too (everything that can have a _SOURCES)
	# * base: the basename of the source file (e.g. "hello")
	# * ext: the extension of the source file (e.g. ".s")
	def source(self, program, base, ext):
		pass
	
	# this is called for every source file in _SOURCES that fits
	# your given extensions in create mode
	# The paramters are:
	# * program: is an instance of a Program target (can be a library
	#            too (everything that can have a _SOURCES)
	# * base: the basename of the source file (e.g. "hello")
	# * ext: the extension of the source file (e.g. ".s")
	def create_source(self, program, base, ext):
		pass

	# this is called after the initial parsing is done to give the Handler
	# a chance to either check something in the parsed Instance or to change
	# something. Handle with care
	# The parameter is an instance of an AmFile
	# The function is called in both create and run mode
	def parse(self, amfile):
		pass

	# this is called for every parsed makefile to do long term actions that
	# should only run in create mode
	# The parameter is an instance of an AmFile
	# The function is called only in create mode
	def create(self, amfile):
		pass

	# this is called for every parsed makefile to collect whatever the handler
	# has to collect. It's not really possible to create new source files in here
	# so do this in create (s.a.)
	def collect(self, amfile):
		pass

	# this is called for every parsed makefile _after_ parsing is done but _before_
	# the compile lines for the programs are created. This allows to write handlers
	# that move sources around or create additional binaries, etc.
	def shuffle_binaries(self, amfile):
		pass

ext_dict = {}
handlers = []

def add_handler(handler):
	self_before, self_after = handler.requirements()

	last_index = -1
	
	for index in range(0, len(handlers)):
		before, after = handlers[index].requirements()
		if last_index < 0 and handler.name in before:
			last_index = index
		if last_index < 0 and handlers[index].name in self_after:
			last_index = index

	if last_index >= 0:
		handlers[last_index:last_index + 1] = [handler, handlers[last_index]]
	else:
		handlers.append(handler)

	handler_index = {}

	# topologic sort
	for index in range(0, len(handlers)):
		hand = handlers[index]
		handler_index[hand.name] = index
		before, after = hand.requirements()
		for handler in after:
			if handler_index.has_key(handler):
				if handler_index[handler] < index:
					handler = handlers[index]
					del handlers[index]
					add_handler(handler)
					break
		for handler in before:
			if handler_index.has_key(handler):
				if handler_index[handler] > index:
					handler = handlers[index]
					del handlers[index]
					add_handler(handler)
					break
	
def register_handler(handler):
	for ext in handler.extensions():
		ext_dict[ext] = handler

	add_handler(handler)
	
