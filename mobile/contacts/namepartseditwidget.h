/*
    This file is part of Akonadi Contact.

    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#ifndef NAMEPARTSEDITWIDGET_H
#define NAMEPARTSEDITWIDGET_H

#include <kcontacts/addressee.h>

#include <QWidget>

class KLineEdit;
class KComboBox;

class NamePartsEditWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit NamePartsEditWidget( QWidget *parent = 0 );

    void loadContact( const KContacts::Addressee &contact );
    void storeContact( KContacts::Addressee &contact ) const;

  Q_SIGNALS:
    void nameChanged( const KContacts::Addressee &contact );

  private Q_SLOTS:
    void inputChanged();

  private:
    KComboBox *mSuffixCombo;
    KComboBox *mPrefixCombo;
    KLineEdit *mFamilyNameEdit;
    KLineEdit *mGivenNameEdit;
    KLineEdit *mAdditionalNameEdit;
    KContacts::Addressee mContact;
};

#endif
