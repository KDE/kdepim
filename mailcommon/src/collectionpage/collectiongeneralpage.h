/*
  Copyright (c) 2009-2015 Montel Laurent <montel@kde.org>

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

#ifndef MAILCOMMON_COLLECTIONGENERALPAGE_H
#define MAILCOMMON_COLLECTIONGENERALPAGE_H

#include "mailcommon_export.h"

#include <CollectionPropertiesPage>

class KLineEdit;

class QCheckBox;

template <typename T> class QSharedPointer;

namespace KIdentityManagement
{
class IdentityCombo;
}
class QVBoxLayout;
namespace MailCommon
{

class FolderCollection;
class IncidencesForWidget;
class ContentTypeWidget;
class MAILCOMMON_EXPORT CollectionGeneralPage : public Akonadi::CollectionPropertiesPage
{
    Q_OBJECT

public:
    explicit CollectionGeneralPage(QWidget *parent = Q_NULLPTR);
    ~CollectionGeneralPage();

    void load(const Akonadi::Collection &collection) Q_DECL_OVERRIDE;
    void save(Akonadi::Collection &collection) Q_DECL_OVERRIDE;

protected:
    void init(const Akonadi::Collection &);

private Q_SLOTS:
    void slotIdentityCheckboxChanged();
    void slotFolderContentsSelectionChanged(int);
    void slotNameChanged(const QString &name);

private:
    void addLine(QWidget *parent, QVBoxLayout *layout);
    QString mColorName;
    ContentTypeWidget *mContentsComboBox;
    IncidencesForWidget *mIncidencesForComboBox;
    QCheckBox *mSharedSeenFlagsCheckBox;
    QCheckBox   *mNotifyOnNewMailCheckBox;
    QCheckBox   *mKeepRepliesInSameFolderCheckBox;
    QCheckBox   *mHideInSelectionDialogCheckBox;
    QCheckBox   *mUseDefaultIdentityCheckBox;
    KLineEdit   *mNameEdit;
    KIdentityManagement::IdentityCombo *mIdentityComboBox;
    QSharedPointer<MailCommon::FolderCollection> mFolderCollection;
    bool mIsLocalSystemFolder;
    bool mIsResourceFolder;
};

AKONADI_COLLECTION_PROPERTIES_PAGE_FACTORY(CollectionGeneralPageFactory, CollectionGeneralPage)

}

#endif
