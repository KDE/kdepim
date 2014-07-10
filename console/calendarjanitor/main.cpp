/*
  Copyright (c) 2013 Sérgio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "calendarjanitor.h"
#include "options.h"
#include "backuper.h"

#include "kdepim-version.h"

#include <k4aboutdata.h>
#include <KLocale>
#include <KCmdLineArgs>
#include <KApplication>

#include <QTextStream>
#include <QString>
#include <qglobal.h>

#ifdef Q_OS_UNIX
#    include <sys/types.h>
#    include <sys/stat.h>
#    include <fcntl.h>
#    include <unistd.h>
#endif

static const char progName[] = "calendarjanitor";
static const char progDisplay[] = "CalendarJanitor";

static const char progVersion[] = KDEPIM_VERSION;
static const char progDesc[] = "A command line interface to report and fix errors in your calendar data";

static void print(const QString &message)
{
    QTextStream out(stdout);
    out << message << "\n";
}

static void printCollectionsUsage()
{
    print(i18n("Error while parsing %1", QLatin1String("--collections")));
    print(i18n("Example usage %1", QLatin1String(": --collections 90,23,40")));
}

static void silenceStderr()
{
#ifdef Q_OS_UNIX
    if (qgetenv("KDE_CALENDARJANITOR_DEBUG") != "1") {
        // Disable stderr so we can actually read what's going on
        int fd = ::open("/dev/null", O_WRONLY);
        ::dup2(fd, 2);
        ::close(fd);
    }
#endif
}

int main(int argv, char *argc[])
{
    K4AboutData aboutData(progName, 0,                 // internal program name
                         ki18n(progDisplay),          // displayable program name.
                         progVersion,                 // version string
                         ki18n(progDesc),             // short program description
                         K4AboutData::License_GPL,     // license type
                         ki18n("(c) 2013, Sérgio Martins"),
                         ki18n(0),                    // any free form text
                         0,                           // program home page address
                         "bugs.kde.org");
    aboutData.addAuthor(ki18n("Sérgio Martins"), ki18n("Maintainer"), "iamsergio@gmail.com", 0);

    KCmdLineArgs::init(argv, argc, &aboutData, KCmdLineArgs::CmdLineArgNone);


    KCmdLineOptions options;
    options.add("collections <ids>", ki18n("List of collection ids to scan"));
    options.add("fix", ki18n("Fix broken incidences"));
    options.add("backup <output.ics>", ki18n("Backup your calendar"));
    options.add("strip-old-alarms", ki18n("Delete alarms older than 365 days"));

    options.add("", ki18n("\nExamples:\n\nScan all collections:\n"
                          "$ calendarjanitor\n\n"
                          "Scan and fix all collections:\n"
                          "$ calendarjanitor --fix\n\n"
                          "Scan and fix some collections:\n"
                          "$ calendarjanitor --collections 10,20 --fix\n\n"
                          "Backup all collections:\n"
                          "$ calendarjanitor --backup backup.ics\n\n"
                          "Backup some collections:\n"
                          "$ calendarjanitor --backup backup.ics --collections 10,20\n\n"
                          "Strip alarms from incidences older than 365 days:\n"
                          "$ calendarjanitor --strip-old-alarms --collections 10,20")
                );

    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    Options janitorOptions;

    if (args->isSet("collections")) {
        QString option = args->getOption("collections");
        QStringList collections = option.split(QLatin1String(","));
        QList<Akonadi::Collection::Id> ids;
        foreach (const QString &collection, collections) {
            bool ok = false;
            int num = collection.toInt(&ok);
            if (ok) {
                ids << num;
            } else {
                printCollectionsUsage();
                return -1;
            }

            if (ids.isEmpty()) {
                printCollectionsUsage();
                return -1;
            } else {
                janitorOptions.setCollections(ids);
            }
        }
    }

    if (args->isSet("fix") && args->isSet("backup")) {
        print(i18n("--fix is incompatible with --backup"));
        return -1;
    }

    if (args->isSet("strip-old-alarms") && args->isSet("backup")) {
        print(i18n("--strip-old-alarms is incompatible with --backup"));
        return -1;
    }

    if (args->isSet("strip-old-alarms") && args->isSet("fix")) {
        print(i18n("--strip-old-alarms is incompatible with --fix"));
        return -1;
    }

    KApplication app(false);

    silenceStderr(); // Switching off mobile phones, movie is about to start

    janitorOptions.setStripOldAlarms(args->isSet("strip-old-alarms"));

    QString backupFile;
    if (args->isSet("fix")) {
        janitorOptions.setAction(Options::ActionScanAndFix);
        print(i18n("Running in fix mode."));
    } else if (args->isSet("backup")) {
        backupFile = args->getOption("backup");
        if (backupFile.isEmpty()) {
            print(i18n("Please specify a output file."));
            return -1;
        }
        janitorOptions.setAction(Options::ActionBackup);
    } else {
        print(i18n("Running in scan only mode."));
        janitorOptions.setAction(Options::ActionScan);
    }

    switch(janitorOptions.action()) {
    case Options::ActionBackup: {
        Backuper *backuper = new Backuper();
        backuper->backup(backupFile, janitorOptions.collections());
        break;
    }
    case Options::ActionScan:
    case Options::ActionScanAndFix: {
        CalendarJanitor *janitor = new CalendarJanitor(janitorOptions);
        janitor->start();
        break;
    }
    default:
        Q_ASSERT(false);
    }


    return app.exec();
}
