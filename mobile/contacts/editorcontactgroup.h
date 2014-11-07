/*
    Copyright (c) 2010 Kevin Krammer <kevin.krammer@gmx.at>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef EDITORCONTACTGROUP_H
#define EDITORCONTACTGROUP_H

#include <QWidget>

namespace Akonadi
{
  class Collection;
}

namespace KContacts
{
  class ContactGroup;
}

class KJob;

class EditorContactGroup : public QWidget
{
  Q_OBJECT

  public:
    explicit EditorContactGroup( QWidget *parent = 0 );

    ~EditorContactGroup();

    void setDefaultCollection( const Akonadi::Collection &collection );

    void loadContactGroup( const KContacts::ContactGroup &contactGroup );

    void saveContactGroup( KContacts::ContactGroup &contactGroup ) const;

    Akonadi::Collection selectedCollection() const;

  Q_SIGNALS:
    void collectionChanged( const Akonadi::Collection &collection );

    void cancelClicked();
    void saveClicked();
    void requestLaunchAccountWizard();

  private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void nameTextChanged( const QString& ) )
    Q_PRIVATE_SLOT( d, void addRecipientClicked() )
    Q_PRIVATE_SLOT( d, void fetchResult( KJob* ) )
    Q_PRIVATE_SLOT( d, void clearRecipientClicked() )
    Q_PRIVATE_SLOT( d, void availableCollectionsChanged() )
    Q_PRIVATE_SLOT( d, void disableSaveButton() )
};

#endif
