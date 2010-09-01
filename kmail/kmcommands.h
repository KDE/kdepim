// -*- mode: C++; c-file-style: "gnu" -*-

#ifndef KMCommands_h
#define KMCommands_h

#include <tqdatetime.h>
#include <tqguardedptr.h>
#include <tqptrlist.h>
#include <tqvaluelist.h>
#include <tqvaluevector.h>
#include <tqtimer.h>
#include <tqfont.h>
#include <kio/job.h>
#include "kmmsgbase.h" // for KMMsgStatus
#include <mimelib/string.h>
#include <kdepimmacros.h>
#include <kservice.h>
#include <ktempfile.h>

class TQPopupMenu;
class KMainWindow;
class KAction;
class KProgressDialog;
class KProcess;
class KMFilter;
class KMFolder;
class KMFolderImap;
class KMFolderNode;
class KMHeaders;
class KMMainWidget;
class KMMessage;
class KMMsgBase;
class KMReaderWin;
class partNode;
class DwBodyPart;
class DwEntity;
namespace KIO { class Job; }
namespace KMail {
  class Composer;
  class FolderJob;
  class EditorWatcher;
  class HeaderStyle;
  class HeaderStrategy;
}
namespace GpgME { class Error; }
namespace Kleo { class SpecialJob; }

typedef TQMap<int,KMFolder*> KMMenuToFolder;
typedef TQMap<partNode*, KMMessage*> PartNodeMessageMap;

class KDE_EXPORT KMCommand : public QObject
{
  Q_OBJECT
    friend class LaterDeleterWithCommandCompletion;

public:
  enum Result { Undefined, OK, Canceled, Failed };

  // Trival constructor, don't retrieve any messages
  KMCommand( TQWidget *parent = 0 );
  // Retrieve all messages in msgList when start is called.
  KMCommand( TQWidget *parent, const TQPtrList<KMMsgBase> &msgList );
  // Retrieve the single message msgBase when start is called.
  KMCommand( TQWidget *parent, KMMsgBase *msgBase );
  // Retrieve the single message msgBase when start is called.
  KMCommand( TQWidget *parent, KMMessage *message );
  virtual ~KMCommand();

  /** These folders will be closed by the dtor, handy, if you need to keep
      a folder open during the lifetime of the command, but don't want to
      care about closing it again.
   */
  void keepFolderOpen( KMFolder *folder );

  /** Returns the result of the command. Only call this method from the slot
      connected to completed().
  */
  Result result();

public slots:
  // Retrieve messages then calls execute
  void start();

  // advance the progressbar, emitted by the folderjob
  void slotProgress( unsigned long done, unsigned long total );

signals:

  /// @param result The status of the command.
  void messagesTransfered( KMCommand::Result result );

  /// Emitted when the command has completed.
  void completed( KMCommand *command );

protected:
  // Returns list of messages retrieved
  const TQPtrList<KMMessage> retrievedMsgs() const;
  // Returns the single message retrieved
  KMMessage *retrievedMessage() const;
  // Returns the parent widget
  TQWidget *parentWidget() const;

  bool deletesItself() { return mDeletesItself; }
  /** Specify whether the subclass takes care of the deletion of the object.
      By default the base class will delete the object.
      @param deletesItself true if the subclass takes care of deletion, false
                           if the base class should take care of deletion
  */
  void setDeletesItself( bool deletesItself )
  { mDeletesItself = deletesItself; }

  bool emitsCompletedItself() { return mEmitsCompletedItself; }
  /** Specify whether the subclass takes care of emitting the completed()
      signal. By default the base class will emit this signal.
      @param emitsCompletedItself true if the subclass emits the completed
                                  signal, false if the base class should emit
                                  the signal
  */
  void setEmitsCompletedItself( bool emitsCompletedItself )
  { mEmitsCompletedItself = emitsCompletedItself; }

  /** Use this to set the result of the command.
      @param result The result of the command.
   */
  void setResult( Result result )
  { mResult = result; }

private:
  // execute should be implemented by derived classes
  virtual Result execute() = 0;

  /** transfers the list of (imap)-messages
   *  this is a necessary preparation for e.g. forwarding */
  void transferSelectedMsgs();

private slots:
  /** Called from start() via a single shot timer. */
  virtual void slotStart();

  void slotPostTransfer( KMCommand::Result result );
  /** the msg has been transferred */
  void slotMsgTransfered(KMMessage* msg);
  /** the KMImapJob is finished */
  void slotJobFinished();
  /** the transfer was canceled */
  void slotTransferCancelled();

private:
  // ProgressDialog for transferring messages
  KProgressDialog* mProgressDialog;
  //Currently only one async command allowed at a time
  static int mCountJobs;
  int mCountMsgs;
  Result mResult;
  bool mDeletesItself : 1;
  bool mEmitsCompletedItself : 1;

  TQWidget *mParent;
  TQPtrList<KMMessage> mRetrievedMsgs;
  TQPtrList<KMMsgBase> mMsgList;
  TQValueList<TQGuardedPtr<KMFolder> > mFolders;
};

class KDE_EXPORT KMMailtoComposeCommand : public KMCommand
{
  Q_OBJECT

public:
  KMMailtoComposeCommand( const KURL &url, KMMessage *msg=0 );

private:
  virtual Result execute();

  KURL mUrl;
  KMMessage *mMessage;
};

class KDE_EXPORT KMMailtoReplyCommand : public KMCommand
{
  Q_OBJECT

public:
  KMMailtoReplyCommand( TQWidget *parent, const KURL &url,
			KMMessage *msg, const TQString &selection );

private:
  virtual Result execute();

  KURL mUrl;
  TQString mSelection;
};

class KDE_EXPORT KMMailtoForwardCommand : public KMCommand
{
  Q_OBJECT

public:
  KMMailtoForwardCommand( TQWidget *parent, const KURL &url,
			  KMMessage *msg );

private:
  virtual Result execute();

  KURL mUrl;
};

class KDE_EXPORT KMMailtoAddAddrBookCommand : public KMCommand
{
  Q_OBJECT

public:
  KMMailtoAddAddrBookCommand( const KURL &url, TQWidget *parent );

private:
  virtual Result execute();

  KURL mUrl;
};

class KDE_EXPORT KMAddBookmarksCommand : public KMCommand
{
  Q_OBJECT

public:
  KMAddBookmarksCommand( const KURL &url, TQWidget *parent );

private:
  virtual Result execute();

  KURL mUrl;
};


class KDE_EXPORT KMMailtoOpenAddrBookCommand : public KMCommand
{
  Q_OBJECT

public:
  KMMailtoOpenAddrBookCommand( const KURL &url, TQWidget *parent );

private:
  virtual Result execute();

  KURL mUrl;
};

class KDE_EXPORT KMUrlCopyCommand : public KMCommand
{
  Q_OBJECT

public:
  KMUrlCopyCommand( const KURL &url, KMMainWidget *mainWidget = 0 );

private:
  virtual Result execute();

  KURL mUrl;
  KMMainWidget *mMainWidget;
};

class KDE_EXPORT KMUrlOpenCommand : public KMCommand
{
  Q_OBJECT

public:
  KMUrlOpenCommand( const KURL &url, KMReaderWin *readerWin );

private:
  virtual Result execute();

  KURL mUrl;
  KMReaderWin *mReaderWin;
};

class KDE_EXPORT KMUrlSaveCommand : public KMCommand
{
  Q_OBJECT

public:
  KMUrlSaveCommand( const KURL &url, TQWidget *parent );

private slots:
  void slotUrlSaveResult( KIO::Job *job );

private:
  virtual Result execute();

  KURL mUrl;
};

class KDE_EXPORT KMEditMsgCommand : public KMCommand
{
  Q_OBJECT

public:
  KMEditMsgCommand( TQWidget *parent, KMMessage *msg );

private:
  virtual Result execute();
};

class KDE_EXPORT KMUseTemplateCommand : public KMCommand
{
  Q_OBJECT

public:
  KMUseTemplateCommand( TQWidget *parent, KMMessage *msg );

private:
  virtual Result execute();
};

class KDE_EXPORT KMShowMsgSrcCommand : public KMCommand
{
  Q_OBJECT

public:
  KMShowMsgSrcCommand( TQWidget *parent, KMMessage *msg,
		       bool fixedFont );
  virtual Result execute();

private:
  bool mFixedFont;
  bool mMsgWasComplete;
};

class KDE_EXPORT KMSaveMsgCommand : public KMCommand
{
  Q_OBJECT

public:
  KMSaveMsgCommand( TQWidget *parent, const TQPtrList<KMMsgBase> &msgList );
  KMSaveMsgCommand( TQWidget *parent, KMMessage * msg );
  KURL url();

private:
  virtual Result execute();

private slots:
  void slotSaveDataReq();
  void slotSaveResult(KIO::Job *job);
  /** the message has been transferred for saving */
  void slotMessageRetrievedForSaving(KMMessage *msg);

private:
  static const int MAX_CHUNK_SIZE = 64*1024;
  KURL mUrl;
  TQValueList<unsigned long> mMsgList;
  TQValueList<KMMsgBase *> mUngetMsgs;
  unsigned int mMsgListIndex;
  KMMessage *mStandAloneMessage;
  TQByteArray mData;
  int mOffset;
  size_t mTotalSize;
  KIO::TransferJob *mJob;
};

class KDE_EXPORT KMOpenMsgCommand : public KMCommand
{
  Q_OBJECT

public:
  KMOpenMsgCommand( TQWidget *parent, const KURL & url = KURL(),
                    const TQString & encoding = TQString() );

private:
  virtual Result execute();

private slots:
  void slotDataArrived( KIO::Job *job, const TQByteArray & data );
  void slotResult( KIO::Job *job );

private:
  static const int MAX_CHUNK_SIZE = 64*1024;
  KURL mUrl;
  DwString mMsgString;
  KIO::TransferJob *mJob;
  const TQString mEncoding;
};

class KDE_EXPORT KMSaveAttachmentsCommand : public KMCommand
{
  Q_OBJECT
public:
  /** Use this to save all attachments of the given message.
      @param parent  The parent widget of the command used for message boxes.
      @param msg     The message of which the attachments should be saved.
   */
  KMSaveAttachmentsCommand( TQWidget *parent, KMMessage *msg  );
  /** Use this to save all attachments of the given messages.
      @param parent  The parent widget of the command used for message boxes.
      @param msgs    The messages of which the attachments should be saved.
   */
  KMSaveAttachmentsCommand( TQWidget *parent, const TQPtrList<KMMsgBase>& msgs );
  /** Use this to save the specified attachments of the given message.
      @param parent       The parent widget of the command used for message
                          boxes.
      @param attachments  The attachments that should be saved.
      @param msg          The message that the attachments belong to.
      @param encoded      True if the transport encoding should not be removed
                          when the attachment is saved.
   */
  KMSaveAttachmentsCommand( TQWidget *parent, TQPtrList<partNode> &attachments,
                            KMMessage *msg, bool encoded = false  );

private slots:
  void slotSaveAll();

private:
  virtual Result execute();
  Result saveItem( partNode *node, const KURL& url );

private:
  PartNodeMessageMap mAttachmentMap;
  bool mImplicitAttachments;
  bool mEncoded;
};

class KDE_EXPORT KMReplyToCommand : public KMCommand
{
  Q_OBJECT

public:
  KMReplyToCommand( TQWidget *parent, KMMessage *msg,
                    const TQString &selection = TQString::null );

private:
  virtual Result execute();

private:
  TQString mSelection;
};

class KDE_EXPORT KMNoQuoteReplyToCommand : public KMCommand
{
  Q_OBJECT

public:
  KMNoQuoteReplyToCommand( TQWidget *parent, KMMessage *msg );

private:
  virtual Result execute();
};

class KMReplyListCommand : public KMCommand
{
  Q_OBJECT

public:
  KMReplyListCommand( TQWidget *parent, KMMessage *msg,
		      const TQString &selection = TQString::null );

private:
  virtual Result execute();

private:
  TQString mSelection;
};

class KDE_EXPORT KMReplyToAllCommand : public KMCommand
{
  Q_OBJECT

public:
  KMReplyToAllCommand( TQWidget *parent, KMMessage *msg,
		       const TQString &selection = TQString::null );

private:
  virtual Result execute();

private:
  TQString mSelection;
};

class KDE_EXPORT KMReplyAuthorCommand : public KMCommand
{
  Q_OBJECT

public:
  KMReplyAuthorCommand( TQWidget *parent, KMMessage *msg,
                        const TQString &selection = TQString::null );

private:
  virtual Result execute();

private:
  TQString mSelection;
};

class KDE_EXPORT KMForwardInlineCommand : public KMCommand
{
  Q_OBJECT

public:
  KMForwardInlineCommand( TQWidget *parent, const TQPtrList<KMMsgBase> &msgList,
                          uint identity = 0 );
  KMForwardInlineCommand( TQWidget *parent, KMMessage * msg,
                          uint identity = 0 );

private:
  virtual Result execute();

private:
  uint mIdentity;
};

class KDE_EXPORT KMForwardAttachedCommand : public KMCommand
{
  Q_OBJECT

public:
  KMForwardAttachedCommand( TQWidget *parent, const TQPtrList<KMMsgBase> &msgList,
			    uint identity = 0, KMail::Composer *win = 0 );
  KMForwardAttachedCommand( TQWidget *parent, KMMessage * msg,
			    uint identity = 0, KMail::Composer *win = 0 );

private:
  virtual Result execute();

  uint mIdentity;
  TQGuardedPtr<KMail::Composer> mWin;
};

class KDE_EXPORT KMForwardDigestCommand : public KMCommand
{
  Q_OBJECT

public:
  KMForwardDigestCommand( TQWidget *parent, const TQPtrList<KMMsgBase> &msgList,
			    uint identity = 0, KMail::Composer *win = 0 );
  KMForwardDigestCommand( TQWidget *parent, KMMessage * msg,
			    uint identity = 0, KMail::Composer *win = 0 );

private:
  virtual Result execute();

  uint mIdentity;
  TQGuardedPtr<KMail::Composer> mWin;
};

class KDE_EXPORT KMRedirectCommand : public KMCommand
{
  Q_OBJECT

public:
  KMRedirectCommand( TQWidget *parent, KMMessage *msg );

private:
  virtual Result execute();
};

class KDE_EXPORT KMCustomReplyToCommand : public KMCommand
{
  Q_OBJECT

public:
  KMCustomReplyToCommand( TQWidget *parent, KMMessage *msg,
                          const TQString &selection,
                          const TQString &tmpl );

private:
  virtual Result execute();

private:
  TQString mSelection;
  TQString mTemplate;
};

class KDE_EXPORT KMCustomReplyAllToCommand : public KMCommand
{
  Q_OBJECT

public:
  KMCustomReplyAllToCommand( TQWidget *parent, KMMessage *msg,
                          const TQString &selection,
                          const TQString &tmpl );

private:
  virtual Result execute();

private:
  TQString mSelection;
  TQString mTemplate;
};

class KDE_EXPORT KMCustomForwardCommand : public KMCommand
{
  Q_OBJECT

public:
  KMCustomForwardCommand( TQWidget *parent, const TQPtrList<KMMsgBase> &msgList,
                          uint identity, const TQString &tmpl );
  KMCustomForwardCommand( TQWidget *parent, KMMessage * msg,
                          uint identity, const TQString &tmpl );

private:
  virtual Result execute();

  uint mIdentity;
  TQString mTemplate;
};

class KDE_EXPORT KMPrintCommand : public KMCommand
{
  Q_OBJECT

public:
  KMPrintCommand( TQWidget *parent, KMMessage *msg,
                  const KMail::HeaderStyle *headerStyle = 0,
                  const KMail::HeaderStrategy *headerStrategy = 0,
                  bool htmlOverride = false,
                  bool htmlLoadExtOverride = false,
                  bool useFixedFont = false,
                  const TQString & encoding = TQString() );

  void setOverrideFont( const TQFont& );

private:
  virtual Result execute();

  const KMail::HeaderStyle *mHeaderStyle;
  const KMail::HeaderStrategy *mHeaderStrategy;
  bool mHtmlOverride;
  bool mHtmlLoadExtOverride;
  bool mUseFixedFont;
  TQFont mOverrideFont;
  TQString mEncoding;
};

class KDE_EXPORT KMSetStatusCommand : public KMCommand
{
  Q_OBJECT

public:
  // Serial numbers
  KMSetStatusCommand( KMMsgStatus status, const TQValueList<Q_UINT32> &,
                      bool toggle=false );

private:
  virtual Result execute();

  KMMsgStatus mStatus;
  TQValueList<Q_UINT32> mSerNums;
  TQValueList<int> mIds;
  bool mToggle;
};

class KDE_EXPORT KMFilterCommand : public KMCommand
{
  Q_OBJECT

public:
  KMFilterCommand( const TQCString &field, const TQString &value );

private:
  virtual Result execute();

  TQCString mField;
  TQString mValue;
};


class KDE_EXPORT KMFilterActionCommand : public KMCommand
{
  Q_OBJECT

public:
  KMFilterActionCommand( TQWidget *parent,
			 const TQPtrList<KMMsgBase> &msgList,
			 KMFilter *filter );

private:
  virtual Result execute();
  TQValueList<Q_UINT32> serNumList;
  KMFilter *mFilter;
};


class KDE_EXPORT KMMetaFilterActionCommand : public QObject
{
  Q_OBJECT

public:
  KMMetaFilterActionCommand( KMFilter *filter, KMHeaders *headers,
			     KMMainWidget *main );

public slots:
  void start();

private:
  KMFilter *mFilter;
  KMHeaders *mHeaders;
  KMMainWidget *mMainWidget;
};

class KDE_EXPORT FolderShortcutCommand : public QObject
{
  Q_OBJECT

public:
  FolderShortcutCommand( KMMainWidget* mainwidget, KMFolder *folder );
  ~FolderShortcutCommand();

public slots:
  void start();
  /** Assign a KActio to the command which is used to trigger it. This
   * action will be deleted along with the command, so you don't need to
   * keep track of it separately. */
  void setAction( KAction* );

private:
  KMMainWidget *mMainWidget;
  KMFolder *mFolder;
  KAction *mAction;
};


class KDE_EXPORT KMMailingListFilterCommand : public KMCommand
{
  Q_OBJECT

public:
  KMMailingListFilterCommand( TQWidget *parent, KMMessage *msg );

private:
  virtual Result execute();
};


  /** Returns a popupmenu containing a hierarchy of folder names.
      Each item in the popupmenu is connected to a slot, if
      move is TRUE this slot will cause all selected messages to
      be moved into the given folder, otherwise messages will be
      copied.
      Am empty KMMenuToFolder must be passed in. */

class KDE_EXPORT KMMenuCommand : public KMCommand
{
  Q_OBJECT

public:
  static void folderToPopupMenu(bool move, TQObject *receiver,
    KMMenuToFolder *aMenuToFolder, TQPopupMenu *menu );

  static void makeFolderMenu(KMFolderNode* item, bool move,
    TQObject *receiver, KMMenuToFolder *aMenuToFolder, TQPopupMenu *menu );
};

class KDE_EXPORT KMCopyCommand : public KMMenuCommand
{
  Q_OBJECT

public:
  KMCopyCommand( KMFolder* destFolder,
		 const TQPtrList<KMMsgBase> &msgList );
  KMCopyCommand( KMFolder* destFolder, KMMessage *msg );

protected slots:
  void slotJobFinished( KMail::FolderJob *job );

  void slotFolderComplete( KMFolderImap*, bool success );

private:
  virtual Result execute();

  KMFolder *mDestFolder;
  TQPtrList<KMMsgBase> mMsgList;
  TQValueList<KMail::FolderJob*> mPendingJobs;
};

namespace KPIM {
  class ProgressItem;
}
class KDE_EXPORT KMMoveCommand : public KMMenuCommand
{
  Q_OBJECT

public:
  KMMoveCommand( KMFolder* destFolder, const TQPtrList<KMMsgBase> &msgList );
  KMMoveCommand( KMFolder* destFolder, KMMessage * msg );
  KMMoveCommand( KMFolder* destFolder, KMMsgBase * msgBase );
  KMFolder* destFolder() const { return mDestFolder; }

public slots:
  void slotImapFolderCompleted(KMFolderImap *folder, bool success);
  void slotMsgAddedToDestFolder(KMFolder *folder, Q_UINT32 serNum);
  void slotMoveCanceled();

protected:
  // Needed for KMDeleteCommand for "move to trash"
  KMMoveCommand( Q_UINT32 sernum );
  void setDestFolder( KMFolder* folder ) { mDestFolder = folder; }
  void addMsg( KMMsgBase *msg ) { mSerNumList.append( msg->getMsgSerNum() ); }
  TQValueVector<KMFolder*> mOpenedFolders;

private:
  virtual Result execute();
  void completeMove( Result result );

  KMFolder *mDestFolder;
  TQValueList<Q_UINT32> mSerNumList;
  // List of serial numbers that have to be transferred to a host.
  // Ticked off as they come in via msgAdded signals.
  TQValueList<Q_UINT32> mLostBoys;
  KPIM::ProgressItem *mProgressItem;
  bool mCompleteWithAddedMsg;
};

class KDE_EXPORT KMDeleteMsgCommand : public KMMoveCommand
{
  Q_OBJECT

public:
  KMDeleteMsgCommand( KMFolder* srcFolder, const TQPtrList<KMMsgBase> &msgList );
  KMDeleteMsgCommand( KMFolder* srcFolder, KMMessage * msg );
  KMDeleteMsgCommand( Q_UINT32 sernum );

private:
  static KMFolder * findTrashFolder( KMFolder * srcFolder );

};

class KDE_EXPORT KMUrlClickedCommand : public KMCommand
{
  Q_OBJECT

public:
  KMUrlClickedCommand( const KURL &url, uint identity,
    KMReaderWin *readerWin, bool mHtmlPref, KMMainWidget *mainWidget = 0 );

private:
  virtual Result execute();

  KURL mUrl;
  uint mIdentity;
  KMReaderWin *mReaderWin;
  bool mHtmlPref;
  KMMainWidget *mMainWidget;
};

class KDE_EXPORT KMLoadPartsCommand : public KMCommand
{
  Q_OBJECT

public:
  KMLoadPartsCommand( TQPtrList<partNode>& parts, KMMessage* msg );
  KMLoadPartsCommand( partNode* node, KMMessage* msg );
  KMLoadPartsCommand( PartNodeMessageMap& partMap );

public slots:
  void slotPartRetrieved( KMMessage* msg, TQString partSpecifier );

signals:
  void partsRetrieved();

private:
  // Retrieve parts then calls execute
  virtual void slotStart();

  virtual Result execute();

  int mNeedsRetrieval;
  PartNodeMessageMap mPartMap;
};

class KDE_EXPORT KMResendMessageCommand : public KMCommand
{
  Q_OBJECT

public:
  KMResendMessageCommand( TQWidget *parent, KMMessage *msg=0 );

private:
  virtual Result execute();
};

class KDE_EXPORT KMMailingListCommand : public KMCommand
{
  Q_OBJECT
public:
  KMMailingListCommand( TQWidget *parent, KMFolder *folder );
private:
  virtual Result execute();
private slots:
  void commandCompleted( KMCommand *command );
protected:
  virtual KURL::List urls() const =0;
protected:
  KMFolder *mFolder;
};

class KDE_EXPORT KMMailingListPostCommand : public KMMailingListCommand
{
  Q_OBJECT
public:
  KMMailingListPostCommand( TQWidget *parent, KMFolder *folder );
protected:
  virtual KURL::List urls() const;
};

class KDE_EXPORT KMMailingListSubscribeCommand : public KMMailingListCommand
{
  Q_OBJECT
public:
  KMMailingListSubscribeCommand( TQWidget *parent, KMFolder *folder );
protected:
  virtual KURL::List urls() const;
};

class KDE_EXPORT KMMailingListUnsubscribeCommand : public KMMailingListCommand
{
  Q_OBJECT
public:
  KMMailingListUnsubscribeCommand( TQWidget *parent, KMFolder *folder );
protected:
  virtual KURL::List urls() const;
};

class KDE_EXPORT KMMailingListArchivesCommand : public KMMailingListCommand
{
  Q_OBJECT
public:
  KMMailingListArchivesCommand( TQWidget *parent, KMFolder *folder );
protected:
  virtual KURL::List urls() const;
};

class KDE_EXPORT KMMailingListHelpCommand : public KMMailingListCommand
{
  Q_OBJECT
public:
  KMMailingListHelpCommand( TQWidget *parent, KMFolder *folder );
protected:
  virtual KURL::List urls() const;
};

class KDE_EXPORT KMIMChatCommand : public KMCommand
{
  Q_OBJECT

public:
  KMIMChatCommand( const KURL &url, KMMessage *msg=0 );

private:
  /**
   * Try to start a chat with the addressee associated the mail address in <i>url</i>.
   * If there is no addressee for the email address, or more than one, a KMessageBox is shown to inform the user.
   * If the addressee does not have instant messaging address(es), this is currently handled by the instant messaging client
   * which handles the request, since we don't have a convenient API for extracting them using KABC.
   */
  virtual Result execute();

  KURL mUrl;
  KMMessage *mMessage;
};

class KDE_EXPORT KMHandleAttachmentCommand : public KMCommand
{
  Q_OBJECT

public:
  /** Supported types of attachment handling */
  enum AttachmentAction
  {
    Open = 1,
    OpenWith = 2,
    View = 3,
    Save = 4,
    Properties = 5,
    ChiasmusEncrypt = 6
  };
  /**
   * Construct a new command
   * @param node the partNode
   * @param msg the KMMessage
   * @param atmId the ID of the attachment, the partNode must know this
   * @param atmName the name of the attachment
   * @param action what to do with the attachment
   * @param offer specify a KService that should handle the "open" action, 0 otherwise
   */
  KMHandleAttachmentCommand( partNode* node, KMMessage* msg, int atmId,
      const TQString& atmName, AttachmentAction action, KService::Ptr offer, TQWidget* parent );


signals:
  void showAttachment( int id, const TQString& name );

private:
  virtual Result execute();

  TQString createAtmFileLink() const;

  /** Get a KService if it was not specified */
  KService::Ptr getServiceOffer();

  /** Open needs a valid KService */
  void atmOpen();

  /** Display an open-with dialog */
  void atmOpenWith();

  /**
   * Viewing is not supported by this command
   * so it just emits showAttachment
   */
  void atmView();

  /** Save the attachment */
  void atmSave();

  /** Display the properties */
  void atmProperties();

  /** Encrypt using chiasmus */
  void atmEncryptWithChiasmus();

private slots:
  /** Called from start() via a single shot timer. */
  virtual void slotStart();

  /**
   * Called when the part was downloaded
   * Calls execute
   */
  void slotPartComplete();

  void slotAtmDecryptWithChiasmusResult( const GpgME::Error &, const TQVariant & );
  void slotAtmDecryptWithChiasmusUploadResult( KIO::Job * );

private:
  partNode* mNode;
  KMMessage* mMsg;
  int mAtmId;
  TQString mAtmName;
  AttachmentAction mAction;
  KService::Ptr mOffer;
  Kleo::SpecialJob *mJob;

};


/** Base class for commands modifying attachements of existing messages. */
class KDE_EXPORT AttachmentModifyCommand : public KMCommand
{
  Q_OBJECT
  public:
    AttachmentModifyCommand( partNode *node, KMMessage *msg, TQWidget *parent );
    AttachmentModifyCommand( int nodeId, KMMessage *msg, TQWidget *parent );
    ~AttachmentModifyCommand();

  protected:
    void storeChangedMessage( KMMessage* msg );
    virtual Result doAttachmentModify() = 0;

  protected:
    int mPartIndex;
    Q_UINT32 mSernum;

  private:
    Result execute();

  private slots:
    void messageStoreResult( KMFolderImap* folder, bool success );
    void messageDeleteResult( KMCommand *cmd );

  private:
    TQGuardedPtr<KMFolder> mFolder;
};

class KDE_EXPORT KMDeleteAttachmentCommand : public AttachmentModifyCommand
{
  Q_OBJECT
  public:
    KMDeleteAttachmentCommand( partNode *node, KMMessage *msg, TQWidget *parent );
    KMDeleteAttachmentCommand( int nodeId, KMMessage *msg, TQWidget *parent );
    ~KMDeleteAttachmentCommand();

  protected:
    Result doAttachmentModify();
};


class KDE_EXPORT KMEditAttachmentCommand : public AttachmentModifyCommand
{
  Q_OBJECT
  public:
    KMEditAttachmentCommand( partNode *node, KMMessage *msg, TQWidget *parent );
    KMEditAttachmentCommand( int nodeId, KMMessage *msg, TQWidget *parent );
    ~KMEditAttachmentCommand();

  protected:
    Result doAttachmentModify();

  private slots:
    void editDone( KMail::EditorWatcher *watcher );

  private:
    KTempFile mTempFile;
};

class KDE_EXPORT CreateTodoCommand : public KMCommand
{
  Q_OBJECT
  public:
    CreateTodoCommand( TQWidget *parent, KMMessage *msg );

  private:
    Result execute();
};

#endif /*KMCommands_h*/
