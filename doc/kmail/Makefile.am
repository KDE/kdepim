
docs_DATA = logotp3.png index.html index-1.html index-2.html index-3.html \
	index-4.html index-5.html index-6.html index-7.html index-8.html \
	index.html

docsdir = $(kde_htmldir)/en/kmail

EXTRA_DIST = $(docs_DATA) index.sgml index.nif

doc: index.sgml index.nif
	ksgml2html index en
