#include <stdio.h>

#include <qstring.h>
#include <qcstring.h>
#include <qfile.h>

#include <dcopclient.h>
#include <kapp.h>
#include <kinstance.h>
#include <kcmdlineargs.h>
#include <kdatastream.h>
#include <klocale.h>
#include <kaboutdata.h>

#include <kab2/FormatDefinition.h>
#include <kab2/KAddressBookServerInterface_stub.h>

static const char * description = I18N_NOOP("KDE addressbook creation tool");
static const char * version = "v0.1";
static DCOPClient * client = 0;

static KCmdLineOptions options[] =
{
  { "commands", I18N_NOOP("Show available commands."), 0 },
  { "+command", I18N_NOOP("Command (see --commands)."), 0 },
  { 0, 0, 0 }
};

  static bool
create(QString name, QString url, QString fmt)
{
  KAddressBookServerInterface_stub server
    ("KAddressBookServer", "KAddressBookServer");

  bool created = server.create(name, url, fmt);

  if (!created)
  {
    qDebug("Server would not create new database.");
    return false;
  }

  return true;
}

  static bool
add_format_definition(QString fmt)
{
  QFile f(fmt);

  if (!f.open(IO_ReadOnly))
  {
    qDebug("Format definition file cannot be opened.");
    return false;
  }

  KAB::FormatDefinition fd(f.readAll());

  if (!fd)
  {
    qDebug("Format definition cannot be parsed.");
    return false;
  }

  KAddressBookServerInterface_stub server
    ("KAddressBookServer", "KAddressBookServer");

  if (!!server.formatDefinition(fd.name()))
  {
    qDebug
      (
       "Server already has format definition called `%s'",
       fmt.utf8().data()
      );

    return false;
  }

  if (!server.addFormatDefinition(fd))
  {
    qDebug("Server would not accept new format definition.");
    return false;
  }

  return true;
}

  static bool
update_format_definition(QString fmt)
{
  QFile f(fmt);

  if (!f.open(IO_ReadOnly))
  {
    qDebug("Format definition file cannot be opened.");
    return false;
  }

  KAB::FormatDefinition fd(f.readAll());

  if (!fd)
  {
    qDebug("Format definition cannot be parsed.");
    return false;
  }

  KAddressBookServerInterface_stub server
    ("KAddressBookServer", "KAddressBookServer");

  if (!server.formatDefinition(fd.name()))
  {
    qDebug
      (
       "Server does not have format definition called `%s'",
       fmt.utf8().data()
      );

    return false;
  }

  if (!server.updateFormatDefinition(fd))
  {
    qDebug("Server would not update format definition.");
    return false;
  }

  return true;
}

  static bool
remove(QString name)
{
  KAddressBookServerInterface_stub server
    ("KAddressBookServer", "KAddressBookServer");

  bool removed = server.remove(name);

  if (!removed)
  {
    qDebug("Server refused to remove addressbook `%s'", name.utf8().data());
    return false;
  }

  return true;
}

  static void
list()
{
  KAddressBookServerInterface_stub server
    ("KAddressBookServer", "KAddressBookServer");

  QStringList abList(server.list());

  for (QStringList::ConstIterator it(abList.begin()); it != abList.end(); ++it)
  {
    qDebug("%s", (*it).utf8().data());
  }
}

  static void
list_format_definitions()
{
  KAddressBookServerInterface_stub server
    ("KAddressBookServer", "KAddressBookServer");

  QStringList fdl(server.formatDefinitionList());

  for (QStringList::ConstIterator it(fdl.begin()); it != fdl.end(); ++it)
  {
    qDebug("%s", (*it).utf8().data());
  }
}

  static bool
default_format_definition()
{
  KAddressBookServerInterface_stub server
    ("KAddressBookServer", "KAddressBookServer");

  KAB::FormatDefinition fd(server.defaultFormatDefinition());

  if (!fd)
  {
    qDebug("Default format definition does not exist.");
    return false;
  }

  QString xml(fd.toXML());

  qDebug("%s", xml.utf8().data());

  return true;
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
        "    # 'definition' specifies name of format definition.\n\n"
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
    printf
      (
       i18n
       (
        "  kab_client list_format_definitions\n"
        "    # List the available format definitions.\n\n"
       )
       .local8Bit()
      );
    printf
      (
       i18n
       (
        "  kab_client add_format_definition 'filename'\n"
        "    # Add format definition.\n"
        "    # 'filename' specified file containing XML format definition.\n\n"
       )
       .local8Bit()
      );
    printf
      (
       i18n
       (
        "  kab_client update_format_definition 'filename'\n"
        "    # Update format definition.\n"
        "    # 'filename' specified file containing XML format definition.\n\n"
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
          return (remove(QString(args->arg(1))) ? 0 : 1);
      }
      else if ("list" == command)
      {
        list();
      }
      else if ("list_format_definitions" == command)
      {
        list_format_definitions();
      }
      else if ("add_format_definition" == command)
      {
        if (args->count() != 2)
          KCmdLineArgs::usage();
        else
          return (add_format_definition(args->arg(1)) ? 0 : 1);
      }
      else if ("update_format_definition" == command)
      {
        if (args->count() != 2)
          KCmdLineArgs::usage();
        else
          return (update_format_definition(args->arg(1)) ? 0 : 1);
      }
      else if ("default_format_definition" == command)
      {
        return (default_format_definition() ? 0 : 1);
      }
      else
      {
        KCmdLineArgs::usage();
      }
    }
  }

  return 0;
}


