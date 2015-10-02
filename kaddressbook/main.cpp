/*
  This file is part of KAddressBook.

  Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

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
*/

#include "aboutdata.h"
#include "mainwindow.h"
#include "mainwidget.h"
#include "kaddressbook_options.h"
#include "kaddressbookmigrateapplication.h"

#include <QCommandLineParser>

#include "kaddressbook_debug.h"
#include <kontactinterface/pimuniqueapplication.h>

//-----------------------------------------------------------------------------

class KAddressBookApplication : public KontactInterface::PimUniqueApplication
{
public:
    KAddressBookApplication(int &argc, char **argv[])
        : KontactInterface::PimUniqueApplication(argc, argv),
          mMainWindow(Q_NULLPTR)
    {
    }
    int activate(const QStringList &arguments, const QString&) Q_DECL_OVERRIDE;

private:
    MainWindow *mMainWindow;
};

int KAddressBookApplication::activate(const QStringList &arguments, const QString&)
{
    if (!mMainWindow) {
        mMainWindow = new MainWindow;
        mMainWindow->show();
    }
    mMainWindow->mainWidget()->handleCommandLine(arguments);
    return 0;
}

int main(int argc, char **argv)
{
    KAddressBookApplication app(argc, &argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    KLocalizedString::setApplicationDomain("kaddressbook");

    AboutData about;
    app.setAboutData(about);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("kcontact")));
    QCommandLineParser *cmdArgs = app.cmdArgs();
    kaddressbook_options(cmdArgs);

    const QStringList args = QCoreApplication::arguments();
    cmdArgs->process(args);
    about.processCommandLine(cmdArgs);

    if (!KAddressBookApplication::start(args)) {
        qCWarning(KADDRESSBOOK_LOG) << "kaddressbook is already running, exiting.";
        return 0;
    }
    KAddressBookMigrateApplication migrate;
    migrate.migrate();

    return app.exec();
}
