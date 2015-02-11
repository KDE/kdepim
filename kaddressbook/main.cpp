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

#include <KCmdLineArgs>
#include "kaddressbook_debug.h"
#include <kontactinterface/pimuniqueapplication.h>

//-----------------------------------------------------------------------------

class KAddressBookApplication : public KontactInterface::PimUniqueApplication
{
public:
    KAddressBookApplication()
        : KontactInterface::PimUniqueApplication(),
          mMainWindow(Q_NULLPTR)
    {
    }
    virtual int newInstance() Q_DECL_OVERRIDE;

private:
    MainWindow *mMainWindow;
};

int KAddressBookApplication::newInstance()
{
    qCDebug(KADDRESSBOOK_LOG);
    if (!mMainWindow) {
        mMainWindow = new MainWindow;
        mMainWindow->show();
    }
    mMainWindow->mainWidget()->handleCommandLine();
    return 0;
}

int main(int argc, char **argv)
{
    KLocalizedString::setApplicationDomain("kaddressbook");
    AboutData about;

    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineArgs::addCmdLineOptions(kaddressbook_options());   // Add KAddressBook options
    KUniqueApplication::addCmdLineOptions();
    if (!KAddressBookApplication::start()) {
        return 0;
    }

    KAddressBookApplication app;

    return app.exec();
}
