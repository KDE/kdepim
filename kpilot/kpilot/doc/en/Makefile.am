###
#
# NOTE that these HTML documents should be derived from the
# docbook sources. However, until everyone has the docbook 
# tools we'll install just the HTML stuff.
#
# Only the HTML and images are installed; the docbook stuff 
# goes into distributions, though.
#
# REMEMBER to change the language (locale) at the bottom
# of this file! And note that the command docbook is
# unlikely to exist elsewhere -- it should be whatever
# you use to covert KDE-DocBook to HTML.
#
###
KPILOT_DOCBOOK=kpilot.docbook usage.docbook

KPILOT_HTML=compilation.html    faq.html      installation.html \
	configuration.html  hotsync.html  introduction.html \
	database.html       index.html    requirements.html \
	credits.html using-kpilot.html conduits.html

KPILOT_PNG=setup-general.png setup-address.png setup-dbspecial.png \
	toolbar_backup.png memo-app.png toolbar.png address-app.png  \
	file-app.png



EXTRA_DIST = $(KPILOT_HTML) $(KPILOT_PNG) $(KPILOT_DOCBOOK)
data_DATA = $(KPILOT_HTML) $(KPILOT_PNG)

# this is the directory, where all datas are installed
# you have given the datas in data_DATA
datadir = $(kde_htmldir)/en/kpilot

###
#
# Note that running the docbook tools once will
# re-build all the HTML files.
#
###
$(KPILOT_HTML) : .allhtml

.allhtml : $(KPILOT_DOCBOOK)
	echo "You need jw and the like to convert the docbook sources to HTML"
	docbook "-o ." kpilot.docbook
	touch .allhtml
