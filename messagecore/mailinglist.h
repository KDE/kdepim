// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

#ifndef MESSAGECORE_MAILINGLIST_H
#define MESSAGECORE_MAILINGLIST_H

#include "messagecore_export.h"

#include <kmime/kmime_message.h>
#include <kurl.h>

#include <QtCore/QByteArray>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>

class KConfigGroup;

namespace MessageCore
{

/**
 * @short A class to extract information about mailing lists from emails.
 *
 * The mailing list header fields are defined as the following:
 *   - "List-*" in RFC2369
 *   - "List-ID" in RFC2919.
 *   - "Archive-At" in RFC5064
 *
 * @author Zack Rusin <zack@kde.org>
 */
class MESSAGECORE_EXPORT MailingList
{
  public:
    /**
     * Defines what entity should manage the mailing list.
     */
    enum Handler {
      KMail,   ///< The list is handled by KMail
      Browser  ///< The list is handled by a browser.
    };

    /**
     * Defines the features a mailinglist can suppport.
     */
    enum Feature {
      None         = 0 << 0, ///< No mailing list fields exist.
      Post         = 1 << 0, ///< List-Post header exists.
      Subscribe    = 1 << 1, ///< List-Subscribe header exists.
      Unsubscribe  = 1 << 2, ///< List-Unsubscribe header exists.
      Help         = 1 << 3, ///< List-Help header exists.
      Archive      = 1 << 4, ///< List-Archive header exists.
      Id           = 1 << 5, ///< List-ID header exists.
      Owner        = 1 << 6, ///< List-Owner header exists.
      ArchivedAt   = 1 << 7  ///< Archive-At header exists.
    };
    Q_DECLARE_FLAGS( Features, Feature )

  public:
    /**
     * Extracts the information about a mailing list from the given @p message.
     */
    static MailingList detect(  const KMime::Message::Ptr &message );

    static QString name( const KMime::Message::Ptr &message, QByteArray &headerName,
                         QString &headerValue );

  public:
    /**
     * Creates an empty mailing list.
     */
    MailingList();

    /**
     * Creates a mailing list from an @p other mailing list.
     */
    MailingList( const MailingList &other );

    /**
     * Overwrites this mailing list with an @p other mailing list.
     */
    MailingList &operator=( const MailingList &other );

    bool operator==( const MailingList &other ) const;
    /**
     * Destroys the mailing list.
     */
    ~MailingList();

    /**
     * Returns the features the mailing list supports. 
     */
    Features features() const;

    /**
     * Sets the @p handler for the mailing list.
     */
    void setHandler( Handler handler );

    /**
     * Returns the handler for the mailing list.
     */
    Handler handler() const;

    /**
     * Sets the list of List-Post @p urls.
     */
    void setPostUrls( const KUrl::List &urls );

    /**
     * Returns the list of List-Post urls.
     */
    KUrl::List postUrls() const;

    /**
     * Sets the list of List-Subscribe @p urls.
     */
    void setSubscribeUrls( const KUrl::List &urls );

    /**
     * Returns the list of List-Subscribe urls.
     */
    KUrl::List subscribeUrls() const;

    /**
     * Sets the list of List-Unsubscribe @p urls.
     */
    void setUnsubscribeUrls( const KUrl::List &urls );

    /**
     * Returns the list of List-Unsubscribe urls.
     */
    KUrl::List unsubscribeUrls() const;

    /**
     * Sets the list of List-Help @p urls.
     */
    void setHelpUrls( const KUrl::List &urls );

    /**
     * Returns the list of List-Help urls.
     */
    KUrl::List helpUrls() const;

    /**
     * Sets the list of List-Archive @p urls.
     */
    void setArchiveUrls( const KUrl::List &urls );

    /**
     * Returns the list of List-Archive urls.
     */
    KUrl::List archiveUrls() const;

    /**
     * Sets the list of List-Owner @p urls.
     */
    void setOwnerUrls( const KUrl::List &urls );

    /**
     * Returns the list of List-Owner urls.
     */
    KUrl::List ownerUrls() const;

    /**
     * Sets the Archived-At @p url.
     */
    void setArchivedAtUrls( const KUrl::List &url );

    /**
     * Returns the Archived-At @p url.
     */
    KUrl::List archivedAtUrls() const;

    /**
     * Sets the @p id of the mailing list.
     */
    void setId( const QString &id );

    /**
     * Returns the @p id of the mailing list.
     */
    QString id() const;

    /**
     * Saves the configuration for the mailing list to the config @p group.
     */
    void writeConfig( KConfigGroup &group ) const;

    /**
     * Restores the configuration for the mailing list from the config @p group.
     */
    void readConfig( const KConfigGroup &group );

  private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS( MessageCore::MailingList::Features )
Q_DECLARE_METATYPE( MessageCore::MailingList::Features )

#endif
