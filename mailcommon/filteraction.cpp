// kmfilteraction.cpp

#include "filteraction.h"

// other KMail headers:
#include "filtermanager.h"
#include "folderrequester.h"
#include "mailutil.h"
#include "mailkernel.h"

#include "messageproperty.h"
using MailCommon::MessageProperty;
#include "regexplineedit.h"
using MailCommon::RegExpLineEdit;

// KD PIM headers
#include "messagecore/stringutil.h"
#include "messagecore/messagehelpers.h"

#include "messagecore/kmfawidgets.h"

#include "messagecomposer/messagesender.h"
#include "messagecomposer/messagefactory.h"
using MessageComposer::MessageFactory;

#include "templateparser/customtemplates_kfg.h"
#include "templateparser/customtemplates.h"

#include "libkdepim/addcontactjob.h"

// KDE PIM libs headers
#include <kpimidentities/identity.h>
#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identitycombo.h>
#include <kpimutils/kfileio.h>
#include <kpimutils/email.h>
#include <akonadi/itemcopyjob.h>
#include <akonadi/itemmodifyjob.h>
#include <kmime/kmime_message.h>

// KDE headers
#include <akonadi/collectioncombobox.h>
#include <kabc/addressee.h>
#include <kcombobox.h>
#include <ktemporaryfile.h>
#include <kdebug.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <phonon/mediaobject.h>
#include <kshell.h>
#include <kprocess.h>
#include <nepomuk/tag.h>

#include <akonadi/kmime/messagestatus.h>

// Qt headers:

// other headers:
#include <assert.h>
#include <string.h>
#include "mdnadvicedialog.h"
#include <messagecore/mdnstateattribute.h>
#include <messagecore/kmfawidgets.h>
#include <QTextDocument>
#include <QHBoxLayout>
#include <QLabel>
#ifndef _WIN32_WCE
#include "soundtestwidget.h"
#endif

using namespace MailCommon;

//=============================================================================
//
// FilterAction
//
//=============================================================================

FilterAction::FilterAction( const char* aName, const QString &aLabel )
{
  mName = aName;
  mLabel = aLabel;
}

FilterAction::~FilterAction()
{
}

bool FilterAction::requiresBody() const
{
  return true;
}

FilterAction* FilterAction::newAction()
{
  return 0;
}

QWidget* FilterAction::createParamWidget(QWidget* parent) const
{
  return new QWidget(parent);
}

void FilterAction::applyParamWidgetValue(QWidget*)
{
}

void FilterAction::setParamWidgetValue( QWidget * ) const
{
}

void FilterAction::clearParamWidget( QWidget * ) const
{
}

bool FilterAction::folderRemoved(const Akonadi::Collection&, const Akonadi::Collection&)
{
  return false;
}

void FilterAction::sendMDN( const Akonadi::Item & item, KMime::MDN::DispositionType d,
                              const QList<KMime::MDN::DispositionModifier> & m ) {
  KMime::Message::Ptr msg = MessageCore::Util::message( item );
  if ( !msg ) return;

  QPair< bool, KMime::MDN::SendingMode > mdnSend = MDNAdviceHelper::instance()->checkAndSetMDNInfo( item, d );
  if( mdnSend.first ) {
    KConfigGroup mdnConfig( KernelIf->config(), "MDN" );
    int quote = mdnConfig.readEntry<int>( "quote-message", 0 );
    MessageFactory factory( msg, Akonadi::Item().id() );
    factory.setIdentityManager( KernelIf->identityManager() );
    KMime::Message::Ptr mdn = factory.createMDN( KMime::MDN::AutomaticAction, d, mdnSend.second, quote, m );
    if ( mdn ) {
      if( !KernelIf->msgSender()->send( mdn, MessageSender::SendLater ) ) {
        kDebug() << "Sending failed.";
      }
    }
  }
}


//=============================================================================
//
// FilterActionWithNone
//
//=============================================================================

FilterActionWithNone::FilterActionWithNone( const char* aName, const QString &aLabel )
  : FilterAction( aName, aLabel )
{
}

const QString FilterActionWithNone::displayString() const
{
  return label();
}


//=============================================================================
//
// FilterActionWithUOID
//
//=============================================================================

FilterActionWithUOID::FilterActionWithUOID( const char* aName, const QString &aLabel )
  : FilterAction( aName, aLabel ), mParameter( 0 )
{
}

void FilterActionWithUOID::argsFromString( const QString &argsStr )
{
  mParameter = argsStr.trimmed().toUInt();
}

const QString FilterActionWithUOID::argsAsString() const
{
  return QString::number( mParameter );
}

const QString FilterActionWithUOID::displayString() const
{
  return label() + " \"" + Qt::escape( argsAsString() ) + "\"";
}


//=============================================================================
//
// FilterActionWithString
//
//=============================================================================

FilterActionWithString::FilterActionWithString( const char* aName, const QString &aLabel )
  : FilterAction( aName, aLabel )
{
}

QWidget* FilterActionWithString::createParamWidget( QWidget* parent ) const
{
  KLineEdit *le = new KLineEdit(parent);
  le->setClearButtonShown( true );
  le->setText( mParameter );
  return le;
}

void FilterActionWithString::applyParamWidgetValue( QWidget* paramWidget )
{
  mParameter = ((KLineEdit*)paramWidget)->text();
}

void FilterActionWithString::setParamWidgetValue( QWidget* paramWidget ) const
{
  ((KLineEdit*)paramWidget)->setText( mParameter );
}

void FilterActionWithString::clearParamWidget( QWidget* paramWidget ) const
{
  ((KLineEdit*)paramWidget)->clear();
}

void FilterActionWithString::argsFromString( const QString &argsStr )
{
  mParameter = argsStr;
}

const QString FilterActionWithString::argsAsString() const
{
  return mParameter;
}

const QString FilterActionWithString::displayString() const
{
  return label() + " \"" + Qt::escape( argsAsString() ) + "\"";
}

//=============================================================================
//
// class FilterActionWithStringList
//
//=============================================================================

FilterActionWithStringList::FilterActionWithStringList( const char* aName, const QString &aLabel )
  : FilterActionWithString( aName, aLabel )
{
}

QWidget* FilterActionWithStringList::createParamWidget( QWidget* parent ) const
{
  KComboBox *cb = new KComboBox( parent );
  cb->setEditable( false );
  cb->addItems( mParameterList );
  setParamWidgetValue( cb );
  return cb;
}

void FilterActionWithStringList::applyParamWidgetValue( QWidget* paramWidget )
{
  mParameter = ((KComboBox*)paramWidget)->currentText();
}

void FilterActionWithStringList::setParamWidgetValue( QWidget* paramWidget ) const
{
  const int idx = mParameterList.indexOf( mParameter );
  ((KComboBox*)paramWidget)->setCurrentIndex( idx >= 0 ? idx : 0 );
}

void FilterActionWithStringList::clearParamWidget( QWidget* paramWidget ) const
{
  ((KComboBox*)paramWidget)->setCurrentIndex(0);
}

void FilterActionWithStringList::argsFromString( const QString &argsStr )
{
  int idx = mParameterList.indexOf( argsStr );
  if ( idx < 0 ) {
    mParameterList.append( argsStr );
    idx = mParameterList.count() - 1;
  }
  mParameter = mParameterList.at( idx );
}


//=============================================================================
//
// class FilterActionWithFolder
//
//=============================================================================

FilterActionWithFolder::FilterActionWithFolder( const char* aName, const QString &aLabel )
  : FilterAction( aName, aLabel )
{
}

QWidget* FilterActionWithFolder::createParamWidget( QWidget* parent ) const
{
  FolderRequester *req = new FolderRequester( parent );
  req->setShowOutbox( false );
  setParamWidgetValue( req );
  return req;
}

void FilterActionWithFolder::applyParamWidgetValue( QWidget* paramWidget )
{
  mFolder = ((FolderRequester *)paramWidget)->folderCollection();
  mFolderName = ((FolderRequester *)paramWidget)->folderId();
}

void FilterActionWithFolder::setParamWidgetValue( QWidget* paramWidget ) const
{
  if ( mFolder.isValid() )
    ((FolderRequester *)paramWidget)->setFolder( mFolder );
  else
    ((FolderRequester *)paramWidget)->setFolder( mFolderName );
}

void FilterActionWithFolder::clearParamWidget( QWidget* paramWidget ) const
{
  ((FolderRequester *)paramWidget)->setFolder( CommonKernel->draftsCollectionFolder() );
}

void FilterActionWithFolder::argsFromString( const QString &argsStr )
{
  mFolder = CommonKernel->collectionFromId( argsStr );
  if ( mFolder.isValid() )
    mFolderName= QString::number(mFolder.id());
  else
    mFolderName = argsStr;
}

const QString FilterActionWithFolder::argsAsString() const
{
  QString result;
  if ( mFolder.isValid() )
    result = QString::number(mFolder.id());
  else
    result = mFolderName;
  return result;
}

const QString FilterActionWithFolder::displayString() const
{
  QString result;
  if ( mFolder.isValid() )
    result = mFolder.url().path();
  else
    result = mFolderName;
  return label() + " \"" + Qt::escape( result ) + "\"";
}

bool FilterActionWithFolder::folderRemoved( const Akonadi::Collection& aFolder, const Akonadi::Collection& aNewFolder )
{
  if ( aFolder == mFolder ) {
    mFolder = aNewFolder;
    if ( aNewFolder.isValid() )
      mFolderName = mFolder.id();
    return true;
  } else
    return false;
}

//=============================================================================
//
// class FilterActionWithAddress
//
//=============================================================================

FilterActionWithAddress::FilterActionWithAddress( const char* aName, const QString &aLabel )
  : FilterActionWithString( aName, aLabel )
{
}

QWidget* FilterActionWithAddress::createParamWidget( QWidget* parent ) const
{
  KMFilterActionWithAddressWidget *w = new KMFilterActionWithAddressWidget(parent);
  w->setText( mParameter );
  return w;
}

void FilterActionWithAddress::applyParamWidgetValue( QWidget* paramWidget )
{
  mParameter = ((KMFilterActionWithAddressWidget*)paramWidget)->text();
}

void FilterActionWithAddress::setParamWidgetValue( QWidget* paramWidget ) const
{
  ((KMFilterActionWithAddressWidget*)paramWidget)->setText( mParameter );
}

void FilterActionWithAddress::clearParamWidget( QWidget* paramWidget ) const
{
  ((KMFilterActionWithAddressWidget*)paramWidget)->clear();
}

//=============================================================================
//
// class FilterActionWithCommand
//
//=============================================================================

FilterActionWithCommand::FilterActionWithCommand( const char* aName, const QString &aLabel )
  : FilterActionWithUrl( aName, aLabel )
{
}

QWidget* FilterActionWithCommand::createParamWidget( QWidget* parent ) const
{
  return FilterActionWithUrl::createParamWidget( parent );
}

void FilterActionWithCommand::applyParamWidgetValue( QWidget* paramWidget )
{
  FilterActionWithUrl::applyParamWidgetValue( paramWidget );
}

void FilterActionWithCommand::setParamWidgetValue( QWidget* paramWidget ) const
{
  FilterActionWithUrl::setParamWidgetValue( paramWidget );
}

void FilterActionWithCommand::clearParamWidget( QWidget* paramWidget ) const
{
  FilterActionWithUrl::clearParamWidget( paramWidget );
}

static KMime::Content* findMimeNodeForIndex( KMime::Content* node, int &index )
{
  if ( index <= 0 )
    return node;
  foreach ( KMime::Content* child, node->contents() ) {
    KMime::Content *result = findMimeNodeForIndex( child, --index );
    if ( result )
      return result;
  }
  return 0;
}

QString FilterActionWithCommand::substituteCommandLineArgsFor( const KMime::Message::Ptr &aMsg, QList<KTemporaryFile*> & aTempFileList ) const
{
  QString result = mParameter;
  QList<int> argList;
  QRegExp r( "%[0-9-]+" );

  // search for '%n'
  int start = -1;
  while ( ( start = r.indexIn( result, start + 1 ) ) > 0 ) {
    int len = r.matchedLength();
    // and save the encountered 'n' in a list.
    bool OK = false;
    int n = result.mid( start + 1, len - 1 ).toInt( &OK );
    if ( OK )
      argList.append( n );
  }

  // sort the list of n's
  qSort( argList );

  // and use QString::arg to substitute filenames for the %n's.
  int lastSeen = -2;
  QString tempFileName;
  for ( QList<int>::Iterator it = argList.begin() ; it != argList.end() ; ++it ) {
    // setup temp files with check for duplicate %n's
    if ( (*it) != lastSeen ) {
      KTemporaryFile *tf = new KTemporaryFile();
      if ( !tf->open() ) {
        delete tf;
        kDebug() << "FilterActionWithCommand: Could not create temp file!";
        return QString();
      }
      aTempFileList.append( tf );
      tempFileName = tf->fileName();
      if ((*it) == -1)
        KPIMUtils::kByteArrayToFile( aMsg->encodedContent(), tempFileName, //###
                          false, false, false );
      else if (aMsg->contents().size() == 0)
        KPIMUtils::kByteArrayToFile( aMsg->decodedContent(), tempFileName,
                          false, false, false );
      else {
        int index = *it; // we pass by reference below, so this is not const
        KMime::Content *content = findMimeNodeForIndex( aMsg.get(), index );
        if ( content ) {
          KPIMUtils::kByteArrayToFile( content->decodedContent(), tempFileName,
                            false, false, false );
        }
      }
      tf->close();
    }
    // QString( "%0 and %1 and %1" ).arg( 0 ).arg( 1 )
    // returns "0 and 1 and %1", so we must call .arg as
    // many times as there are %n's, regardless of their multiplicity.
    if ((*it) == -1) result.replace( "%-1", tempFileName );
    else result = result.arg( tempFileName );
  }

  // And finally, replace the %{foo} with the content of the foo
  // header field:
  QRegExp header_rx( "%\\{([a-z0-9-]+)\\}", Qt::CaseInsensitive );
  int idx = 0;
  while ( ( idx = header_rx.indexIn( result, idx ) ) != -1 ) {
    QString replacement = KShell::quoteArg( aMsg->headerByType( header_rx.cap(1).toLatin1() ) ? aMsg->headerByType( header_rx.cap(1).toLatin1() )->as7BitString(): "");
    result.replace( idx, header_rx.matchedLength(), replacement );
    idx += replacement.length();
  }

  return result;
}


FilterAction::ReturnCode FilterActionWithCommand::genericProcess( const Akonadi::Item &item, bool withOutput ) const
{
  const KMime::Message::Ptr aMsg = item.payload<KMime::Message::Ptr>();
  Q_ASSERT( aMsg );

  if ( mParameter.isEmpty() )
    return ErrorButGoOn;

  // KProcess doesn't support a QProcess::launch() equivalent, so
  // we must use a temp file :-(
  KTemporaryFile * inFile = new KTemporaryFile;
  if ( !inFile->open() )
    return ErrorButGoOn;

  QList<KTemporaryFile*> atmList;
  atmList.append( inFile );

  QString commandLine = substituteCommandLineArgsFor( aMsg, atmList );
  if ( commandLine.isEmpty() )
  {
    qDeleteAll( atmList );
    atmList.clear();
    return ErrorButGoOn;
  }
  // The parentheses force the creation of a subshell
  // in which the user-specified command is executed.
  // This is to really catch all output of the command as well
  // as to avoid clashes of our redirection with the ones
  // the user may have specified. In the long run, we
  // shouldn't be using tempfiles at all for this class, due
  // to security aspects. (mmutz)
  commandLine =  '(' + commandLine + ") <" + inFile->fileName();

  // write message to file
  QString tempFileName = inFile->fileName();
  if ( !KPIMUtils::kByteArrayToFile( aMsg->encodedContent(), tempFileName, //###
                                     false, false, false ) ) {
    qDeleteAll( atmList );
    atmList.clear();
    return CriticalError;
  }
  inFile->close();

  KProcess shProc;
  shProc.setOutputChannelMode( KProcess::SeparateChannels );
  shProc.setShellCommand( commandLine );
  int result = shProc.execute();

  if ( result != 0 ) {
    qDeleteAll( atmList );
    atmList.clear();
    return ErrorButGoOn;
  }

  if ( withOutput ) {
    // read altered message:
    QByteArray msgText = shProc.readAllStandardOutput();

    if ( !msgText.isEmpty() ) {
    /* If the pipe through alters the message, it could very well
       happen that it no longer has a X-UID header afterwards. That is
       unfortunate, as we need to removed the original from the folder
       using that, and look it up in the message. When the (new) message
       is uploaded, the header is stripped anyhow. */
      QString uid = aMsg->headerByType( "X-UID" ) ?aMsg->headerByType( "X-UID")->asUnicodeString() : "" ;
      aMsg->setContent( msgText );

      KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-UID", aMsg.get(), uid, "utf-8" );
      aMsg->setHeader( header );
    }
    else {
      qDeleteAll( atmList );
      atmList.clear();
      return ErrorButGoOn;
    }
  }
  qDeleteAll( atmList );
  atmList.clear();
  return GoOn;
}


//=============================================================================
//
//   Specific  Filter  Actions
//
//=============================================================================

//=============================================================================
// FilterActionSendReceipt - send receipt
// Return delivery receipt.
//=============================================================================
class FilterActionSendReceipt : public FilterActionWithNone
{
public:
  FilterActionSendReceipt();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  static FilterAction* newAction(void);
};

FilterAction* FilterActionSendReceipt::newAction(void)
{
  return (new FilterActionSendReceipt);
}

FilterActionSendReceipt::FilterActionSendReceipt()
  : FilterActionWithNone( "confirm delivery", i18n("Confirm Delivery") )
{
}

FilterAction::ReturnCode FilterActionSendReceipt::process( const Akonadi::Item& item ) const
{
  const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
  MessageFactory factory( msg, item.id() );
  factory.setFolderIdentity( Util::folderIdentity( item ) );
  factory.setIdentityManager( KernelIf->identityManager() );
  const KMime::Message::Ptr receipt = factory.createDeliveryReceipt();
  if ( !receipt ) return ErrorButGoOn;

  // Queue message. This is a) so that the user can check
  // the receipt before sending and b) for speed reasons.
  KernelIf->msgSender()->send( receipt, MessageSender::SendLater );

  return GoOn;
}



//=============================================================================
// FilterActionSetTransport - set transport to...
// Specify mail transport (smtp server) to be used when replying to a message
// TODO: use TransportComboBox so the user does not enter an invalid transport
//=============================================================================
class FilterActionTransport: public FilterActionWithString
{
public:
  FilterActionTransport();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  static FilterAction* newAction(void);
};

FilterAction* FilterActionTransport::newAction(void)
{
  return (new FilterActionTransport);
}

FilterActionTransport::FilterActionTransport()
  : FilterActionWithString( "set transport", i18n("Set Transport To") )
{
}

FilterAction::ReturnCode FilterActionTransport::process( const Akonadi::Item& item ) const
{
  if ( mParameter.isEmpty() )
    return ErrorButGoOn;
  const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
  KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Transport", msg.get(), mParameter, "utf-8");
  msg->setHeader( header );
  msg->assemble();

  new Akonadi::ItemModifyJob( item, FilterIf->filterManager() );
  return GoOn;
}


//=============================================================================
// FilterActionReplyTo - set Reply-To to
// Set the Reply-to header in a message
//=============================================================================
class FilterActionReplyTo: public FilterActionWithString
{
public:
  FilterActionReplyTo();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  static FilterAction* newAction(void);
};

FilterAction* FilterActionReplyTo::newAction(void)
{
  return (new FilterActionReplyTo);
}

FilterActionReplyTo::FilterActionReplyTo()
  : FilterActionWithString( "set Reply-To", i18n("Set Reply-To To") )
{
  mParameter = "";
}

FilterAction::ReturnCode FilterActionReplyTo::process( const Akonadi::Item& item ) const
{
  const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
  KMime::Headers::Generic *header = new KMime::Headers::Generic( "Reply-To", msg.get(), mParameter, "utf-8");
  msg->setHeader( header );
  msg->assemble();

  new Akonadi::ItemModifyJob( item, FilterIf->filterManager() );
  return GoOn;
}



//=============================================================================
// FilterActionIdentity - set identity to
// Specify Identity to be used when replying to a message
//=============================================================================
class FilterActionIdentity: public FilterActionWithUOID
{
public:
  FilterActionIdentity();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  static FilterAction* newAction();

  QWidget * createParamWidget( QWidget * parent ) const;
  void applyParamWidgetValue( QWidget * parent );
  void setParamWidgetValue( QWidget * parent ) const;
  void clearParamWidget( QWidget * param ) const;
};

FilterAction* FilterActionIdentity::newAction()
{
  return (new FilterActionIdentity);
}

FilterActionIdentity::FilterActionIdentity()
  : FilterActionWithUOID( "set identity", i18n("Set Identity To") )
{
  mParameter = KernelIf->identityManager()->defaultIdentity().uoid();
}

FilterAction::ReturnCode FilterActionIdentity::process( const Akonadi::Item& item ) const
{
  const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
  KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-KMail-Identity", msg.get(), QString::number(mParameter), "utf-8");
  msg->setHeader( header );
  msg->assemble();

  new Akonadi::ItemModifyJob( item, FilterIf->filterManager() );
  return GoOn;
}

QWidget * FilterActionIdentity::createParamWidget( QWidget * parent ) const
{
  KPIMIdentities::IdentityCombo * ic = new KPIMIdentities::IdentityCombo( KernelIf->identityManager(), parent );
  ic->setCurrentIdentity( mParameter );
  return ic;
}

void FilterActionIdentity::applyParamWidgetValue( QWidget * paramWidget )
{
  KPIMIdentities::IdentityCombo * ic = dynamic_cast<KPIMIdentities::IdentityCombo*>( paramWidget );
  assert( ic );
  mParameter = ic->currentIdentity();
}

void FilterActionIdentity::clearParamWidget( QWidget * paramWidget ) const
{
  KPIMIdentities::IdentityCombo * ic = dynamic_cast<KPIMIdentities::IdentityCombo*>( paramWidget );
  assert( ic );
  ic->setCurrentIndex( 0 );
  //ic->setCurrentIdentity( kmkernel->identityManager()->defaultIdentity() );
}

void FilterActionIdentity::setParamWidgetValue( QWidget * paramWidget ) const
{
  KPIMIdentities::IdentityCombo * ic = dynamic_cast<KPIMIdentities::IdentityCombo*>( paramWidget );
  assert( ic );
  ic->setCurrentIdentity( mParameter );
}

//=============================================================================
// FilterActionSetStatus - set status to
// Set the status of messages
//=============================================================================
class FilterActionSetStatus: public FilterActionWithStringList
{
public:
  FilterActionSetStatus();
  virtual ReturnCode process( const Akonadi::Item& item ) const;
  virtual bool requiresBody() const;

  static FilterAction* newAction();

  virtual bool isEmpty() const { return false; }

  virtual void argsFromString( const QString &argsStr );
  virtual const QString argsAsString() const;
  virtual const QString displayString() const;
};


static const Akonadi::MessageStatus stati[] =
{
  Akonadi::MessageStatus::statusImportant(),
  Akonadi::MessageStatus::statusRead(),
  Akonadi::MessageStatus::statusUnread(),
  Akonadi::MessageStatus::statusReplied(),
  Akonadi::MessageStatus::statusForwarded(),
  Akonadi::MessageStatus::statusWatched(),
  Akonadi::MessageStatus::statusIgnored(),
  Akonadi::MessageStatus::statusSpam(),
  Akonadi::MessageStatus::statusHam(),
  Akonadi::MessageStatus::statusToAct()
};
static const int StatiCount = sizeof( stati ) / sizeof( Akonadi::MessageStatus );

FilterAction* FilterActionSetStatus::newAction()
{
  return (new FilterActionSetStatus);
}

FilterActionSetStatus::FilterActionSetStatus()
  : FilterActionWithStringList( "set status", i18n("Mark As") )
{
  // if you change this list, also update
  // FilterActionSetStatus::stati above
  mParameterList.append( "" );
  mParameterList.append( i18nc("msg status","Important") );
  mParameterList.append( i18nc("msg status","Read") );
  mParameterList.append( i18nc("msg status","Unread") );
  mParameterList.append( i18nc("msg status","Replied") );
  mParameterList.append( i18nc("msg status","Forwarded") );
  mParameterList.append( i18nc("msg status","Watched") );
  mParameterList.append( i18nc("msg status","Ignored") );
  mParameterList.append( i18nc("msg status","Spam") );
  mParameterList.append( i18nc("msg status","Ham") );
  mParameterList.append( i18nc("msg status","Action Item") );

  mParameter = mParameterList.at(0);
}

FilterAction::ReturnCode FilterActionSetStatus::process( const Akonadi::Item& item ) const
{
  const int idx = mParameterList.indexOf( mParameter );
  if ( idx < 1 ) return ErrorButGoOn;

  Akonadi::MessageStatus status;
  status.setStatusFromFlags( item.flags() );
  status.set( stati[ idx - 1 ] );
  Akonadi::Item i( item.id() );
  i.setRevision( item.revision() );
  i.setFlags( status.statusFlags() );
  new Akonadi::ItemModifyJob( i, FilterIf->filterManager() ); // TODO handle error
  return GoOn;
}

bool FilterActionSetStatus::requiresBody() const
{
  return false;
}

void FilterActionSetStatus::argsFromString( const QString &argsStr )
{
  if ( argsStr.length() == 1 ) {
    Akonadi::MessageStatus status;
    int i;
    for ( i = 0 ; i < StatiCount ; ++i )
    {
      status = stati[i];
      if ( status.statusStr()[0] == argsStr[0].toLatin1() ) {
        mParameter = mParameterList.at(i+1);
        return;
      }
    }
  }
  mParameter = mParameterList.at(0);
}

const QString FilterActionSetStatus::argsAsString() const
{
  const int idx = mParameterList.indexOf( mParameter );
  if ( idx < 1 ) return QString();

  return stati[idx-1].statusStr();
}

const QString FilterActionSetStatus::displayString() const
{
  return label() + " \"" + Qt::escape( argsAsString() ) + "\"";
}

//=============================================================================
// FilterActionAddTag - append tag to message
// Appends a tag to messages
//=============================================================================
class FilterActionAddTag: public FilterActionWithStringList
{
public:
  FilterActionAddTag();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  virtual bool requiresBody() const;

  static FilterAction* newAction();

  virtual bool isEmpty() const { return false; }

  virtual void argsFromString( const QString &argsStr );
  virtual const QString argsAsString() const;
  virtual const QString displayString() const;

private:
  QStringList mLabelList;
};

FilterAction* FilterActionAddTag::newAction()
{
  return (new FilterActionAddTag);
}

FilterActionAddTag::FilterActionAddTag()
  : FilterActionWithStringList( "add tag", i18n("Add Tag") )
{
  foreach( const Nepomuk::Tag &tag, Nepomuk::Tag::allTags() ) {
    mParameterList.append( tag.label() );
    mLabelList.append( tag.resourceUri().toString() );
  }
}

FilterAction::ReturnCode FilterActionAddTag::process( const Akonadi::Item & item ) const
{
  const int idx = mParameterList.indexOf( mParameter );
  if ( idx == -1 ) return ErrorButGoOn;

  Nepomuk::Resource n_resource( item.url() );
  n_resource.addTag( mParameter );
  return GoOn;
}

bool FilterActionAddTag::requiresBody() const
{
  return false;
}

void FilterActionAddTag::argsFromString( const QString &argsStr )
{
  foreach ( const QString& tag, mParameterList ) {
    if ( tag == argsStr ) {
      mParameter = tag;
      return;
    }
  }
  if ( mParameterList.size() > 0 )
    mParameter = mParameterList.at( 0 );
}

const QString FilterActionAddTag::argsAsString() const
{
  const int idx = mParameterList.indexOf( mParameter );
  if ( idx == -1 ) return QString();

  return mParameterList.at( idx );
}

const QString FilterActionAddTag::displayString() const
{
  return label() + " \"" + Qt::escape( argsAsString() ) + "\"";
}

//=============================================================================
// FilterActionFakeDisposition - send fake MDN
// Sends a fake MDN or forces an ignore.
//=============================================================================
class FilterActionFakeDisposition: public FilterActionWithStringList
{
public:
  FilterActionFakeDisposition();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  static FilterAction* newAction() {
    return (new FilterActionFakeDisposition);
  }

  virtual bool isEmpty() const { return false; }

  virtual void argsFromString( const QString &argsStr );
  virtual const QString argsAsString() const;
  virtual const QString displayString() const;
};


// if you change this list, also update
// the count in argsFromString
static const KMime::MDN::DispositionType mdns[] =
{
  KMime::MDN::Displayed,
  KMime::MDN::Deleted,
  KMime::MDN::Dispatched,
  KMime::MDN::Processed,
  KMime::MDN::Denied,
  KMime::MDN::Failed,
};
static const int numMDNs = sizeof mdns / sizeof *mdns;


FilterActionFakeDisposition::FilterActionFakeDisposition()
  : FilterActionWithStringList( "fake mdn", i18n("Send Fake MDN") )
{
  // if you change this list, also update
  // mdns above
  mParameterList.append( "" );
  mParameterList.append( i18nc("MDN type","Ignore") );
  mParameterList.append( i18nc("MDN type","Displayed") );
  mParameterList.append( i18nc("MDN type","Deleted") );
  mParameterList.append( i18nc("MDN type","Dispatched") );
  mParameterList.append( i18nc("MDN type","Processed") );
  mParameterList.append( i18nc("MDN type","Denied") );
  mParameterList.append( i18nc("MDN type","Failed") );

  mParameter = mParameterList.at(0);
}

FilterAction::ReturnCode FilterActionFakeDisposition::process( const Akonadi::Item &item ) const
{
  const int idx = mParameterList.indexOf( mParameter );
  if ( idx < 1 ) return ErrorButGoOn;

  if ( idx == 1 ) { // ignore
    if( item.hasAttribute< Akonadi::MDNStateAttribute >() ) {
      item.attribute< Akonadi::MDNStateAttribute >()->setMDNState( Akonadi::MDNStateAttribute::MDNIgnore );
      Akonadi::ItemModifyJob* modifyJob = new Akonadi::ItemModifyJob( item );
      modifyJob->setIgnorePayload( true );
    }
  } else // send
    sendMDN( item, mdns[idx-2] ); // skip first two entries: "" and "ignore"
  return GoOn;
}

void FilterActionFakeDisposition::argsFromString( const QString &argsStr )
{
  if ( argsStr.length() == 1 ) {
    if ( argsStr[0] == 'I' ) { // ignore
      mParameter = mParameterList.at(1);
      return;
    }
    for ( int i = 0 ; i < numMDNs ; i++ )
      if ( char(mdns[i]) == argsStr[0] ) { // send
        mParameter = mParameterList.at(i+2);
        return;
      }
  }
  mParameter = mParameterList.at(0);
}

const QString FilterActionFakeDisposition::argsAsString() const
{
  const int idx = mParameterList.indexOf( mParameter );
  if ( idx < 1 ) return QString();

  return QString( QChar( idx < 2 ? 'I' : char(mdns[idx-2]) ) );
}

const QString FilterActionFakeDisposition::displayString() const
{
  return label() + " \"" + Qt::escape( argsAsString() ) + "\"";
}

//=============================================================================
// FilterActionRemoveHeader - remove header
// Remove all instances of the given header field.
//=============================================================================
class FilterActionRemoveHeader: public FilterActionWithStringList
{
public:
  FilterActionRemoveHeader();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  virtual QWidget* createParamWidget( QWidget* parent ) const;
  virtual void setParamWidgetValue( QWidget* paramWidget ) const;

  static FilterAction* newAction();
};

FilterAction* FilterActionRemoveHeader::newAction()
{
  return (new FilterActionRemoveHeader);
}

FilterActionRemoveHeader::FilterActionRemoveHeader()
  : FilterActionWithStringList( "remove header", i18n("Remove Header") )
{
  mParameterList << ""
                 << "Reply-To"
                 << "Delivered-To"
                 << "X-KDE-PR-Message"
                 << "X-KDE-PR-Package"
                 << "X-KDE-PR-Keywords";
  mParameter = mParameterList.at(0);
}

QWidget* FilterActionRemoveHeader::createParamWidget( QWidget* parent ) const
{
  KComboBox *cb = new KComboBox( parent );
  cb->setEditable( true );
  cb->setInsertPolicy( QComboBox::InsertAtBottom );
  setParamWidgetValue( cb );
  return cb;
}

FilterAction::ReturnCode FilterActionRemoveHeader::process( const Akonadi::Item& item ) const
{
  if ( mParameter.isEmpty() ) return ErrorButGoOn;

  KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
  while ( msg->headerByType( mParameter.toLatin1() ) )
    msg->removeHeader( mParameter.toLatin1() );
  msg->assemble();
  new Akonadi::ItemModifyJob( item, FilterIf->filterManager() );
  return GoOn;
}

void FilterActionRemoveHeader::setParamWidgetValue( QWidget* paramWidget ) const
{
  KComboBox * cb = dynamic_cast<KComboBox*>(paramWidget);
  Q_ASSERT( cb );

  const int idx = mParameterList.indexOf( mParameter );
  cb->clear();
  cb->addItems( mParameterList );
  if ( idx < 0 ) {
    cb->addItem( mParameter );
    cb->setCurrentIndex( cb->count() - 1 );
  } else {
    cb->setCurrentIndex( idx );
  }
}


//=============================================================================
// FilterActionAddHeader - add header
// Add a header with the given value.
//=============================================================================
class FilterActionAddHeader: public FilterActionWithStringList
{
public:
  FilterActionAddHeader();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  virtual QWidget* createParamWidget( QWidget* parent ) const;
  virtual void setParamWidgetValue( QWidget* paramWidget ) const;
  virtual void applyParamWidgetValue( QWidget* paramWidget );
  virtual void clearParamWidget( QWidget* paramWidget ) const;

  virtual const QString argsAsString() const;
  virtual void argsFromString( const QString &argsStr );

  virtual const QString displayString() const;

  static FilterAction* newAction()
  {
    return (new FilterActionAddHeader);
  }
private:
  QString mValue;
};

FilterActionAddHeader::FilterActionAddHeader()
  : FilterActionWithStringList( "add header", i18n("Add Header") )
{
  mParameterList << ""
                 << "Reply-To"
                 << "Delivered-To"
                 << "X-KDE-PR-Message"
                 << "X-KDE-PR-Package"
                 << "X-KDE-PR-Keywords";
  mParameter = mParameterList.at(0);
}

FilterAction::ReturnCode FilterActionAddHeader::process( const Akonadi::Item& item ) const
{
  if ( mParameter.isEmpty() ) return ErrorButGoOn;

  KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
  KMime::Headers::Generic *header = new KMime::Headers::Generic( mParameter.toLatin1(), msg.get(), mValue, "utf-8" );
  msg->setHeader( header );
  msg->assemble();
  new Akonadi::ItemModifyJob( item, FilterIf->filterManager() );
  return GoOn;
}

QWidget* FilterActionAddHeader::createParamWidget( QWidget* parent ) const
{
  QWidget *w = new QWidget( parent );
  QHBoxLayout *hbl = new QHBoxLayout( w );
  hbl->setSpacing( 4 );
  hbl->setMargin( 0 );
  KComboBox *cb = new KComboBox( w );
  cb->setObjectName( "combo" );
  cb->setEditable( true );
  cb->setInsertPolicy( QComboBox::InsertAtBottom );
  hbl->addWidget( cb, 0 /* stretch */ );
  QLabel *l = new QLabel( i18n("With value:"), w );
  l->setFixedWidth( l->sizeHint().width() );
  hbl->addWidget( l, 0 );
  KLineEdit *le = new KLineEdit( w );
  le->setObjectName( "ledit" );
  le->setClearButtonShown( true );
  hbl->addWidget( le, 1 );
  setParamWidgetValue( w );
  return w;
}

void FilterActionAddHeader::setParamWidgetValue( QWidget* paramWidget ) const
{
  const int idx = mParameterList.indexOf( mParameter );
  KComboBox *cb = paramWidget->findChild<KComboBox*>("combo");
  Q_ASSERT( cb );
  cb->clear();
  cb->addItems( mParameterList );
  if ( idx < 0 ) {
    cb->addItem( mParameter );
    cb->setCurrentIndex( cb->count() - 1 );
  } else {
    cb->setCurrentIndex( idx );
  }
  KLineEdit *le = paramWidget->findChild<KLineEdit*>("ledit");
  Q_ASSERT( le );
  le->setText( mValue );
}

void FilterActionAddHeader::applyParamWidgetValue( QWidget* paramWidget )
{
  KComboBox *cb = paramWidget->findChild<KComboBox*>("combo");
  Q_ASSERT( cb );
  mParameter = cb->currentText();

  KLineEdit *le = paramWidget->findChild<KLineEdit*>("ledit");
  Q_ASSERT( le );
  mValue = le->text();
}

void FilterActionAddHeader::clearParamWidget( QWidget* paramWidget ) const
{
  KComboBox *cb = paramWidget->findChild<KComboBox*>("combo");
  Q_ASSERT( cb );
  cb->setCurrentIndex(0);
  KLineEdit *le = paramWidget->findChild<KLineEdit*>("ledit");
  Q_ASSERT( le );
  le->clear();
}

const QString FilterActionAddHeader::argsAsString() const
{
  QString result = mParameter;
  result += '\t';
  result += mValue;

  return result;
}

const QString FilterActionAddHeader::displayString() const
{
  return label() + " \"" + Qt::escape( argsAsString() ) + "\"";
}

void FilterActionAddHeader::argsFromString( const QString &argsStr )
{
  const QStringList l = argsStr.split( '\t' );
  QString s;
  if ( l.count() < 2 ) {
    s = l[0];
    mValue = "";
  } else {
    s = l[0];
    mValue = l[1];
  }

  int idx = mParameterList.indexOf( s );
  if ( idx < 0 ) {
    mParameterList.append( s );
    idx = mParameterList.count() - 1;
  }
  mParameter = mParameterList.at( idx );
}


//=============================================================================
// FilterActionRewriteHeader - rewrite header
// Rewrite a header using a regexp.
//=============================================================================
class FilterActionRewriteHeader: public FilterActionWithStringList
{
public:
  FilterActionRewriteHeader();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  virtual QWidget* createParamWidget( QWidget* parent ) const;
  virtual void setParamWidgetValue( QWidget* paramWidget ) const;
  virtual void applyParamWidgetValue( QWidget* paramWidget );
  virtual void clearParamWidget( QWidget* paramWidget ) const;

  virtual const QString argsAsString() const;
  virtual void argsFromString( const QString &argsStr );

  virtual const QString displayString() const;

  static FilterAction* newAction()
  {
    return (new FilterActionRewriteHeader);
  }
private:
  QRegExp mRegExp;
  QString mReplacementString;
};

FilterActionRewriteHeader::FilterActionRewriteHeader()
  : FilterActionWithStringList( "rewrite header", i18n("Rewrite Header") )
{
  mParameterList << ""
                 << "Subject"
                 << "Reply-To"
                 << "Delivered-To"
                 << "X-KDE-PR-Message"
                 << "X-KDE-PR-Package"
                 << "X-KDE-PR-Keywords";
  mParameter = mParameterList.at(0);
}

FilterAction::ReturnCode FilterActionRewriteHeader::process( const Akonadi::Item & item ) const
{
  if ( mParameter.isEmpty() || !mRegExp.isValid() )
    return ErrorButGoOn;

  const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
  // QString::replace is not const.
  QString value = msg->headerByType( mParameter.toLatin1() ) ? msg->headerByType( mParameter.toLatin1() )->asUnicodeString(): "";

  KMime::Headers::Generic *header = new KMime::Headers::Generic( mParameter.toLatin1(), msg.get(), value.replace( mRegExp, mReplacementString ), "utf-8" );
  msg->setHeader( header );
  msg->assemble();

  new Akonadi::ItemModifyJob( item, FilterIf->filterManager() );
  return GoOn;
}

QWidget* FilterActionRewriteHeader::createParamWidget( QWidget* parent ) const
{
  QWidget *w = new QWidget( parent );
  QHBoxLayout *hbl = new QHBoxLayout( w );
  hbl->setSpacing( 4 );
  hbl->setMargin( 0 );

  KComboBox *cb = new KComboBox( w );
  cb->setEditable( true );
  cb->setObjectName( "combo" );
  cb->setInsertPolicy( QComboBox::InsertAtBottom );
  hbl->addWidget( cb, 0 /* stretch */ );

  QLabel *l = new QLabel( i18n("Replace:"), w );
  l->setFixedWidth( l->sizeHint().width() );
  hbl->addWidget( l, 0 );

  RegExpLineEdit *rele = new RegExpLineEdit( w );
  rele->setObjectName( "search" );
  hbl->addWidget( rele, 1 );

  l = new QLabel( i18n("With:"), w );
  l->setFixedWidth( l->sizeHint().width() );
  hbl->addWidget( l, 0 );

  KLineEdit *le = new KLineEdit( w );
  le->setObjectName( "replace" );
  le->setClearButtonShown( true );
  hbl->addWidget( le, 1 );

  setParamWidgetValue( w );
  return w;
}

void FilterActionRewriteHeader::setParamWidgetValue( QWidget* paramWidget ) const
{
  const int idx = mParameterList.indexOf( mParameter );
  KComboBox *cb = paramWidget->findChild<KComboBox*>("combo");
  Q_ASSERT( cb );

  cb->clear();
  cb->addItems( mParameterList );
  if ( idx < 0 ) {
    cb->addItem( mParameter );
    cb->setCurrentIndex( cb->count() - 1 );
  } else {
    cb->setCurrentIndex( idx );
  }

  RegExpLineEdit *rele = paramWidget->findChild<RegExpLineEdit*>("search");
  Q_ASSERT( rele );
  rele->setText( mRegExp.pattern() );

  KLineEdit *le = paramWidget->findChild<KLineEdit*>("replace");
  Q_ASSERT( le );
  le->setText( mReplacementString );
}

void FilterActionRewriteHeader::applyParamWidgetValue( QWidget* paramWidget )
{
  KComboBox *cb = paramWidget->findChild<KComboBox*>("combo");
  Q_ASSERT( cb );
  mParameter = cb->currentText();

  RegExpLineEdit *rele = paramWidget->findChild<RegExpLineEdit*>("search");
  Q_ASSERT( rele );
  mRegExp.setPattern( rele->text() );

  KLineEdit *le = paramWidget->findChild<KLineEdit*>("replace");
  Q_ASSERT( le );
  mReplacementString = le->text();
}

void FilterActionRewriteHeader::clearParamWidget( QWidget* paramWidget ) const
{
  KComboBox *cb = paramWidget->findChild<KComboBox*>("combo");
  Q_ASSERT( cb );
  cb->setCurrentIndex(0);

  RegExpLineEdit *rele = paramWidget->findChild<RegExpLineEdit*>("search");
  Q_ASSERT( rele );
  rele->clear();

  KLineEdit *le = paramWidget->findChild<KLineEdit*>("replace");
  Q_ASSERT( le );
  le->clear();
}

const QString FilterActionRewriteHeader::argsAsString() const
{
  QString result = mParameter;
  result += '\t';
  result += mRegExp.pattern();
  result += '\t';
  result += mReplacementString;

  return result;
}

const QString FilterActionRewriteHeader::displayString() const
{
  return label() + " \"" + Qt::escape( argsAsString() ) + "\"";
}

void FilterActionRewriteHeader::argsFromString( const QString &argsStr )
{
  const QStringList l = argsStr.split( '\t' );
  QString s;

  s = l[0];
  mRegExp.setPattern( l[1] );
  mReplacementString = l[2];

  int idx = mParameterList.indexOf( s );
  if ( idx < 0 ) {
    mParameterList.append( s );
    idx = mParameterList.count() - 1;
  }
  mParameter = mParameterList.at( idx );
}


//=============================================================================
// FilterActionMove - move into folder
// File message into another mail folder
//=============================================================================
class FilterActionMove: public FilterActionWithFolder
{
public:
  FilterActionMove();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  virtual bool requiresBody() const;
  static FilterAction* newAction(void);
};

FilterAction* FilterActionMove::newAction(void)
{
  return (new FilterActionMove);
}

FilterActionMove::FilterActionMove()
  : FilterActionWithFolder( "transfer", i18n("Move Into Folder") )
{
}

FilterAction::ReturnCode FilterActionMove::process( const Akonadi::Item & item ) const
{
  if ( !mFolder.isValid() ) {
    const Akonadi::Collection targetFolder = CommonKernel->collectionFromId( mFolderName );
    if( !targetFolder.isValid() )
      return ErrorButGoOn;
    MessageProperty::setFilterFolder( item, targetFolder );
    return GoOn;
  }
  MessageProperty::setFilterFolder( item, mFolder );
  return GoOn;
}

bool FilterActionMove::requiresBody() const
{
    return false;
}


//=============================================================================
// FilterActionCopy - copy into folder
// Copy message into another mail folder
//=============================================================================
class FilterActionCopy: public FilterActionWithFolder
{
public:
  FilterActionCopy();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  virtual bool requiresBody() const;
  static FilterAction* newAction(void);
};

FilterAction* FilterActionCopy::newAction( void )
{
  return ( new FilterActionCopy );
}

FilterActionCopy::FilterActionCopy()
  : FilterActionWithFolder( "copy", i18n("Copy Into Folder") )
{
}

FilterAction::ReturnCode FilterActionCopy::process( const Akonadi::Item & item ) const
{
  // copy the message 1:1
  new Akonadi::ItemCopyJob( item, mFolder, FilterIf->filterManager() ); // TODO handle error
  return GoOn;
}

bool FilterActionCopy::requiresBody() const
{
  return false;
}

//=============================================================================
// FilterActionForward - forward to
// Forward message to another user, with a defined template
//=============================================================================
class FilterActionForward: public FilterActionWithAddress
{
  public:
    FilterActionForward();
    static FilterAction* newAction( void );
    virtual ReturnCode process( const Akonadi::Item & item ) const;
    virtual QWidget* createParamWidget( QWidget* parent ) const;
    virtual void applyParamWidgetValue( QWidget* paramWidget );
    virtual void setParamWidgetValue( QWidget* paramWidget ) const;
    virtual void clearParamWidget( QWidget* paramWidget ) const;
    virtual void argsFromString( const QString &argsStr );
    virtual const QString argsAsString() const;
    virtual const QString displayString() const;

private:

  mutable QString mTemplate;
};

FilterAction *FilterActionForward::newAction( void )
{
  return ( new FilterActionForward );
}

FilterActionForward::FilterActionForward()
  : FilterActionWithAddress( "forward", i18n("Forward To") )
{
}

FilterAction::ReturnCode FilterActionForward::process( const Akonadi::Item & item ) const
{
  if ( mParameter.isEmpty() )
    return ErrorButGoOn;

  const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
  // avoid endless loops when this action is used in a filter
  // which applies to sent messages
  if ( MessageCore::StringUtil::addressIsInAddressList( mParameter,
                                                   QStringList( msg->to()->asUnicodeString() ) ) ) {
    kWarning() << "Attempt to forward to receipient of original message, ignoring.";
    return ErrorButGoOn;
  }
  MessageFactory factory( msg, item.id() );
  factory.setIdentityManager( KernelIf->identityManager() );
  factory.setFolderIdentity( Util::folderIdentity( item ) );
  factory.setTemplate( mTemplate );
  KMime::Message::Ptr fwdMsg = factory.createForward();
  fwdMsg->to()->fromUnicodeString( fwdMsg->to()->asUnicodeString() + ',' + mParameter, "utf-8" );
  if ( !KernelIf->msgSender()->send( fwdMsg, MessageSender::SendDefault ) ) {
    kWarning() << "FilterAction: could not forward message (sending failed)";
    return ErrorButGoOn; // error: couldn't send
  }
  else
    sendMDN( item, KMime::MDN::Dispatched );

  // (the msgSender takes ownership of the message, so don't delete it here)
  return GoOn;
}

QWidget* FilterActionForward::createParamWidget( QWidget* parent ) const
{
  QWidget *addressAndTemplate = new QWidget( parent );
  QHBoxLayout *hBox = new QHBoxLayout( addressAndTemplate );
  hBox->setMargin( 0 );
  QWidget *addressEdit = FilterActionWithAddress::createParamWidget( addressAndTemplate );
  addressEdit->setObjectName( "addressEdit" );
  hBox->addWidget( addressEdit );

  KLineEdit *lineEdit = addressEdit->findChild<KLineEdit*>( "addressEdit" );
  Q_ASSERT( lineEdit );
  lineEdit->setToolTip( i18n( "The addressee to whom the message will be forwarded." ) );
  lineEdit->setWhatsThis( i18n( "The filter will forward the message to the addressee entered here." ) );

  KComboBox *templateCombo = new KComboBox( addressAndTemplate );
  templateCombo->setObjectName( "templateCombo" );
  hBox->addWidget( templateCombo );

  templateCombo->addItem( i18n( "Default Template" ) );
  QStringList templateNames = SettingsIf->customTemplates();
  foreach( const QString &templateName, templateNames ) {
    CTemplates templat( templateName );
    if ( templat.type() == CustomTemplates::TForward ||
         templat.type() == CustomTemplates::TUniversal )
      templateCombo->addItem( templateName );
  }
  templateCombo->setEnabled( templateCombo->count() > 1 );
  templateCombo->setToolTip( i18n( "The template used when forwarding" ) );
  templateCombo->setWhatsThis( i18n( "Set the forwarding template that will be used with this filter." ) );

  return addressAndTemplate;
}

void FilterActionForward::applyParamWidgetValue( QWidget* paramWidget )
{
  QWidget *addressEdit = paramWidget->findChild<QWidget*>( "addressEdit" );
  Q_ASSERT( addressEdit );
  FilterActionWithAddress::applyParamWidgetValue( addressEdit );

  KComboBox *templateCombo = paramWidget->findChild<KComboBox*>( "templateCombo" );
  Q_ASSERT( templateCombo );

  if ( templateCombo->currentIndex() == 0 ) {
    // Default template, so don't use a custom one
    mTemplate.clear();
  }
  else {
    mTemplate = templateCombo->currentText();
  }
}

void FilterActionForward::setParamWidgetValue( QWidget* paramWidget ) const
{
  QWidget *addressEdit = paramWidget->findChild<QWidget*>( "addressEdit" );
  Q_ASSERT( addressEdit );
  FilterActionWithAddress::setParamWidgetValue( addressEdit );

  KComboBox *templateCombo = paramWidget->findChild<KComboBox*>( "templateCombo" );
  Q_ASSERT( templateCombo );

  if ( mTemplate.isEmpty() ) {
    templateCombo->setCurrentIndex( 0 );
  }
  else {
    int templateIndex = templateCombo->findText( mTemplate );
    if ( templateIndex != -1 ) {
      templateCombo->setCurrentIndex( templateIndex );
    }
    else {
      mTemplate.clear();
    }
  }
}

void FilterActionForward::clearParamWidget( QWidget* paramWidget ) const
{
  QWidget *addressEdit = paramWidget->findChild<QWidget*>( "addressEdit" );
  Q_ASSERT( addressEdit );
  FilterActionWithAddress::clearParamWidget( addressEdit );

  KComboBox *templateCombo = paramWidget->findChild<KComboBox*>( "templateCombo" );
  Q_ASSERT( templateCombo );

  templateCombo->setCurrentIndex( 0 );
}

// We simply place a "@$$@" between the two parameters. The template is the last
// parameter in the string, for compatibility reasons.
static const QString forwardFilterArgsSeperator = "@$$@";

void FilterActionForward::argsFromString( const QString &argsStr )
{
  int seperatorPos = argsStr.indexOf( forwardFilterArgsSeperator );

  if ( seperatorPos == - 1 ) {
    // Old config, assume that the whole string is the addressee
    FilterActionWithAddress::argsFromString( argsStr );
  }
  else {
    const QString addressee = argsStr.left( seperatorPos );
    mTemplate = argsStr.mid( seperatorPos + forwardFilterArgsSeperator.length() );
    FilterActionWithAddress::argsFromString( addressee );
  }
}

const QString FilterActionForward::argsAsString() const
{
  return FilterActionWithAddress::argsAsString() + forwardFilterArgsSeperator + mTemplate;
}

const QString FilterActionForward::displayString() const
{
  if ( mTemplate.isEmpty() )
    return i18n( "Forward to %1 with default template", mParameter );
  else
    return i18n( "Forward to %1 with template %2", mParameter, mTemplate );
}

//=============================================================================
// FilterActionRedirect - redirect to
// Redirect message to another user
//=============================================================================
class FilterActionRedirect: public FilterActionWithAddress
{
public:
  FilterActionRedirect();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  static FilterAction* newAction(void);
};

FilterAction* FilterActionRedirect::newAction(void)
{
  return (new FilterActionRedirect);
}

FilterActionRedirect::FilterActionRedirect()
  : FilterActionWithAddress( "redirect", i18n("Redirect To") )
{
}

FilterAction::ReturnCode FilterActionRedirect::process( const Akonadi::Item & item ) const
{
  if ( mParameter.isEmpty() )
    return ErrorButGoOn;

  KMime::Message::Ptr msg = MessageCore::Util::message( item );
  MessageFactory factory( msg, item.id() );
  factory.setFolderIdentity( Util::folderIdentity( item ) );
  factory.setIdentityManager( KernelIf->identityManager() );
  KMime::Message::Ptr rmsg = factory.createRedirect( mParameter );
  if ( !rmsg )
    return ErrorButGoOn;

  sendMDN( item, KMime::MDN::Dispatched );

  if ( !KernelIf->msgSender()->send( rmsg, MessageSender::SendLater ) ) {
    kDebug() << "FilterAction: could not redirect message (sending failed)";
    return ErrorButGoOn; // error: couldn't send
  }
  return GoOn;
}


//=============================================================================
// FilterActionExec - execute command
// Execute a shell command
//=============================================================================
class FilterActionExec : public FilterActionWithCommand
{
public:
  FilterActionExec();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  static FilterAction* newAction(void);
};

FilterAction* FilterActionExec::newAction(void)
{
  return (new FilterActionExec());
}

FilterActionExec::FilterActionExec()
  : FilterActionWithCommand( "execute", i18n("Execute Command") )
{
}

FilterAction::ReturnCode FilterActionExec::process( const Akonadi::Item & item ) const
{
  return FilterActionWithCommand::genericProcess( item, false ); // ignore output
}

//=============================================================================
// FilterActionExtFilter - use external filter app
// External message filter: executes a shell command with message
// on stdin; altered message is expected on stdout.
//=============================================================================

class FilterActionExtFilter: public FilterActionWithCommand
{
public:
  FilterActionExtFilter();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  static FilterAction* newAction(void);
};

FilterAction* FilterActionExtFilter::newAction(void)
{
  return (new FilterActionExtFilter);
}

FilterActionExtFilter::FilterActionExtFilter()
  : FilterActionWithCommand( "filter app", i18n("Pipe Through") )
{
}
FilterAction::ReturnCode FilterActionExtFilter::process( const Akonadi::Item & item ) const
{
  return FilterActionWithCommand::genericProcess( item, true ); // use output
}


#ifndef _WIN32_WCE
//=============================================================================
// FilterActionExecSound - execute command
// Execute a sound
//=============================================================================
class FilterActionExecSound : public FilterActionWithTest
{
public:
  FilterActionExecSound();
  ~FilterActionExecSound();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  virtual bool requiresBody() const;
  static FilterAction* newAction(void);
private:
  mutable Phonon::MediaObject* mPlayer;
};

FilterActionWithTest::FilterActionWithTest( const char* aName, const QString &aLabel )
  : FilterAction( aName, aLabel )
{
}

FilterActionWithTest::~FilterActionWithTest()
{
}

QWidget* FilterActionWithTest::createParamWidget( QWidget* parent ) const
{
  SoundTestWidget *le = new SoundTestWidget(parent);
  le->setUrl( mParameter );
  return le;
}


void FilterActionWithTest::applyParamWidgetValue( QWidget* paramWidget )
{
  mParameter = ((SoundTestWidget*)paramWidget)->url();
}

void FilterActionWithTest::setParamWidgetValue( QWidget* paramWidget ) const
{
  ((SoundTestWidget*)paramWidget)->setUrl( mParameter );
}

void FilterActionWithTest::clearParamWidget( QWidget* paramWidget ) const
{
  ((SoundTestWidget*)paramWidget)->clear();
}

void FilterActionWithTest::argsFromString( const QString &argsStr )
{
  mParameter = argsStr;
}

const QString FilterActionWithTest::argsAsString() const
{
  return mParameter;
}

const QString FilterActionWithTest::displayString() const
{
  return label() + " \"" + Qt::escape( argsAsString() ) + "\"";
}

FilterActionExecSound::FilterActionExecSound()
  : FilterActionWithTest( "play sound", i18n("Play Sound") ),
    mPlayer(0)
{
}

FilterActionExecSound::~FilterActionExecSound()
{
  delete mPlayer;
}

FilterAction* FilterActionExecSound::newAction(void)
{
  return (new FilterActionExecSound());
}

FilterAction::ReturnCode FilterActionExecSound::process( const Akonadi::Item&  ) const
{
  if ( mParameter.isEmpty() )
    return ErrorButGoOn;

  if ( !mPlayer )
    mPlayer = Phonon::createPlayer(Phonon::NotificationCategory);

  mPlayer->setCurrentSource( mParameter );
  mPlayer->play();
  return GoOn;
}

bool FilterActionExecSound::requiresBody() const
{
  return false;
}

#endif

FilterActionWithUrl::FilterActionWithUrl( const char* aName, const QString &aLabel )
  : FilterAction( aName, aLabel )
{
}

FilterActionWithUrl::~FilterActionWithUrl()
{
}

QWidget* FilterActionWithUrl::createParamWidget( QWidget* parent ) const
{
  KUrlRequester *le = new KUrlRequester(parent);
  le->setUrl( KUrl( mParameter ) );
  return le;
}


void FilterActionWithUrl::applyParamWidgetValue( QWidget* paramWidget )
{
    const KUrl url = ((KUrlRequester*)paramWidget)->url();
  mParameter = url.isLocalFile() ? url.toLocalFile() : url.path();
}

void FilterActionWithUrl::setParamWidgetValue( QWidget* paramWidget ) const
{
  ((KUrlRequester*)paramWidget)->setUrl( KUrl( mParameter ) );
}

void FilterActionWithUrl::clearParamWidget( QWidget* paramWidget ) const
{
  ((KUrlRequester*)paramWidget)->clear();
}

void FilterActionWithUrl::argsFromString( const QString &argsStr )
{
  mParameter = argsStr;
}

const QString FilterActionWithUrl::argsAsString() const
{
  return mParameter;
}

const QString FilterActionWithUrl::displayString() const
{
  return label() + " \"" + Qt::escape( argsAsString() ) + "\"";
}

//=============================================================================
// FilterActionAddToAddressBook
// - add email address from header to address book
//=============================================================================
class FilterActionAddToAddressBook: public FilterActionWithStringList
{
public:
  FilterActionAddToAddressBook();
  virtual ReturnCode process( const Akonadi::Item & item ) const;
  static FilterAction* newAction();

  virtual bool isEmpty() const { return false; }

  virtual QWidget* createParamWidget( QWidget* parent ) const;
  virtual void setParamWidgetValue( QWidget* paramWidget ) const;
  virtual void applyParamWidgetValue( QWidget* paramWidget );
  virtual void clearParamWidget( QWidget* paramWidget ) const;

  virtual const QString argsAsString() const;
  virtual void argsFromString( const QString &argsStr );

private:
  enum HeaderType
  {
    FromHeader,
    ToHeader,
    CcHeader,
    BccHeader
  };

  const QString mFromStr, mToStr, mCCStr, mBCCStr;
  HeaderType mHeaderType;
  Akonadi::Collection::Id mCollectionId;
  QString mCategory;
};

FilterAction* FilterActionAddToAddressBook::newAction()
{
  return ( new FilterActionAddToAddressBook );
}

FilterActionAddToAddressBook::FilterActionAddToAddressBook()
  : FilterActionWithStringList( "add to address book", i18n( "Add to Address Book" ) ),
    mFromStr( i18nc( "Email sender", "From" ) ),
    mToStr( i18nc( "Email recipient", "To" ) ),
    mCCStr( i18n( "CC" ) ),
    mBCCStr( i18n( "BCC" ) ),
    mHeaderType( FromHeader ),
    mCollectionId( -1 ),
    mCategory( i18n( "KMail Filter" ) )
{
}

FilterAction::ReturnCode FilterActionAddToAddressBook::process( const Akonadi::Item & item ) const
{
  const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();

  QString headerLine;
  switch ( mHeaderType ) {
    case FromHeader: headerLine = msg->from()->asUnicodeString(); break;
    case ToHeader: headerLine = msg->to()->asUnicodeString(); break;
    case CcHeader: headerLine = msg->cc()->asUnicodeString(); break;
    case BccHeader: headerLine = msg->bcc()->asUnicodeString(); break;
  }

  const QStringList emails = KPIMUtils::splitAddressList( headerLine );

  foreach ( const QString singleEmail, emails ) {
    QString name, email;
    KABC::Addressee::parseEmailAddress( singleEmail, name, email );

    KABC::Addressee contact;
    contact.setNameFromString( name );
    contact.insertEmail( email, true );
    if ( !mCategory.isEmpty() )
      contact.insertCategory( mCategory );

    KPIM::AddContactJob *job = new KPIM::AddContactJob( contact, Akonadi::Collection( mCollectionId ) );
    job->start();
  }

  return GoOn;
}

QWidget* FilterActionAddToAddressBook::createParamWidget( QWidget* parent ) const
{
  QWidget *widget = new QWidget( parent );
  QGridLayout *layout = new QGridLayout ( widget );

  KComboBox *headerCombo = new KComboBox( widget );
  headerCombo->setObjectName( "HeaderComboBox" );
  layout->addWidget( headerCombo, 0, 0, 2, 1, Qt::AlignVCenter );

  QLabel *label = new QLabel( i18n( "with category" ), widget );
  layout->addWidget( label, 0, 1 );

  KLineEdit *categoryEdit = new KLineEdit( widget );
  categoryEdit->setObjectName( "CategoryEdit" );
  layout->addWidget( categoryEdit, 0, 2 );

  label = new QLabel( i18n( "in address book" ), widget );
  layout->addWidget( label, 1, 1 );

  Akonadi::CollectionComboBox *collectionComboBox = new Akonadi::CollectionComboBox( widget );
  collectionComboBox->setMimeTypeFilter( QStringList() << KABC::Addressee::mimeType() );
  collectionComboBox->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );

  collectionComboBox->setObjectName( "AddressBookComboBox" );
  collectionComboBox->setToolTip( i18n( "<p>This defines the preferred address book.<br />"
        "If it is not accessible, the filter will fallback to the default address book.</p>" ) );
  layout->addWidget( collectionComboBox, 1, 2 );

  setParamWidgetValue( widget );

  return widget;
}

void FilterActionAddToAddressBook::setParamWidgetValue( QWidget* paramWidget ) const
{
  KComboBox *headerCombo = paramWidget->findChild<KComboBox*>( "HeaderComboBox" );
  Q_ASSERT( headerCombo );
  headerCombo->clear();
  headerCombo->addItem( mFromStr, FromHeader );
  headerCombo->addItem( mToStr, ToHeader );
  headerCombo->addItem( mCCStr, CcHeader );
  headerCombo->addItem( mBCCStr, BccHeader );

  headerCombo->setCurrentIndex( headerCombo->findData( mHeaderType ) );

  KLineEdit *categoryEdit = paramWidget->findChild<KLineEdit*>( "CategoryEdit" );
  Q_ASSERT( categoryEdit );
  categoryEdit->setText( mCategory );

  Akonadi::CollectionComboBox *collectionComboBox = paramWidget->findChild<Akonadi::CollectionComboBox*>( "AddressBookComboBox" );
  Q_ASSERT( collectionComboBox );
  collectionComboBox->setDefaultCollection( Akonadi::Collection( mCollectionId ) );
  collectionComboBox->setProperty( "collectionId", mCollectionId );
}

void FilterActionAddToAddressBook::applyParamWidgetValue( QWidget* paramWidget )
{
  KComboBox *headerCombo = paramWidget->findChild<KComboBox*>( "HeaderComboBox" );
  Q_ASSERT( headerCombo );
  mHeaderType = static_cast<HeaderType>( headerCombo->itemData( headerCombo->currentIndex() ).toInt() );

  KLineEdit *categoryEdit = paramWidget->findChild<KLineEdit*>( "CategoryEdit" );
  Q_ASSERT( categoryEdit );
  mCategory = categoryEdit->text();

  Akonadi::CollectionComboBox *collectionComboBox = paramWidget->findChild<Akonadi::CollectionComboBox*>( "AddressBookComboBox" );
  Q_ASSERT( collectionComboBox );
  const Akonadi::Collection collection = collectionComboBox->currentCollection();

  // it might be that the model of collectionComboBox has not finished loading yet, so
  // we use the previously 'stored' value from the 'collectionId' property
  if ( collection.isValid() )
    mCollectionId = collection.id();
  else {
    const QVariant value = collectionComboBox->property( "collectionId" );
    if ( value.isValid() )
      mCollectionId = value.toLongLong();
  }
}

void FilterActionAddToAddressBook::clearParamWidget( QWidget* paramWidget ) const
{
  KComboBox *headerCombo = paramWidget->findChild<KComboBox*>( "HeaderComboBox" );
  Q_ASSERT( headerCombo );
  headerCombo->setCurrentItem( 0 );

  KLineEdit *categoryEdit = paramWidget->findChild<KLineEdit*>( "CategoryEdit" );
  Q_ASSERT( categoryEdit );
  categoryEdit->setText( mCategory );
}

const QString FilterActionAddToAddressBook::argsAsString() const
{
  QString result;

  switch ( mHeaderType ) {
    case FromHeader: result = QLatin1String( "From" ); break;
    case ToHeader: result = QLatin1String( "To" ); break;
    case CcHeader: result = QLatin1String( "CC" ); break;
    case BccHeader: result = QLatin1String( "BCC" ); break;
  }

  result += QLatin1Char( '\t' );
  result += QString::number( mCollectionId );
  result += QLatin1Char( '\t' );
  result += mCategory;

  return result;
}

void FilterActionAddToAddressBook::argsFromString( const QString &argsStr )
{
  const QStringList parts = argsStr.split( QLatin1Char( '\t' ), QString::KeepEmptyParts );
  if ( parts[ 0 ] == QLatin1String( "From" ) )
    mHeaderType = FromHeader;
  else if ( parts[ 0 ] == QLatin1String( "To" ) )
    mHeaderType = ToHeader;
  else if ( parts[ 0 ] == QLatin1String( "CC" ) )
    mHeaderType = CcHeader;
  else if ( parts[ 0 ] == QLatin1String( "BCC" ) )
    mHeaderType = BccHeader;

  if ( parts.count() >= 2 )
    mCollectionId = parts[ 1 ].toLongLong();

  if ( parts.count() < 3 )
    mCategory.clear();
  else
    mCategory = parts[ 2 ];
}

//=============================================================================
//
//   Filter  Action  Dictionary
//
//=============================================================================
FilterActionDict::~FilterActionDict()
{
  qDeleteAll( mList );
}

void FilterActionDict::init(void)
{
  insert( FilterActionMove::newAction );
  insert( FilterActionCopy::newAction );
  insert( FilterActionIdentity::newAction );
  insert( FilterActionSetStatus::newAction );
  insert( FilterActionAddTag::newAction );
  insert( FilterActionFakeDisposition::newAction );
  insert( FilterActionTransport::newAction );
  insert( FilterActionReplyTo::newAction );
  insert( FilterActionForward::newAction );
  insert( FilterActionRedirect::newAction );
  insert( FilterActionSendReceipt::newAction );
  insert( FilterActionExec::newAction );
  insert( FilterActionExtFilter::newAction );
  insert( FilterActionRemoveHeader::newAction );
  insert( FilterActionAddHeader::newAction );
  insert( FilterActionRewriteHeader::newAction );
#ifndef _WIN32_WCE
  insert( FilterActionExecSound::newAction );
#endif
  insert( FilterActionAddToAddressBook::newAction );
  // Register custom filter actions below this line.
}
// The int in the QDict constructor (41) must be a prime
// and should be greater than the double number of FilterAction types
FilterActionDict::FilterActionDict()
  : QMultiHash<QString, FilterActionDesc*>()
{
  init();
}

void FilterActionDict::insert( FilterActionNewFunc aNewFunc )
{
  FilterAction *action = aNewFunc();
  FilterActionDesc* desc = new FilterActionDesc;
  desc->name = action->name();
  desc->label = action->label();
  desc->create = aNewFunc;
  QMultiHash<QString, FilterActionDesc*>::insert( desc->name, desc );
  QMultiHash<QString, FilterActionDesc*>::insert( desc->label, desc );
  mList.append( desc );
  delete action;
}

