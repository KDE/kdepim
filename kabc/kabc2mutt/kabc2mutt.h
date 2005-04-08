/*
    KAbc2Mutt

    Copyright (c) 2003 - 2004 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef KABC2MUTT_H
#define KABC2MUTT_H

#include <kabc/stdaddressbook.h>

class KABC2Mutt : public QObject
{
  Q_OBJECT

  public:

    enum Format { Aliases, Query };

    KABC2Mutt( QObject *parent, const char *name = 0 );

    void setQuery( const QString &query ) { mQuery = query; }
    void setFormat( Format format ) { mFormat = format; }
    void setIgnoreCase( bool ignoreCase ) { mIgnoreCase = ignoreCase; }
    void setAllAddresses( bool allAddresses ) { mAllAddresses = allAddresses; }
    void setAlternateKeyFormat( bool alternateKeyFormat ) { mAlternateKeyFormat = alternateKeyFormat; }

    void run();

  private slots:
    void loadingFinished();

  private:
    QString mQuery;
    Format mFormat;
    bool mIgnoreCase;
    bool mAllAddresses;
    bool mAlternateKeyFormat;

    KABC::AddressBook *mAddressBook;
};

#endif
