/*
  Copyright (c) 2009 Montel Laurent <montel@kde.org>

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

#ifndef MAILCOMMON_FOLDERCOLLECTION_H
#define MAILCOMMON_FOLDERCOLLECTION_H

#include "mailcommon_export.h"

#include <messagecore/misc/mailinglist.h>
using MessageCore::MailingList;

#include <Collection>
#include <CollectionStatistics>

#include <KSharedConfig>
#include <KShortcut>
#include <KIO/Job>

namespace MailCommon {

class MAILCOMMON_EXPORT FolderCollection : public QObject
{
    Q_OBJECT

public:
    static QSharedPointer<FolderCollection> forCollection(
            const Akonadi::Collection &coll, bool writeConfig = true );

    ~FolderCollection();

    Akonadi::Collection collection() const;
    void setCollection( const Akonadi::Collection &collection );

    static QString configGroupName( const Akonadi::Collection &col );
    static void clearCache();

    bool isWriteConfig() const;
    void setWriteConfig( bool writeConfig );

    void writeConfig() const;
    void readConfig();

    QString name() const;

    bool isReadOnly() const;

    bool isStructural() const;

    bool isSystemFolder() const;

    qint64 count() const;

    bool canDeleteMessages() const;

    bool canCreateMessages() const;

    bool isValid() const;

    Akonadi::Collection::Rights rights() const;

    Akonadi::CollectionStatistics statistics() const;

    void setShortcut( const KShortcut & );
    const KShortcut &shortcut() const
    {
        return mShortcut;
    }

    /**
     *  Get / set whether the default identity should be used instead of the
     *  identity specified by setIdentity().
     */
    void setUseDefaultIdentity( bool useDefaultIdentity );
    bool useDefaultIdentity() const
    {
        return mUseDefaultIdentity;
    }

    void setIdentity( uint identity );
    uint identity() const;

    /**
     * Returns true if this folder is associated with a mailing-list.
     */
    void setMailingListEnabled( bool enabled );
    bool isMailingListEnabled() const
    {
        return mMailingListEnabled;
    }

    void setMailingList( const MailingList &mlist );

    MailingList mailingList() const
    {
        return mMailingList;
    }

    /**
     * Returns true if the replies to mails from this folder should be
     * put in the same folder.
     */
    bool putRepliesInSameFolder() const
    {
        return mPutRepliesInSameFolder;
    }
    void setPutRepliesInSameFolder( bool b )
    {
        mPutRepliesInSameFolder = b;
    }

    /**
     * Returns true if this folder should be hidden from all folder selection dialogs
     */
    bool hideInSelectionDialog() const
    {
        return mHideInSelectionDialog;
    }
    void setHideInSelectionDialog( bool hide )
    {
        mHideInSelectionDialog = hide;
    }

    QString mailingListPostAddress() const;

protected slots:
    void slotIdentitiesChanged();

private:
    explicit FolderCollection( const Akonadi::Collection &col, bool writeconfig );

    Akonadi::Collection mCollection;

    /** Mailing list attributes */
    bool                mMailingListEnabled;
    MailingList         mMailingList;

    bool mUseDefaultIdentity;
    uint mIdentity;

    /** Should replies to messages in this folder be put in here? */
    bool mPutRepliesInSameFolder;

    /** Should this folder be hidden in the folder selection dialog? */
    bool mHideInSelectionDialog;

    /** shortcut associated with this folder or null, if none is configured. */
    KShortcut mShortcut;
    bool mWriteConfig;
};

}

#endif
