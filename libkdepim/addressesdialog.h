/*  -*- mode: C++; c-file-style: "gnu" -*-
 *
 *  This file is part of libkdepim.
 *
 *  Copyright (c) 2003 Zack Rusin <zack@kde.org>
 *  Copyright (c) 2003 Aaron J. Seigo <aseigo@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef KDEPIM_ADDRESSESDIALOG_H
#define KDEPIM_ADDRESSESDIALOG_H

#include "kdepim_export.h"

#include <kabc/addressee.h>
#include <kabc/contactgroup.h>

#include <KDialog>

#include <QObject>
#include <QList>
#include <QSet>
#include <QStringList>

class QModelIndex;
class QStandardItemModel;
class QStandardItem;
class KJob;

namespace Akonadi {
  class Item;
  class Session;
  class ChangeRecorder;
  class EntityTreeView;
}

namespace KPIM {

class LdapSearchDialog;
class ProxyModel;

class KDEPIM_EXPORT AddressesDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
     * Creates a new addresses dialog.
     *
     * @param widget The parent widget.
     * @param session The session to use for Akonadi.
     */
    AddressesDialog( QWidget *widget = 0, Akonadi::Session *session = 0 );

    /**
     * Destroys the addresses dialog.
     */
    virtual ~AddressesDialog();

    /**
     * Describes the email address receiver types.
     */
    enum ReceiverType
    {
      ToReceiver, ///< Direct receivers
      CcReceiver, ///< Carbon copy receivers
      BccReceiver ///< Blind carbon copy receivers
    };

    /**
     * Returns the selected email addresses for the given receiver @p type.
     */
    QStringList emailAddresses( ReceiverType type ) const;

    /**
     * Returns the selected contacts for the given receiver @p type.
     */
    KABC::Addressee::List contacts( ReceiverType type ) const;

    /**
     * Returns the list of picked "To" addresses as KABC::Addressee::List.
     * Note that this does include the distribution lists
     * Multiple Addressees are removed
     */
    KABC::Addressee::List allToAddressesNoDuplicates()  const;

    /**
     * Returns the selected contact groups for the given receiver @p type.
     */
    QStringList distributionLists( ReceiverType type ) const;

  public Q_SLOTS:
    /**
     * Displays the CC field if @p b is true, else
     * hides it. By default displays it.
     */
    void setShowCC( bool b );

    /**
     * Displays the BCC field if @p b is true, else
     * hides it. By default displays it.
     */
    void setShowBCC( bool b );

    void setRecentAddresses( const KABC::Addressee::List&) {}

    /**
     * Adds addresses in @p l to the selected "To" group.
     */
    void setSelectedTo( const QStringList& l );

    /**
     * Adds addresses in @p l to the selected "CC" group.
     */
    void setSelectedCC( const QStringList& l );

    /**
     * Adds addresses in @p l to the selected "BCC" group.
     */
    void setSelectedBCC( const QStringList& l );

  private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void availableSelectionChanged() )
    Q_PRIVATE_SLOT( d, void selectedSelectionChanged() )
    Q_PRIVATE_SLOT( d, void selectedCountChanged() )
    Q_PRIVATE_SLOT( d, void addSelectedTo() )
    Q_PRIVATE_SLOT( d, void addSelectedCC() )
    Q_PRIVATE_SLOT( d, void addSelectedBCC() )
    Q_PRIVATE_SLOT( d, void removeEntry() )
    Q_PRIVATE_SLOT( d, void searchLdap() )
    Q_PRIVATE_SLOT( d, void ldapSearchResult() )
    Q_PRIVATE_SLOT( d, void saveAsDistributionList() )
    Q_PRIVATE_SLOT( d, void saveAsDistributionListDone( KJob* ) )
};

}

#endif
