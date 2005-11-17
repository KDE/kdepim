#!/usr/bin/env python
import random
import re

def init_chain(infile):
	chain = {}
	last = ('','')
	for line in infile:
		for word in line.split():
			if not chain.has_key(last):
				chain[last]=[]
			chain[last].append(word)
			last=(last[1],word)
	chain[last]=None
	return chain

def output(chain,length,outputfile):
	last = ('','')
	start=2000
	for i in range(length+start):
		if chain[last] is None:
			break
		word = random.choice(chain[last])
		last=(last[1],word)
		if i > start:
			outputfile.write(word)
			outputfile.write(' ')
	outputfile.write("\n")

def get_words(chain,nwords,outputfile,scriptfile):
	scriptfile.write("(for f in output/text_*; echo $f) > tmp/so_far\n")
	for i in range(nwords):
		word='1'
		while re.compile("\d").search(word):
			word=random.choice(random.choice(chain.keys()))
		word=re.sub(r'\W','',word)
		outputfile.write(word+"\n")
		scriptfile.write("grep -i -E -e '(\W|^)%s' -l output/text_* >tmp/part_%s\n" % (word,word))
		scriptfile.write("perl -e '($file1, $file2) = @ARGV; open F2, $file2; while (<F2>) {$h2{$_}++}; open F1, $file1; while (<F1>) {if ($h2{$_}) {print $_; $h2{$_} = 0;}}' tmp/part_%s tmp/so_far >tmp/so_far_\n" % word) # From scriptome
		scriptfile.write("mv tmp/so_far_ tmp/so_far\n")
		scriptfile.write("rm tmp/part_%s\n" % word)
	scriptfile.write("mv tmp/so_far tmp/expected\n")
		

chain=init_chain(file("/dev/stdin"))
for i in range(10000):
	output(chain,2000,file("output/text_"+str(i+1),'w'))


for i in range(1000):
	get_words(chain,random.randint(1,5),file("output/words_%s.list"%str(i+1),'w'),file("output/words_%s.script"%str(i+1),'w'))
