Summary: MUA for KDE
Name: empath
Version: 1.0
Release: 1
Copyright: GPL
Group: Applications/Mail
Source: ftp://without.netpedia.net/empath/
BuildRoot: /tmp/empath_build

%description
Empath is a mail client for KDE. Empath supports local (Mbox, MMDF, MH, Maildir)
and remote (POP3 and IMAP4) folders. Empath is highly configurable, looks great,
does threads nicely, and is GPL. What more could you want ?

%prep
%setup

%build
echo "Welcome to the long, drawn out Empath build process. This usually takes"
echo "a while, so put the kettle on."
./configure
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
make install

%clean
make clean

%files
