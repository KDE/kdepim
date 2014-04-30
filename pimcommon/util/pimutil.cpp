/*******************************************************************************
**
** Filename   : util
** Created on : 03 April, 2005
** Copyright  : (c) 2005 Till Adam
** Email      : <adam@kde.org>
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   It is distributed in the hope that it will be useful, but
**   WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**   General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
**   In addition, as a special exception, the copyright holders give
**   permission to link the code of this program with any edition of
**   the Qt library by Trolltech AS, Norway (or with modified versions
**   of Qt that use the same license as Qt), and distribute linked
**   combinations including the two.  You must obey the GNU General
**   Public License in all respects for all of the code used other than
**   Qt.  If you modify this file, you may extend this exception to
**   your version of the file, but you are not obligated to do so.  If
**   you do not wish to do so, delete this exception statement from
**   your version.
**
*******************************************************************************/

#include "pimutil.h"

#include "imapresourcesettings.h"

#include <KFileDialog>
#include <KMessageBox>
#include <KLocalizedString>

#include <QTextStream>
#include <QWidget>
#include <QPointer>

#include <errno.h>

OrgKdeAkonadiImapSettingsInterface *PimCommon::Util::createImapSettingsInterface( const QString &ident )
{
    return
            new OrgKdeAkonadiImapSettingsInterface(
                QLatin1String("org.freedesktop.Akonadi.Resource.") + ident, QLatin1String("/Settings"), QDBusConnection::sessionBus() );
}

void PimCommon::Util::saveTextAs( const QString &text, const QString &filter, QWidget *parent, const KUrl &url, const QString &caption )
{
    QPointer<KFileDialog> fdlg( new KFileDialog( url, filter, parent) );
    if (!caption.isEmpty())
        fdlg->setWindowTitle(caption);
    fdlg->setMode( KFile::File );
    fdlg->setOperationMode( KFileDialog::Saving );
    fdlg->setConfirmOverwrite(true);
    if ( fdlg->exec() == QDialog::Accepted && fdlg ) {
        const QString fileName = fdlg->selectedFile();
        if ( !saveToFile( fileName, text ) ) {
            KMessageBox::error( parent,
                                i18n( "Could not write the file %1:\n"
                                      "\"%2\" is the detailed error description.",
                                      fileName,
                                      QString::fromLocal8Bit( strerror( errno ) ) ),
                                i18n( "Save File Error" ) );
        }
    }
    delete fdlg;
}

bool PimCommon::Util::saveToFile( const QString &filename, const QString &text)
{
    QFile file( filename );
    if ( !file.open( QIODevice::WriteOnly|QIODevice::Text ) )
        return false;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << text;
    file.close();
    return true;
}

QString PimCommon::Util::loadToFile(const QString &filter, QWidget *parent, const KUrl &url, const QString &caption)
{
    QPointer<KFileDialog> fdlg( new KFileDialog( url, filter, parent) );
    if (!caption.isEmpty())
        fdlg->setWindowTitle(caption);
    fdlg->setMode( KFile::File );
    fdlg->setOperationMode( KFileDialog::Opening );
    QString result;
    if ( fdlg->exec() == QDialog::Accepted && fdlg ) {
        const QString fileName = fdlg->selectedFile();
        QFile file( fileName );
        if (!file.open( QIODevice::ReadOnly|QIODevice::Text ) ) {
            KMessageBox::error( parent,
                                i18n( "Could not read the file %1:\n"
                                      "\"%2\" is the detailed error description.",
                                      fileName,
                                      QString::fromLocal8Bit( strerror( errno ) ) ),
                                i18n( "Load File Error" ) );
        } else {
            result = QString::fromUtf8(file.readAll());
            file.close();
        }
    }
    delete fdlg;
    return result;
}

