/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (c) 2009 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef MAILCOMMON_COLLECTIONGENERALPAGE_H
#define MAILCOMMON_COLLECTIONGENERALPAGE_H

#include "mailcommon_export.h"

#include <akonadi/collectionpropertiespage.h>

class KComboBox;
class KLineEdit;
class QCheckBox;

template <typename T> class QSharedPointer;

namespace KPIMIdentities {
  class IdentityCombo;
}

namespace MailCommon {

class FolderCollection;

class MAILCOMMON_EXPORT CollectionGeneralPage : public Akonadi::CollectionPropertiesPage
{
  Q_OBJECT

  public:
    explicit CollectionGeneralPage( QWidget *parent = 0 );
    ~CollectionGeneralPage();

    void load( const Akonadi::Collection &collection );
    void save( Akonadi::Collection &collection );

    enum FolderContentsType
    {
      ContentsTypeMail = 0,
      ContentsTypeCalendar,
      ContentsTypeContact,
      ContentsTypeNote,
      ContentsTypeTask,
      ContentsTypeJournal,
      ContentsTypeLast = ContentsTypeJournal
    };

    enum IncidencesFor
    {
      IncForNobody,
      IncForAdmins,
      IncForReaders
    };

  protected:
    void init( const Akonadi::Collection& );

  private Q_SLOTS:
    void slotIdentityCheckboxChanged();
    void slotFolderContentsSelectionChanged( int );
    void slotNameChanged( const QString& name );

  private:
    KComboBox *mContentsComboBox;
    KComboBox *mIncidencesForComboBox;
    QCheckBox *mSharedSeenFlagsCheckBox;
  #if 0
    QCheckBox   *mNewMailCheckBox;
  #endif
    QCheckBox   *mNotifyOnNewMailCheckBox;
    QCheckBox   *mKeepRepliesInSameFolderCheckBox;
    QCheckBox   *mHideInSelectionDialogCheckBox;
    QCheckBox   *mUseDefaultIdentityCheckBox;
    KLineEdit   *mNameEdit;
    KPIMIdentities::IdentityCombo *mIdentityComboBox;
    QSharedPointer<MailCommon::FolderCollection> mFolderCollection;
    bool mIsLocalSystemFolder;
    bool mIsResourceFolder;
};

AKONADI_COLLECTION_PROPERTIES_PAGE_FACTORY( CollectionGeneralPageFactory, CollectionGeneralPage )

}

#endif
