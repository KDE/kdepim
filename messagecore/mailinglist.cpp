// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

#include "mailinglist.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kurl.h>
#include <kdebug.h>

#include <QtCore/QSharedData>
#include <QtCore/QStringList>

#include <boost/shared_ptr.hpp>

using namespace MessageCore;

typedef QString (*MagicDetectorFunc)( const KMime::Message::Ptr&, QByteArray&, QString& );

/* Sender: (owner-([^@]+)|([^@+]-owner)@ */
static QString check_sender( const KMime::Message::Ptr &message,
                             QByteArray &headerName,
                             QString &headerValue )
{
  QString header = message->sender()->asUnicodeString();

  if ( header.isEmpty() )
    return QString();

  if ( header.left( 6 ) == QLatin1String("owner-") ) {
    headerName = "Sender";
    headerValue = header;
    header = header.mid( 6, header.indexOf( '@' ) - 6 );
  } else {
    const int index = header.indexOf( "-owner@ " );
    if ( index == -1 )
      return QString();

    header.truncate( index );
    headerName = "Sender";
    headerValue = header;
  }

  return header;
}

/* X-BeenThere: ([^@]+) */
static QString check_x_beenthere( const KMime::Message::Ptr &message,
                                  QByteArray &headerName,
                                  QString &headerValue )
{
  QString header = message->headerByType( "X-BeenThere" ) ? message->headerByType( "X-BeenThere" )->asUnicodeString() : "";
  if ( header.isNull() || header.indexOf( '@' ) == -1 )
    return QString();

  headerName = "X-BeenThere";
  headerValue = header;
  header.truncate( header.indexOf( '@' ) );

  return header;
}

/* Delivered-To:: <([^@]+) */
static QString check_delivered_to( const KMime::Message::Ptr &message,
                                   QByteArray &headerName,
                                   QString &headerValue )
{
  QString header = message->headerByType( "Delivered-To" ) ? message->headerByType( "Delivered-To" )->asUnicodeString() : "";
  if ( header.isNull() || header.left( 13 ) != "mailing list"
       || header.indexOf( '@' ) == -1 )
    return QString();

  headerName = "Delivered-To";
  headerValue = header;

  return header.mid( 13, header.indexOf( '@' ) - 13 );
}

/* X-Mailing-List: <?([^@]+) */
static QString check_x_mailing_list( const KMime::Message::Ptr &message,
                                     QByteArray &headerName,
                                     QString &headerValue )
{
  QString header = message->headerByType( "X-Mailing-List" ) ? message->headerByType( "X-Mailing-List" )->asUnicodeString() : "";
  if ( header.isEmpty() )
    return QString();

  if ( header.indexOf( '@' ) < 1 )
    return QString();

  headerName = "X-Mailing-List";
  headerValue = header;
  if ( header[0] == '<' )
    header = header.mid( 1,  header.indexOf( '@' ) - 1 );
  else
    header.truncate( header.indexOf( '@' ) );

  return header;
}

/* List-Id: [^<]* <([^.]+) */
static QString check_list_id( const KMime::Message::Ptr &message,
                              QByteArray &headerName,
                              QString &headerValue )
{
  QString header = message->headerByType( "List-Id" ) ? message->headerByType( "List-Id" )->asUnicodeString() : "";
  if ( header.isEmpty() )
    return QString();

  const int leftAnglePos = header.indexOf( '<' );
  if ( leftAnglePos < 0 )
    return QString();

  const int firstDotPos = header.indexOf( '.', leftAnglePos );
  if ( firstDotPos < 0 )
    return QString();

  headerName = "List-Id";
  headerValue = header.mid( leftAnglePos );
  header = header.mid( leftAnglePos + 1, firstDotPos - leftAnglePos - 1 );

  return header;
}


/* List-Post: <mailto:[^< ]*>) */
static QString check_list_post( const KMime::Message::Ptr &message,
                                QByteArray &headerName,
                                QString &headerValue )
{
  QString header = message->headerByType( "List-Post" ) ? message->headerByType( "List-Post" )->asUnicodeString() : "";
  if ( header.isEmpty() )
    return QString();

  int leftAnglePos = header.indexOf( "<mailto:" );
  if ( leftAnglePos < 0 )
    return QString();

  headerName = "List-Post";
  headerValue = header;
  header = header.mid( leftAnglePos + 8, header.length() );
  header.truncate( header.indexOf( '@' ) );

  return header;
}

/* Mailing-List: list ([^@]+) */
static QString check_mailing_list( const KMime::Message::Ptr &message,
                                   QByteArray &headerName,
                                   QString &headerValue )
{
  QString header = message->headerByType( "Mailing-List" ) ? message->headerByType( "Mailing-List" )->asUnicodeString() : "";
  if ( header.isEmpty() )
    return QString();

  if ( header.left( 5 ) != "list " || header.indexOf( '@' ) < 5 )
    return QString();

  headerName = "Mailing-List";
  headerValue = header;
  header = header.mid( 5,  header.indexOf( '@' ) - 5 );

  return header;
}


/* X-Loop: ([^@]+) */
static QString check_x_loop( const KMime::Message::Ptr &message,
                             QByteArray &headerName,
                             QString &headerValue )
{
  QString header = message->headerByType( "X-Loop" ) ? message->headerByType( "X-Loop" )->asUnicodeString() : "";
  if ( header.isEmpty() )
    return QString();

  if (header.indexOf( '@' ) < 2 )
    return QString();

  headerName = "X-Loop";
  headerValue = header;
  header.truncate( header.indexOf( '@' ) );

  return header;
}

/* X-ML-Name: (.+) */
static QString check_x_ml_name( const KMime::Message::Ptr &message,
                                QByteArray &headerName,
                                QString &headerValue )
{
  QString header = message->headerByType( "X-ML-Name" ) ? message->headerByType( "X-ML-Name" )->asUnicodeString() : "";
  if ( header.isEmpty() )
    return QString();

  headerName = "X-ML-Name";
  headerValue = header;
  header.truncate( header.indexOf( '@' ) );

  return header;
}

MagicDetectorFunc magic_detector[] =
{
  check_list_id,
  check_list_post,
  check_sender,
  check_x_mailing_list,
  check_mailing_list,
  check_delivered_to,
  check_x_beenthere,
  check_x_loop,
  check_x_ml_name
};

static const int num_detectors = sizeof( magic_detector ) / sizeof( magic_detector[0] );

static QStringList headerToAddress( const QString &header )
{
  QStringList addresses;
  int start = 0;
  int end = 0;

  if ( header.isEmpty() )
    return addresses;

  while ( (start = header.indexOf( "<", start )) != -1 ) {
    if ( (end = header.indexOf( ">", ++start ) ) == -1 ) {
      kWarning() << "Serious mailing list header parsing error!";
      return addresses;
    }

    addresses.append( header.mid( start, end - start ) );
  }

  return  addresses;
}

class MessageCore::MailingList::Private : public QSharedData
{
  public:
    Private()
      : mFeatures( None ),
        mHandler( KMail )
    {
    }

    Private( const Private &other )
      : QSharedData( other )
    {
      mFeatures = other.mFeatures;
      mHandler = other.mHandler;
      mPostUrls = other.mPostUrls;
      mSubscribeUrls = other.mSubscribeUrls;
      mUnsubscribeUrls = other.mUnsubscribeUrls;
      mHelpUrls = other.mHelpUrls;
      mArchiveUrls = other.mArchiveUrls;
      mOwnerUrls = other.mOwnerUrls;
      mArchivedAtUrl = other.mArchivedAtUrl;
      mId = other.mId;
    }

    Features mFeatures;
    Handler mHandler;
    KUrl::List mPostUrls;
    KUrl::List mSubscribeUrls;
    KUrl::List mUnsubscribeUrls;
    KUrl::List mHelpUrls;
    KUrl::List mArchiveUrls;
    KUrl::List mOwnerUrls;
    KUrl mArchivedAtUrl;
    QString mId;
};

MailingList MailingList::detect( const KMime::Message::Ptr &message )
{
  MailingList mailingList;

  if ( message->headerByType( "List-Post" ) )
    mailingList.setPostUrls( headerToAddress( message->headerByType( "List-Post" )->asUnicodeString() ) );

  if ( message->headerByType( "List-Help" ) )
    mailingList.setHelpUrls( headerToAddress( message->headerByType( "List-Help" )->asUnicodeString() ) );

  if ( message->headerByType( "List-Subscribe" ) )
    mailingList.setSubscribeUrls( headerToAddress( message->headerByType( "List-Subscribe" )->asUnicodeString() ) );

  if ( message->headerByType( "List-Unsubscribe" ) )
    mailingList.setUnsubscribeUrls( headerToAddress( message->headerByType( "List-Unsubscribe" )->asUnicodeString() ) );

  if ( message->headerByType( "List-Archive" ) )
    mailingList.setArchiveUrls( headerToAddress( message->headerByType( "List-Archive" )->asUnicodeString() ) );

  if ( message->headerByType( "List-Owner" ) )
    mailingList.setOwnerUrls( headerToAddress( message->headerByType( "List-Owner" )->asUnicodeString() ) );

  if ( message->headerByType( "Archived-At" ) )
    mailingList.setArchivedAtUrl( KUrl( message->headerByType( "Archived-At" )->asUnicodeString() ) );

  if ( message->headerByType( "List-Id" ) )
    mailingList.setId( message->headerByType( "List-Id" )->asUnicodeString() );

  return mailingList;
}

QString MailingList::name( const KMime::Message::Ptr &message,
                           QByteArray &headerName, QString &headerValue )
{
  QString mailingList;
  headerName = QByteArray();
  headerValue.clear();

  if ( !message )
    return QString();

  for ( int i = 0; i < num_detectors; i++ ) {
    mailingList = magic_detector[i]( message, headerName, headerValue );
    if ( !mailingList.isNull() )
      return mailingList;
  }

  return QString();
}

MailingList::MailingList()
  : d( new Private )
{
}

MailingList::MailingList( const MailingList &other )
  : d( other.d )
{
}

MailingList& MailingList::operator=( const MailingList &other )
{
  if ( this != &other )
    d = other.d;

  return *this;
}

MailingList::~MailingList()
{
}

MailingList::Features MailingList::features() const
{
  return d->mFeatures;
}

void MailingList::setHandler( MailingList::Handler handler )
{
  d->mHandler = handler;
}

MailingList::Handler MailingList::handler() const
{
  return d->mHandler;
}

void MailingList::setPostUrls( const KUrl::List &urls )
{
  d->mFeatures |= Post;

  if ( urls.empty() ) {
    d->mFeatures ^= Post;
  }

  d->mPostUrls = urls;
}

KUrl::List MailingList::postUrls() const
{
  return d->mPostUrls;
}

void MailingList::setSubscribeUrls( const KUrl::List &urls )
{
  d->mFeatures |= Subscribe;

  if ( urls.empty() ) {
    d->mFeatures ^= Subscribe;
  }

  d->mSubscribeUrls = urls;
}

KUrl::List MailingList::subscribeUrls() const
{
  return d->mSubscribeUrls;
}

void MailingList::setUnsubscribeUrls( const KUrl::List &urls )
{
  d->mFeatures |= Unsubscribe;

  if ( urls.empty() ) {
    d->mFeatures ^= Unsubscribe;
  }

  d->mUnsubscribeUrls = urls;
}

KUrl::List MailingList::unsubscribeUrls() const
{
  return d->mUnsubscribeUrls;
}

void MailingList::setHelpUrls( const KUrl::List &urls )
{
  d->mFeatures |= Help;

  if ( urls.empty() ) {
    d->mFeatures ^= Help;
  }

  d->mHelpUrls = urls;
}

KUrl::List MailingList::helpUrls() const
{
  return d->mHelpUrls;
}

void MailingList::setArchiveUrls( const KUrl::List &urls )
{
  d->mFeatures |= Archive;

  if ( urls.empty() ) {
    d->mFeatures ^= Archive;
  }

  d->mArchiveUrls = urls;
}

KUrl::List MailingList::archiveUrls() const
{
  return d->mArchiveUrls;
}

void MailingList::setOwnerUrls( const KUrl::List &urls )
{
  d->mFeatures |= Owner;

  if ( urls.empty() ) {
    d->mFeatures ^= Owner;
  }

  d->mOwnerUrls = urls;
}

KUrl::List MailingList::ownerUrls() const
{
  return d->mOwnerUrls;
}

void MailingList::setArchivedAtUrl( const KUrl &url )
{
  d->mFeatures |= ArchivedAt;

  if ( !url.isValid() ) {
    d->mFeatures ^= ArchivedAt;
  }

  d->mArchivedAtUrl = url;
}

KUrl MailingList::archivedAtUrl() const
{
  return d->mArchivedAtUrl;
}

void MailingList::setId( const QString &id )
{
  d->mFeatures |= Id;

  if ( id.isEmpty() ) {
    d->mFeatures ^= Id;
  }

  d->mId = id;
}

QString MailingList::id() const
{
  return d->mId;
}

void MailingList::writeConfig( KConfigGroup &group ) const
{
  group.writeEntry( "MailingListFeatures", static_cast<int>( d->mFeatures ) );
  group.writeEntry( "MailingListHandler", static_cast<int>( d->mHandler ) );
  group.writeEntry( "MailingListId", d->mId );
  group.writeEntry( "MailingListPostingAddress", d->mPostUrls.toStringList() );
  group.writeEntry( "MailingListSubscribeAddress", d->mSubscribeUrls.toStringList() );
  group.writeEntry( "MailingListUnsubscribeAddress", d->mUnsubscribeUrls.toStringList() );
  group.writeEntry( "MailingListArchiveAddress", d->mArchiveUrls.toStringList() );
  group.writeEntry( "MailingListOwnerAddress", d->mOwnerUrls.toStringList() );
  group.writeEntry( "MailingListHelpAddress", d->mHelpUrls.toStringList() );
  /* Note: mArchivedAtUrl deliberately not saved here as it refers to a single 
   * instance of a message rather than an element of a general mailing list.
   * http://reviewboard.kde.org/r/1768/#review2783
   */
}

void MailingList::readConfig( const KConfigGroup &group )
{
  d->mFeatures = static_cast<MailingList::Features>( group.readEntry( "MailingListFeatures", 0 ) );
  d->mHandler = static_cast<MailingList::Handler>( group.readEntry( "MailingListHandler",
                                                                    static_cast<int>( MailingList::KMail ) ) );
  d->mId = group.readEntry("MailingListId");
  d->mPostUrls = group.readEntry( "MailingListPostingAddress", QStringList() );
  d->mSubscribeUrls = group.readEntry( "MailingListSubscribeAddress", QStringList() );
  d->mUnsubscribeUrls = group.readEntry( "MailingListUnsubscribeAddress", QStringList() );
  d->mArchiveUrls = group.readEntry( "MailingListArchiveAddress", QStringList() );
  d->mOwnerUrls = group.readEntry( "MailingListOwnerddress", QStringList() );
  d->mHelpUrls = group.readEntry( "MailingListHelpAddress", QStringList() );
}
