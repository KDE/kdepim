# Makefile for the conduit programming tutorial

# This will probably go horribly wrong because all of the
# files have been reduced to just the sources, and so
# pdflatex can't read the required embedded pdf / eps
# pictures that should be created from the png's.


all : index.dvi index.pdf



# latex index && pdflatex index && latex2html -local_icons index && latex2rtf index && dvips index 
#
# We'll need to create the required pdf files in pictures/ as well.


ConduitStructures.pdf : ConduitStructure.eps
	epstopdf ConduitStructure.eps

index.dvi : index.tex
	latex index

index.pdf : ConduitStructures.pdf index.tex index.dvi
	pdflatex index
