/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

/* based on filter_sylpheed filter */

#include "filter_clawsmail.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <qdebug.h>
#include <QDomDocument>
#include <QDomElement>

using namespace MailImporter;

/** Default constructor. */
FilterClawsMail::FilterClawsMail() : FilterSylpheed()
{
    setName(i18n( "Import Claws-mail Maildirs and Folder Structure" ));
    setAuthor("Laurent Montel");
    setInfo(i18n("<p><b>Claws-mail import filter</b></p>"
                 "<p>Select the base directory of the Claws-mail mailfolder you want to import "
                 "(usually: ~/Mail ).</p>"
                 "<p>Since it is possible to recreate the folder structure, the folders "
                 "will be stored under: \"ClawsMail-Import\" in your local folder.</p>"
                 "<p>This filter also recreates the status of message, e.g. new or forwarded.</p>"));
}

/** Destructor. */
FilterClawsMail::~FilterClawsMail()
{
}

QString FilterClawsMail::defaultSettingsPath()
{
    return QDir::homePath() + QLatin1String( "/.claws-mail/" );
}

QString FilterClawsMail::localMailDirPath()
{
    QFile folderListFile( FilterClawsMail::defaultSettingsPath() + QLatin1String( "/folderlist.xml" ) );
    if ( folderListFile.exists() ) {
        QDomDocument doc;
        QString errorMsg;
        int errorRow;
        int errorCol;
        if ( !doc.setContent( &folderListFile, &errorMsg, &errorRow, &errorCol ) ) {
            qDebug() << "Unable to load document.Parse error in line " << errorRow
                     << ", col " << errorCol << ": " << errorMsg;
            return QString();
        }
        QDomElement settings = doc.documentElement();

        if ( settings.isNull() ) {
            return QString();
        }

        for ( QDomElement e = settings.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
            if ( e.tagName() == QLatin1String( "folder" ) ) {
                if ( e.hasAttribute( "type" ) ) {
                    if ( e.attribute( "type" ) == QLatin1String( "mh" ) ) {
                        return QDir::homePath() + QDir::separator() + e.attribute("path" );
                    }
                }
            }
        }
    }
    return QString();
}

bool FilterClawsMail::excludeFile(const QString &file)
{
    if(file.endsWith(QLatin1String(".claws_cache")) ||
            file.endsWith(QLatin1String(".claws_mark")) ||
            file.endsWith(QLatin1String(".mh_sequences")) ) {
        return true;
    }
    return false;
}

QString FilterClawsMail::defaultInstallFolder() const
{
    return i18nc("define folder name where we will import clawsmail mails", "ClawsMail-Import") + QLatin1Char('/');
}

QString FilterClawsMail::markFile() const
{
    return QString::fromLatin1(".claws_mark");
}




