# $Id$

SUBDIRS = en 

# this is a general list for merging!
APPS = kpilot 

merge:
	@apps='$(APPS)'; \
	langs='$(SUBDIRS)'; \
	for cat in $$apps; do \
	  echo $$cat; \
	  for lang in $$langs; do \
	    echo $$lang/$$cat ; \
	    if test -r $$lang/$$cat.po; then \
	      echo $$lang; \
	      msgmerge -o $$cat.new $$lang/$$cat.po $$cat.pot; \
	      if test -s $$cat.new && diff $$lang/$$cat.po $$cat.new; then \
		rm $$cat.new;  \
	      else  \
		mv $$cat.new $$lang/$$cat.po ; \
	    fi; \
	  fi ;\
	 done ;\
	done

messages:
