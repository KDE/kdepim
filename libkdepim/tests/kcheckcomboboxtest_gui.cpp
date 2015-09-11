/*
    This file is part of libkdepim.

    Copyright (c) 2010 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <KAboutData>

#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStandardPaths>

#include "../src/widgets/kcheckcombobox.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("testcheckcombo"), i18n("Test KCheckComboBox"), QStringLiteral("0.1"));
    QCommandLineParser parser;
    QStandardPaths::setTestModeEnabled(true);
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KPIM::KCheckComboBox *combo = new KPIM::KCheckComboBox(Q_NULLPTR);
    combo->addItems(QStringList() << QStringLiteral("KDE") << QStringLiteral("Mac OSX") << QStringLiteral("Windows") << QStringLiteral("XFCE") << QStringLiteral("FVWM") << QStringLiteral("TWM"));
    combo->setCheckedItems(QStringList() << QStringLiteral("KDE") << QStringLiteral("Mac OSX") << QStringLiteral("Windows"));
    combo->resize(400, 20);
    combo->setSqueezeText(true);
    combo->show();

    return app.exec();

}

