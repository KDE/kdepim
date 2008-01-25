SUBDIRS = ktnef lib gui tests

messages: rc.cpp
	$(EXTRACTRC) gui/*.rc gui/*.ui >> rc.cpp
	$(XGETTEXT) rc.cpp gui/*.cpp lib/*.cpp -o $(podir)/ktnef.pot
