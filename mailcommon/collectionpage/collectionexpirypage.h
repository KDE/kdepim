/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (c) 2009 Montel Laurent <montel@kde.org>
  Copyright (c) 2013 Jonathan Marten <jjm@keelhaul.me.uk>

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

#ifndef MAILCOMMON_COLLECTIONEXPIRYPAGE_H
#define MAILCOMMON_COLLECTIONEXPIRYPAGE_H

#include "mailcommon_export.h"

#include <Akonadi/Collection>
#include <Akonadi/CollectionPropertiesPage>

class KIntSpinBox;
class KJob;

class QCheckBox;
class QPushButton;
class QRadioButton;

namespace MailCommon {

class FolderCollection;
class FolderRequester;

class MAILCOMMON_EXPORT CollectionExpiryPage : public Akonadi::CollectionPropertiesPage
{
  Q_OBJECT

public:
    explicit CollectionExpiryPage( QWidget *parent = 0 );
    ~CollectionExpiryPage();

    bool canHandle( const Akonadi::Collection &col ) const;
    void load( const Akonadi::Collection &collection );
    void save( Akonadi::Collection &collection );

protected:
    void init();

protected slots:
    void slotUpdateControls();
    void slotCollectionModified(KJob* job);
    void slotChanged();
    void slotSaveAndExpire();

private:
    void saveAndExpire( Akonadi::Collection &collection, bool saveSettings, bool _expirenow );

private:
    QCheckBox *expireReadMailCB;
    KIntSpinBox *expireReadMailSB;
    QCheckBox *expireUnreadMailCB;
    KIntSpinBox *expireUnreadMailSB;
    QRadioButton *moveToRB;
    FolderRequester *folderSelector;
    QRadioButton *deletePermanentlyRB;
    QPushButton *expireNowPB;

    Akonadi::Collection mCollection;
    bool mChanged;
};

AKONADI_COLLECTION_PROPERTIES_PAGE_FACTORY( CollectionExpiryPageFactory, CollectionExpiryPage )

}

#endif
