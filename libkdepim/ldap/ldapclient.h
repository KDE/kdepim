/* kldapclient.h - LDAP access
 *      Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KLDAP_LDAPCLIENT_H
#define KLDAP_LDAPCLIENT_H

#include "libkdepim/kdepim_export.h"

#include <QtCore/QObject>
#include <QtCore/QStringList>

class KConfig;
class KConfigGroup;
class KJob;

namespace KLDAP {

class LdapObject;
class LdapServer;

/**
 * @short An object that represents a configured LDAP server.
 *
 * This class represents a client that to an LDAP server that
 * can be used for LDAP lookups. Every client is identified by
 * a unique numeric id.
 *
 * @since 4.5
 */
class KDEPIM_EXPORT LdapClient : public QObject
{
  Q_OBJECT

  public:
    /**
     * Creates a new ldap client.
     *
     * @param clientNumber The unique number of this client.
     * @param parent The parent object.
     */
    explicit LdapClient( int clientNumber, QObject *parent = 0 );

    /**
     * Destroys the ldap client.
     */
    virtual ~LdapClient();

    /**
     * Returns the number of this client.
     */
    int clientNumber() const;

    /**
     * Returns whether this client is currently running
     * a search query.
     */
    bool isActive() const;

    /**
     * Sets the completion @p weight of this client.
     *
     * This value will be used to sort the results of this
     * client when used for auto completion.
     */
    void setCompletionWeight( int weight );

    /**
     * Returns the completion weight of this client.
     */
    int completionWeight() const;

    /**
     * Sets the LDAP @p server information that shall be
     * used by this client.
     */
    void setServer( const KLDAP::LdapServer &server );

    /**
     * Returns the ldap server information that are used
     * by this client.
     */
    const KLDAP::LdapServer server() const;

    /**
     * Sets the LDAP @p attributes that should be returned
     * in the query result.
     *
     * Pass an empty list to include all available attributes.
     */
    void setAttributes( const QStringList &attributes );

    /**
     * Returns the LDAP attributes that should be returned
     * in the query result.
     */
    QStringList attributes() const;

    /**
     * Sets the @p scope of the LDAP query.
     *
     * Valid values are 'one' or 'sub'.
     */
    void setScope( const QString scope );

    /**
     * Starts the query with the given @p filter.
     */
    void startQuery( const QString &filter );

    /**
     * Cancels a running query.
     */
    void cancelQuery();

  Q_SIGNALS:
    /**
     * This signal is emitted when the query has finished.
     */
    void done();

    /**
     * This signal is emitted in case of an error.
     *
     * @param message A message that describes the error.
     */
    void error( const QString &message );

    /**
     * This signal is emitted once for each object that is
     * returned from the query
     */
    void result( const KLDAP::LdapClient &client, const KLDAP::LdapObject& );

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void slotData( KIO::Job*, const QByteArray& ) )
    Q_PRIVATE_SLOT( d, void slotData( const QByteArray& ) )
    Q_PRIVATE_SLOT( d, void slotInfoMessage( KJob*, const QString&, const QString& ) )
    Q_PRIVATE_SLOT( d, void slotDone() )
    //@endcond
};

/**
 * Describes the result returned by an LdapClientSearch query.
 *
 * @since 4.5
 */
struct LdapResult
{
  /**
   * A list of LdapResult objects.
   */
  typedef QList<LdapResult> List;

  QString name;         ///< The full name of the contact.
  QStringList email;    ///< The list of emails of the contact.
  int clientNumber;     ///< The client the contact comes from (used for sorting in a ldap-only lookup).
  int completionWeight; ///< The weight of the contact (used for sorting in a completion list).
};

/**
 * @since 4.5
 */
class KDEPIM_EXPORT LdapClientSearch : public QObject
{
  Q_OBJECT

  public:
    /**
     * Creates a new ldap client search object.
     *
     * @param parent The parent object.
     */
    explicit LdapClientSearch( QObject *parent = 0 );

    /**
     * Destroys the ldap client search object.
     */
    ~LdapClientSearch();

    /**
     * Returns the global config object, which stores the LdapClient configurations.
     */
    static KConfig *config();

    /**
     * Reads the LDAP @p server settings from the given config @p group for the
     * given LDAP @p clientNumber.
     *
     * @param active Defines whether the active settings shall be read.
     */
    static void readConfig( KLDAP::LdapServer &server, const KConfigGroup &group,
                            int clientNumber, bool active );

    /**
     * Writes the LDAP @p server settings to the given config @p group for the
     * given LDAP @p clientNumber.
     *
     * @param active Defines whether the active settings shall be written.
     */
    static void writeConfig( const KLDAP::LdapServer &server, KConfigGroup &group,
                             int clientNumber, bool active );

    /**
     * Starts the LDAP search on all configured LDAP clients with the given search @p query.
     */
    void startSearch( const QString &query );

    /**
     * Cancels the currently running search query.
     */
    void cancelSearch();

    /**
     * Returns whether LDAP search is possible at all.
     *
     * @note This method can return @c false if either no LDAP is configured
     *       or the system does not support the KIO LDAP protocol.
     */
    bool isAvailable() const;

    /**
     * Updates the completion weights for the configured LDAP clients from
     * the configuration file.
     */
    void updateCompletionWeights();

    /**
     * Returns the list of configured LDAP clients.
     */
    QList<LdapClient*> clients() const;

  Q_SIGNALS:
    /**
     * This signal is emitted whenever new contacts have been found
     * during the lookup.
     *
     * @param results The contacts in the form "Full Name <email>"
     */
    void searchData( const QStringList &results );

    /**
     * This signal is emitted whenever new contacts have been found
     * during the lookup.
     *
     * @param results The list of found contacts.
     */
    void searchData( const KLDAP::LdapResult::List &results );

    /**
     * This signal is emitted whenever the lookup is complete or the
     * user has canceled the query.
     */
    void searchDone();

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void slotLDAPResult( const KLDAP::LdapClient&, const KLDAP::LdapObject& ) )
    Q_PRIVATE_SLOT( d, void slotLDAPError( const QString& ) )
    Q_PRIVATE_SLOT( d, void slotLDAPDone() )
    Q_PRIVATE_SLOT( d, void slotDataTimer() )
    Q_PRIVATE_SLOT( d, void slotFileChanged( const QString& ) )
    //@endcond
};

}

#endif
