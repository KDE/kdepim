#include <qstring.h>
#include <qcstring.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>

#include <kdatastream.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include <kab2/KAddressBookServerInterface.h>
#include <kab2/KAddressBookServerInterface_stub.h>
#include <kab2/KAddressBookInterface.h>
#include <kab2/KAddressBookInterface_stub.h>
#include <kab2/Entry.h>
#include <kab2/Field.h>

const char * accountsFilename = "/x/cvs/kde/head/kde-common/accounts";

  int
main(int argc, char ** argv)
{
	KCmdLineArgs::init(argc, argv, "blah", "blah", "blah");

  new KApplication(false, false);

	DCOPClient * client = new DCOPClient;

	if (!client->attach())
    qFatal("Can't attach to DCOP");

  KAddressBookServerInterface_stub server
    ("KAddressBookServer", "KAddressBookServer");

  QStringList addressBookList = server.list();

  if (addressBookList.contains("cvs accounts"))
  {
    qDebug("CVS accounts addressbook already exists. Not creating.");
    return 1;
  }

  bool createOK = server.create("cvs accounts", "/tmp/cvs_accounts.kab", 
      QFileInfo("test_kab_definition").absFilePath());

  if (!createOK)
  {
    qWarning("Can't create a new addressbook");
    return 1;
  }

  KAddressBookInterface_stub * ab =
    new KAddressBookInterface_stub("KAddressBookServer", "cvs accounts");

  QFile f(accountsFilename);

  if (!f.open(IO_ReadOnly))
    qFatal("Can't open accounts file at %s", accountsFilename);

  QTextStream t(&f);

  while (!t.atEnd())
  {
    QString line = t.readLine();

    QString user = line.left(11).stripWhiteSpace();
    QString name = line.mid(11, 30).stripWhiteSpace();
    QString mail = line.mid(42).stripWhiteSpace();

    qDebug("user: %s", user.ascii());
    qDebug("name: %s", name.ascii());
    qDebug("mail: %s", mail.ascii());

    KAB::Entry e;

    e.addField(KAB::Field("username", user));
    e.addField(KAB::Field("name",     name));
    e.addField(KAB::Field("email",    mail));

    ab->insert(e);
  }
}
