/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "filtericedove.h"
#include <KLocalizedString>
using namespace MailImporter;

FilterIcedove::FilterIcedove()
    : FilterThunderbird()
{
    setName(i18n("Import Icedove Mails and Folder Structure"));
    setAuthor(QStringLiteral("Laurent Montel"));
    setInfo(i18n("<p><b>Icedove import filter</b></p>"
                 "<p>Select your base Icedove mailfolder"
                 " (usually ~/.icedove/*.default/Mail/Local Folders/).</p>"
                 "<p><b>Note:</b> Never choose a Folder which <u>does not</u> contain mbox-files (for example,"
                 " a maildir): if you do, you will get many new folders.</p>"
                 "<p>Since it is possible to recreate the folder structure, the folders "
                 "will be stored under: \"Icedove-Import\".</p>"));

}

FilterIcedove::~FilterIcedove()
{

}

QString FilterIcedove::settingsPath()
{
    return FilterIcedove::defaultSettingsPath();
}

QString FilterIcedove::defaultSettingsPath()
{
    return QDir::homePath() + QLatin1String("/.icedove/");
}

QString FilterIcedove::defaultInstallFolder() const
{
    return QStringLiteral("Icedove-Import/");
}
