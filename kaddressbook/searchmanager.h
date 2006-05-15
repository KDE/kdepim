/*
    This file is part of KAddressBook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include <qobject.h>

#include <kabc/stdaddressbook.h>
#include <libkdepim/distributionlist.h>

namespace KAB {

class SearchManager : public QObject
{
  Q_OBJECT

  public:
    enum Type {
      StartsWith,
      EndsWith,
      Contains,
      Equals
    };

    SearchManager( KABC::AddressBook *ab,
                   QObject *parent, const char *name = 0 );

    /**
      This method takes a pattern and searches for a match of the specified
      field of all available contacts. The result is propagated via
      contactsUpdated().

      @param pattern The search string.
      @param field The field which shall be compared with the search string.
      @param type The type for the matching.
     */
    void search( const QString &pattern, KABC::Field *field, Type type = Contains );


    void setJumpButtonFilter( const QStringList &patterns, KABC::Field *field );

    void reconfigure();

    /**
      Returns the contacts which matched the last search query.
     */
    KABC::Addressee::List contacts() const;

    /**
      Returns all the distribution lists.
     */
    KPIM::DistributionList::List distributionLists() const;

    /**
      Returns the name of all the distribution lists.
     */
    QStringList distributionListNames() const;

  signals:
    /**
      Emitted whenever the contacts have changed.
     */
    void contactsUpdated();

  public slots:
    void reload();

  private:
    void doSearch( const QString&, KABC::Field*, Type, const KABC::Addressee::List& );
    static void sortOutDistributionLists( KABC::Addressee::List& addrlist,
                                          KPIM::DistributionList::List& distrlists );

    KABC::Addressee::List mContacts;
    KPIM::DistributionList::List mDistributionLists;
    KABC::AddressBook *mAddressBook;

    QString mLastPattern;
    KABC::Field *mLastField;
    Type mLastType;

    QStringList mJumpButtonPatterns;
    KABC::Field *mJumpButtonField;

    bool mLimitContactDisplay;
};

}

#endif
