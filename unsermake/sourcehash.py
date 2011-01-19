import zlib
import cPickle as pickle

class Source:
	def __init__(self, hash, mtime):
		self.hash = hash
		self.ctime = mtime # hash changed time
		self.mtime = mtime

class SourceHash:
	def __init__(self, filename = ''):
		try:
			self.__src = pickle.load(file(filename,'r'))
		except IOError:
			self.__src = {}
		except EOFError:
			self.__src = {}
		self.__dirty = False
	
	def save(self, filename):
		if self.__dirty:
			pickle.dump(self.__src, file(filename,'w'))
	
	def strip(self, filename):
		data = file(filename).read()
		white = " \t\n"
		punct = r'!%^&*()-+=[]{};:~<>,./?#'
		
		inWhite = [False]
		inComment1 = False
		inComment2 = False
		inQuote = None
		escaped = False
		
		result = []
		
		def setInWhite(white = True):
			if inWhite[0] != white:
				inWhite[0] = white
				if white:
					result.append(' ')
		
		# assume starts with whitespace
		prev = ' '
		setInWhite()
		for char in data:
			# ending comments
			if inComment1 and char == '\n' and prev != '\\':
				inComment1 = False
			elif inComment2 and char == '/' and prev == '*':
				inComment2 = False
				char = ' ' # /**// is not start of //
			# in comments - ignore
			elif inComment1 or inComment2:
				pass
			# ending quote
			elif char == inQuote and not escaped:
				inQuote = False
				result.append(char)
			# in quote - copy
			elif inQuote:
				if char == '\\':
					escaped = not escaped
				else:
					escaped = False
				result.append(char)
			# starting comments - strip previous /
			elif char == '/' and prev == '/':
				# remove /
				result.pop()
				# find out if we were in whitespace
				if result[-1] == ' ':
					inWhite[0] = True
				inComment1 = True
				setInWhite()
			elif char == '*' and prev == '/':
				result.pop()
				if result[-1] == ' ':
					inWhite[0] = True
				inComment2 = True
				setInWhite()
			# starting quote
			elif char == '"' or char == "'":
				escaped = False
				inQuote = char
				# assume whitespace before quote
				setInWhite()
				setInWhite(False)
				result.append(char)
			# whitespace - collapse to single space
			elif char in white:
				setInWhite()
			# other character: record
			else:
				punc = char in punct
				ppunc = prev in punct
				# assume space between punctuation and identifiers
				if punc != ppunc:
					setInWhite()
				result.append(char)
				setInWhite(False)
			prev = char
		# assume ends with whitespace
		setInWhite()
		
		if inQuote:
			print 'Warning: %s ends in quote' % filename
		if inComment1 or inComment2:
			print 'Warning: %s ends in comment' % filename
		
		return ''.join(result)
	
	def hash(self, filename):
		return zlib.adler32(self.strip(filename))
	
	def mtime(self, filename, real_mtime):
		try:
			try:
				src = self.__src[filename]
				if src.mtime == real_mtime:
					# cache up-to-date
					return src.ctime
				new_hash = self.hash(filename)
				src.mtime = real_mtime
				self.__dirty = True
				if src.hash != new_hash:
					# file changed
					src.hash = new_hash
					src.ctime = real_mtime
					return real_mtime
				else:
					# file appears unchanged
					print 'skipping %s' % filename
					return src.ctime
			except KeyError:
				myhash = self.hash(filename)
				self.__src[filename] = Source(myhash, real_mtime)
				self.__dirty = True
				return real_mtime
		except IOError:
			return real_mtime
