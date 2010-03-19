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
    AddressesDialog( QWidget *widget = 0, Akonadi::Session *session = 0 );
    virtual ~AddressesDialog();

    /**
     * Returns the list of picked "To" addresses as a QStringList.
     */
    QStringList to() const;

    /**
     * Returns the list of picked "CC" addresses as a QStringList.
     */
    QStringList cc() const;

    /**
     * Returns the list of picked "BCC" addresses as a QStringList.
     */
    QStringList bcc() const;

    /**
     * Returns the list of picked "To" addresses as KABC::Addressee::List.
     * Note that this doesn't include the distribution lists
     */
    KABC::Addressee::List toAddresses()  const;

    /**
     * Returns the list of picked "To" addresses as KABC::Addressee::List.
     * Note that this does include the distribution lists
     * Multiple Addressees are removed
     */
    KABC::Addressee::List allToAddressesNoDuplicates()  const;

    /**
     * Returns the list of picked "CC" addresses as KABC::Addressee::List.
     * Note that this doesn't include the distribution lists
     */
    KABC::Addressee::List ccAddresses()  const;

    /**
     * Returns the list of picked "BCC" addresses as KABC::Addressee::List.
     * Note that this doesn't include the distribution lists
     */
    KABC::Addressee::List bccAddresses() const;

    /**
     * Returns the list of picked "To" distribution lists.
     * This complements @ref toAddresses.
     */
    QStringList toDistributionLists() const;

    /**
     * Returns the list of picked "CC" distribution lists.
     * This complements @ref ccAddresses.
     */
    QStringList ccDistributionLists() const;

    /**
     * Returns the list of picked "BCC" distribution lists.
     * This complements @ref bccAddresses.
     */
    QStringList bccDistributionLists() const;

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

  private Q_SLOTS:
    void availableSelectionChanged();
    void selectedSelectionChanged();
    void selectedCountChanged();
    void addSelectedTo();
    void addSelectedCC();
    void addSelectedBCC();
    void removeEntry();
    void searchLdap();
    void ldapSearchResult();
    void saveAsDistributionList();
    void saveAsDistributionListDone( KJob * );

  private:
    Akonadi::Session *m_session;
    Akonadi::ChangeRecorder *m_recorder;
    Akonadi::EntityTreeView *m_availableView, *m_selectedView;
    ProxyModel *m_availableModel;
    QStandardItemModel *m_selectedModel;
    QStandardItem *m_toItem, *m_ccItem, *m_bccItem;
    QPushButton *m_toButton, *m_ccButton, *m_bccButton, *m_removeButton, *m_saveButton;
    LdapSearchDialog  *m_ldapSearchDialog;

    QStandardItem* selectedToItem();
    QStandardItem* selectedCcItem();
    QStandardItem* selectedBccItem();

    KABC::Addressee::List allAddressee( QStandardItem* parent ) const;
    void addToSelected( const Akonadi::Item& item, QStandardItem *parentItem );
    void addContactToSelected( const KABC::Addressee& contact, QStandardItem *parentItem );
    void addGroupToSelected( const KABC::ContactGroup& group, QStandardItem *parentItem );
    QStringList allDistributionLists( QStandardItem* parent ) const;
    QStringList entryToString( const KABC::Addressee::List& l ) const;
};

}

#endif
