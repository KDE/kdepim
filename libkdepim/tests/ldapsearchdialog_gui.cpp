/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include <KAboutData>

#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStandardPaths>

#include "../src/ldap/ldapsearchdialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("testldapsearchdialog"), i18n("Test LDapSearchDialog"), QStringLiteral("0.1"));
    QCommandLineParser parser;
    //We can't use it otherwise we need to call kbuilsyscoca in test mode too.
    //QStandardPaths::setTestModeEnabled(true);
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KLDAP::LdapSearchDialog dlg;
    dlg.exec();
    return app.exec();

}

