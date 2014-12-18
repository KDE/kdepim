/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "pimcommon_debug.h"

#include "pimcommon/folderdialog/selectmulticollectiondialog.h"
#include <KMime/Message>
#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>

int main(int argc, char **argv)
{
    KAboutData aboutData(QStringLiteral("selectmulticollection_gui"), i18n("SelectMultiCollectiontest_Gui"), QStringLiteral("1.0"));
    aboutData.setShortDescription(i18n("Test for selectmulticollection dialog"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    PimCommon::SelectMultiCollectionDialog *dialog = new PimCommon::SelectMultiCollectionDialog(KMime::Message::mimeType());
    dialog->exec();
    delete dialog;
    return 0;
}

