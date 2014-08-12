/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteractionwithfolder.h"

#include "folderrequester.h"
#include "kernel/mailkernel.h"
#include "util/mailutil.h"
#include "dialog/filteractionmissingargumentdialog.h"

#include <QTextDocument>
#include <QPointer>

using namespace MailCommon;

FilterActionWithFolder::FilterActionWithFolder( const QString &name, const QString &label, QObject *parent )
    : FilterAction( name, label, parent )
{
}

bool FilterActionWithFolder::isEmpty() const
{
    return !mFolder.isValid();
}

QWidget* FilterActionWithFolder::createParamWidget( QWidget *parent ) const
{
    FolderRequester *requester = new FolderRequester( parent );
    requester->setShowOutbox( false );
    setParamWidgetValue( requester );

    connect( requester, SIGNAL(folderChanged(Akonadi::Collection)),
             this, SIGNAL(filterActionModified()) );

    return requester;
}

void FilterActionWithFolder::applyParamWidgetValue( QWidget *paramWidget )
{
    mFolder = static_cast<FolderRequester*>( paramWidget )->collection();
}

void FilterActionWithFolder::setParamWidgetValue( QWidget *paramWidget ) const
{
    static_cast<FolderRequester*>( paramWidget )->setCollection( mFolder );
}

void FilterActionWithFolder::clearParamWidget( QWidget *paramWidget ) const
{
    static_cast<FolderRequester*>( paramWidget )->setCollection( CommonKernel->draftsCollectionFolder() );
}

bool FilterActionWithFolder::argsFromStringInteractive( const QString &argsStr , const QString& name)
{
    bool needUpdate = false;
    argsFromString( argsStr );
    if ( !mFolder.isValid() ) {
        bool exactPath = false;
        Akonadi::Collection::List lst = FilterActionMissingCollectionDialog::potentialCorrectFolders( argsStr, exactPath );
        if ( lst.count() == 1 && exactPath )
            mFolder = lst.at( 0 );
        else {
            QPointer<FilterActionMissingCollectionDialog> dlg = new FilterActionMissingCollectionDialog( lst, name, argsStr );
            if ( dlg->exec() ) {
                mFolder = dlg->selectedCollection();
                needUpdate = true;
            }
            delete dlg;
        }
    }
    return needUpdate;
}

QString FilterActionWithFolder::argsAsStringReal() const
{
    if ( KernelIf->collectionModel() )
        return MailCommon::Util::fullCollectionPath( mFolder );
    return FilterActionWithFolder::argsAsString();
}

void FilterActionWithFolder::argsFromString( const QString &argsStr )
{
    bool ok = false;
    const Akonadi::Collection::Id id = argsStr.toLongLong( &ok );
    if ( ok ) {
        mFolder = Akonadi::Collection( id );
    } else {
        mFolder = Akonadi::Collection();
    }
}

QString FilterActionWithFolder::argsAsString() const
{
    QString result;
    if ( mFolder.isValid() )
        result = QString::number( mFolder.id() );

    return result;
}

QString FilterActionWithFolder::displayString() const
{
    QString result;
    if ( mFolder.isValid() )
        result = MailCommon::Util::fullCollectionPath(MailCommon::Util::updatedCollection( mFolder ));

    return label() + QLatin1String( " \"" ) + result.toHtmlEscaped() + QLatin1String( "\"" );
}

bool FilterActionWithFolder::folderRemoved( const Akonadi::Collection &oldFolder, const Akonadi::Collection &newFolder )
{
    if ( oldFolder == mFolder ) {
        mFolder = newFolder;
        return true;
    } else
        return false;
}


