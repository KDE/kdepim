#include <qstring.h>
#include <qcstring.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qdir.h>

#include <kdatastream.h>
#include <dcopclient.h>
#include <kapp.h>
#include <kcmdlineargs.h>

#include "KAddressBookInterface.h"
#include "KAddressBookInterface_stub.h"
#include "Entry.h"
#include "Field.h"

const char * accountsFilename = "/x/cvs/kde/head/kde-common/accounts";

  int
main(int argc, char ** argv)
{
	KCmdLineArgs::init(argc, argv, "blah", "blah", "blah");

  new KApplication(false, false);

	DCOPClient * client = new DCOPClient;

	if (!client->attach())
    qFatal("Can't attach to DCOP");

  QStringList addressBookList;

  {
    QByteArray args, retVal;
    QCString retType;

    bool ok =
      client->call
      (
       "KAddressBookServer",
       "KAddressBookServer",
       "list()",
       args,
       retType,
       retVal
      );

    if (!ok)
      qFatal("Can't talk to KAddressBook server");

    QDataStream str(retVal, IO_ReadOnly);

    str >> addressBookList;
  }

  if (addressBookList.contains("cvs accounts"))
  {
    qDebug("CVS accounts addressbook already exists. Not creating.");
    return 1;
  }

  {
    QByteArray args, retVal;
    QCString retType;

    QDataStream inStr(args, IO_WriteOnly);

    inStr << QString("cvs accounts") << QString("/tmp/cvs_accounts.kab");

    bool ok =
      client->call
      (
       "KAddressBookServer",
       "KAddressBookServer",
       "create(QString,QString)",
       args,
       retType,
       retVal
      );

    if (!ok)
      qFatal("Can't call create()");

    QDataStream outStr(retVal, IO_ReadOnly);

    outStr >> ok;

    if (!ok)
      qFatal("Can't create addressbook");
  }

  KAddressBook_stub * ab =
    new KAddressBook_stub("KAddressBookServer", "cvs accounts");

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

    Entry e;

    e.addField(Field("username", user));
    e.addField(Field("name", name));
    e.addField(Field("email", mail));

    ab->insert(e);
  }
}
