## $Id$

if compile_kio_sieve
  SIEVE_SUBDIR = sieve
endif

if compile_kioslave_ldap
  LDAP_SUBDIR=ldap
endif
SUBDIRS = imap4 mbox pop3 smtp $(SIEVE_SUBDIR) $(LDAP_SUBDIR)

DOXYGEN_REFERENCES = kioslaves/imap4 kioslaves/mbox
include $(top_srcdir)/admin/Doxyfile.am

