SUBDIRS = ktnef lib gui

messages: rc.cpp
	$(EXTRACTRC) gui/*.rc gui/*.ui >> rc.cpp
	$(XGETTEXT) rc.cpp gui/*.cpp ktnef/*.cpp lib/*.cpp -o $(podir)/ktnef.pot
