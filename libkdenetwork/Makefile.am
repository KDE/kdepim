INCLUDES = $(all_includes) -I$(top_builddir)/libkdepim

if needs_gpgme_copy
GPGME_COPY_SUBDIRS = libgpg-error-copy libgpgme-copy
endif
SUBDIRS = $(GPGME_COPY_SUBDIRS) gpgmepp qgpgme .

include ../admin/Doxyfile.am
