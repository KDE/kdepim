/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "noteutils.h"
#include "notesharedglobalconfig.h"
#include <KProcess>
#include <KMessageBox>
#include <KLocale>

NOTESHARED_EXPORT bool NoteShared::NoteUtils::sendToMail(QWidget *parent, const QString &title, const QString &message)
{
    // get the mail action command
    const QStringList cmd_list = NoteShared::NoteSharedGlobalConfig::mailAction().split( QLatin1Char(' '), QString::SkipEmptyParts );
    if (cmd_list.isEmpty()) {
        KMessageBox::sorry( parent, i18n( "Please configure send mail action." ) );
        return false;
    }
    KProcess mail;
    Q_FOREACH ( const QString &cmd, cmd_list ) {
        if ( cmd == QLatin1String("%f") ) {
            mail << message;
        } else if ( cmd == QLatin1String("%t") ) {
            mail << title;
        } else {
            mail << cmd;
        }
    }

    if ( !mail.startDetached() ) {
        KMessageBox::sorry( parent, i18n( "Unable to start the mail process." ) );
        return false;
    }
    return true;
}
