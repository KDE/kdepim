SUBDIRS = shared kabc knotes kcal

updatedir = $(kde_datadir)/kconf_update
update_DATA = kolab-resource.upd
update_SCRIPTS = upgrade-resourcetype.pl 

messages: rc.cpp
	$(XGETTEXT) */*.cpp -o $(podir)/kres_kolab.pot
