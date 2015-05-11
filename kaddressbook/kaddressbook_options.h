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

#ifndef KADDRESSBOOK_OPTIONS_H
#define KADDRESSBOOK_OPTIONS_H

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <KLocalizedString>

static void kaddressbook_options(QCommandLineParser *parser)
{
    parser->addOption(QCommandLineOption(QLatin1String("import"),
                                         i18n("Import the given file")));
    parser->addPositionalArgument(QLatin1String("urls"),
                                  i18n("Files or URLs. THe user will be asked whether to import."),
                                  QLatin1String("[urls...]"));
    parser->addHelpOption();
    parser->addVersionOption();
}

#endif // KADDRESSBOOK_OPTIONS_H
