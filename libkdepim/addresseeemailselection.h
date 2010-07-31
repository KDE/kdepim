/*
    This file is part of libkdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KPIM_ADDRESSEE_EMAILSELECTION_H
#define KPIM_ADDRESSEE_EMAILSELECTION_H

#include <addresseeselector.h>

namespace KPIM {

class KDE_EXPORT AddresseeEmailSelection : public Selection
{
  public:
    AddresseeEmailSelection();

    /**
      Returns the number of fields the selection offers.
     */
    virtual uint fieldCount() const;

    /**
      Returns the title for the field specified by index.
     */
    virtual TQString fieldTitle( uint index ) const;

    /**
      Returns the number of items for the given addressee.
     */
    virtual uint itemCount( const KABC::Addressee &addresse ) const;

    /**
      Returns the text that's used for the item specified by index.
     */
    virtual TQString itemText( const KABC::Addressee &addresse, uint index ) const;

    /**
      Returns the icon that's used for the item specified by index.
     */
    virtual TQPixmap itemIcon( const KABC::Addressee &addresse, uint index ) const;

    /**
      Returns whether the item specified by index is enabled.
     */
    virtual bool itemEnabled( const KABC::Addressee &addresse, uint index ) const;

    /**
      Returns whether the item specified by index matches the passed pattern.
     */
    virtual bool itemMatches( const KABC::Addressee &addresse, uint index, const TQString &pattern ) const;

    /**
      Returns whether the item specified by index equals the passed pattern.
     */
    virtual bool itemEquals( const KABC::Addressee &addresse, uint index, const TQString &pattern ) const;

    /**
      Returns the text that's used for the given distribution list.
     */
    virtual TQString distributionListText( const KABC::DistributionList *distributionList ) const;

    /**
      Returns the icon that's used for the given distribution list.
     */
    virtual TQPixmap distributionListIcon( const KABC::DistributionList *distributionList ) const;

    /**
      Returns whether the given distribution list is enabled.
     */
    virtual bool distributionListEnabled( const KABC::DistributionList *distributionList ) const;

    /**
      Returns whether the given distribution list matches the passed pattern.
     */
    virtual bool distributionListMatches(  const KABC::DistributionList *distributionList,
                                           const TQString &pattern ) const;

    /**
      Returns the number of additional address books.
     */
    virtual uint addressBookCount() const;

    /**
      Returns the title for an additional address book.
     */
    virtual TQString addressBookTitle( uint index ) const;

    /**
      Returns the content for an additional address book.
     */
    virtual KABC::Addressee::List addressBookContent( uint index ) const;

    TQStringList to() const;
    TQStringList cc() const;
    TQStringList bcc() const;

    KABC::Addressee::List toAddresses() const;
    KABC::Addressee::List ccAddresses() const;
    KABC::Addressee::List bccAddresses() const;

    TQStringList toDistributionLists() const;
    TQStringList ccDistributionLists() const;
    TQStringList bccDistributionLists() const;

    void setSelectedTo( const TQStringList &emails );
    void setSelectedCC( const TQStringList &emails );
    void setSelectedBCC( const TQStringList &emails );

  private:
    virtual void addSelectedAddressees( uint fieldIndex, const KABC::Addressee&, uint itemIndex );
    virtual void addSelectedDistributionList( uint fieldIndex, const KABC::DistributionList* );

    TQString email( const KABC::Addressee&, uint ) const;
    void setSelectedItem( uint fieldIndex, const TQStringList& );

    KABC::Addressee::List mToAddresseeList;
    KABC::Addressee::List mCcAddresseeList;
    KABC::Addressee::List mBccAddresseeList;

    TQStringList mToEmailList;
    TQStringList mCcEmailList;
    TQStringList mBccEmailList;

    TQStringList mToDistributionList;
    TQStringList mCcDistributionList;
    TQStringList mBccDistributionList;
};

}

#endif
