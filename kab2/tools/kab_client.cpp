#include <stdio.h>

#include <qstring.h>
#include <qcstring.h>

#include <dcopclient.h>
#include <kapp.h>
#include <kinstance.h>
#include <kcmdlineargs.h>
#include <kdatastream.h>
#include <klocale.h>
#include <kaboutdata.h>

#include "KAddressBookInterface.h"

static const char * description = I18N_NOOP("KDE addressbook creation tool");
static const char * version = "v0.1";
static DCOPClient * client = 0;

static KCmdLineOptions options[] =
{
  { "commands", I18N_NOOP("Show available commands."), 0 },
  { "+command", I18N_NOOP("Command (see --commands)."), 0 },
  { 0, 0, 0 }
};

  bool
create(QString name, QString url, QString fmt)
{
  qDebug("name: %s", name.ascii());
  qDebug("url: %s", url.ascii());
  qDebug("fmt: %s", fmt.ascii());

  QByteArray params, retVal;

  QCString retType;

  QDataStream paramStr(params, IO_WriteOnly);

  paramStr << name << url << fmt;

  bool ok =
    client->call
    (
     "KAddressBookServer",
     "KAddressBookServer",
     "create(QString,QString,QString)",
     params,
     retType,
     retVal
    );

  if (!ok)
  {
    fprintf(stderr, "kab_client: can't communicate with KAddressBook server.\n");
    return false;
  }

  QDataStream str(retVal, IO_ReadOnly);

  str >> ok;

  if (!ok)
  {
    fprintf(stderr, "kab_client: can't create addressbook `%s'.\n", name.latin1());
    return false;
  }

  return true;
}

  bool
remove(QString name)
{
  qDebug("name: %s", name.ascii());

  QByteArray params, retVal;

  QCString retType;

  QDataStream paramStr(params, IO_WriteOnly);

  paramStr << name;

  bool ok =
    client->call
    (
     "KAddressBookServer",
     "KAddressBookServer",
     "remove(QString)",
     params,
     retType,
     retVal
    );

  if (!ok)
  {
    fprintf(stderr, "kab_client: can't communicate with KAddressBook server.\n");
    return false;
  }

  QDataStream str(retVal, IO_ReadOnly);

  str >> ok;

  if (!ok)
  {
    fprintf(stderr, "kab_client: can't remove addressbook `%s'.\n", name.latin1());
    return false;
  }

  return true;
}

  void
list()
{
  QByteArray params, retVal;

  QCString retType;

  bool ok =
    client->call
    (
     "KAddressBookServer",
     "KAddressBookServer",
     "list()",
     params,
     retType,
     retVal
    );

  if (!ok)
  {
    fprintf(stderr, "kab_client: can't communicate with KAddressBook server.\n");
    return;
  }

  QDataStream str(retVal, IO_ReadOnly);

  QStringList abList;
  str >> abList;

  if (!ok)
  {
    fprintf(stderr, "kab_client: can't read addressbook list.\n");
    return;
  }

  for (QStringList::ConstIterator it(abList.begin()); it != abList.end(); ++it)
  {
    qDebug("%s", (*it).local8Bit().data());
  }
}

  int
main(int argc, char ** argv)
{
  KAboutData about
    (
     "kab_client",
     I18N_NOOP("kab_client"),
     version,
     description,
     KAboutData::License_GPL,
     "(C) 2001 Rik Hemsley"
    );

  about.addAuthor("Rik Hemsley", 0, "rik@kde.org");

	KCmdLineArgs::init(argc, argv, &about, true);

  KCmdLineArgs::addCmdLineOptions(options);

  KCmdLineArgs * args = KCmdLineArgs::parsedArgs();

  KInstance i("kab_client");

  client = new DCOPClient;

  if (0 == client)
  {
    fprintf(stderr, "kab_client: internal error: can't create DCOP client.\n");
    return 1;
  }

  if (!client->attach())
  {
    fprintf(stderr, "kab_client: internal error: can't attach to DCOP.\n");
    return 1;
  }

  if (args->isSet("commands"))
  {
    printf(i18n("\nSyntax:\n").local8Bit());
    printf
      (
       i18n
       (
        "  kab_client create 'name' 'url' 'definition'\n"
        "    # Create a new addressbook.\n"
        "    # 'name' specifies name of book.\n"
        "    # 'url' specifies protocol and location\n"
        "    # 'definition' specifies filename containing format definition.\n\n"
       )
       .local8Bit()
      );
    printf
      (
       i18n
       (
        "  kab_client remove 'name'\n"
        "    # Remove an addressbook.\n"
        "    # 'name' specifies name of book to remove.\n\n"
       )
       .local8Bit()
      );
    printf
      (
       i18n
       (
        "  kab_client list\n"
        "    # List the available addressbooks.\n\n"
       )
       .local8Bit()
      );
  }
  else
  {
    if (args->count() == 0)
      KCmdLineArgs::usage();

    else
    {
      QString command(args->arg(0));

      if ("create" == command)
      {
        if (args->count() != 4)
          KCmdLineArgs::usage();
        else
          return (create(args->arg(1), args->arg(2), args->arg(3)) ? 0 : 1);
      }
      else if ("remove" == command)
      {
        if (args->count() != 2)
          KCmdLineArgs::usage();
        else
          return (remove(args->arg(1)) ? 0 : 1);
      }
      else if ("list" == command)
      {
        list();
      }
      else
      {
        KCmdLineArgs::usage();
      }
    }
  }

  return 0;
}


