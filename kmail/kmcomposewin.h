/* -*- mode: C++; c-file-style: "gnu" -*-
 * KMComposeWin Header File
 * Author: Markus Wuebben <markus.wuebben@kde.org>
 */
#ifndef __KMComposeWin
#define __KMComposeWin

#ifndef KDE_USE_FINAL
# ifndef REALLY_WANT_KMCOMPOSEWIN_H
#  error Do not include kmcomposewin.h anymore. Include composer.h instead.
# endif
#endif

#include "composer.h"
#include "messagesender.h"

#include <set>

#include <tqlabel.h>
#include <tqlistview.h>

#include <tqcheckbox.h>
#include <tqpushbutton.h>
#include <tqclipboard.h>
#include <tqpalette.h>
#include <tqfont.h>
#include <tqptrlist.h>
#include <tqvaluevector.h>
#include <tqsplitter.h>

#include <kio/job.h>
#include <kglobalsettings.h>
#include <kdeversion.h>
#include <keditcl.h>
#include <ktempdir.h>

#include "mailcomposerIface.h"
#include "accountmanager.h"

#include <libkdepim/addresseelineedit.h>
#include <mimelib/mediatyp.h>

#include <kleo/enum.h>

class TQCloseEvent;
class TQComboBox;
class TQFrame;
class TQGridLayout;
class TQListView;
class TQPopupMenu;
class TQPushButton;
class TQCString;
class KCompletion;
class KMEdit;
class KMComposeWin;
class KMFolderComboBox;
class KMFolder;
class KMMessage;
class KMMessagePart;
class KProcess;
class KDirWatch;
class KSelectAction;
class KFontAction;
class KFontSizeAction;
class KSelectAction;
class KStatusBar;
class KAction;
class KToggleAction;
class KTempFile;
class KToolBar;
class KToggleAction;
class KSelectColorAction;
class KURL;
class KRecentFilesAction;
class SpellingFilter;
class MessageComposer;
class RecipientsEditor;
class KMLineEdit;
class KMLineEditSpell;
class KMAtmListViewItem;
class SnippetWidget;

namespace KPIM {
  class IdentityCombo;
  class Identity;
}

namespace KMail {
  class AttachmentListView;
  class DictionaryComboBox;
  class EditorWatcher;
}

namespace GpgME {
  class Error;
}

//-----------------------------------------------------------------------------
class KMComposeWin : public KMail::Composer, public MailComposerIface
{
  Q_OBJECT
  friend class ::KMEdit;
  friend class ::MessageComposer;

private: // mailserviceimpl, kmkernel, kmcommands, callback, kmmainwidget
  KMComposeWin( KMMessage* msg=0, uint identity=0 );
  ~KMComposeWin();
public:
  static Composer * create( KMMessage * msg = 0, uint identity = 0 );

  MailComposerIface * asMailComposerIFace() { return this; }
  const MailComposerIface * asMailComposerIFace() const { return this; }

public: // mailserviceimpl
  /**
   * From MailComposerIface
   */
  void send(int how);
  void addAttachmentsAndSend(const KURL::List &urls, const TQString &comment, int how);
  void addAttachment(KURL url,TQString comment);
  void addAttachment(const TQString &name,
                    const TQCString &cte,
                    const TQByteArray &data,
                    const TQCString &type,
                    const TQCString &subType,
                    const TQCString &paramAttr,
                    const TQString &paramValue,
                    const TQCString &contDisp);
public: // kmcommand
  void setBody (TQString body);

private:
  /**
   * To catch palette changes
   */
  virtual bool event(TQEvent *e);

  /**
   * update colors
   */
  void readColorConfig();

  /**
   * Write settings to app's config file.
   */
   void writeConfig(void);

  /**
   * If necessary increases the word wrap of the editor so that it will
   * not wrap the body string
   */
   void verifyWordWrapLengthIsAdequate(const TQString&);

public: // kmkernel, kmcommands, callback
  /**
   * Set the message the composer shall work with. This discards
   * previous messages without calling applyChanges() on them before.
   */
   void setMsg(KMMessage* newMsg, bool mayAutoSign=TRUE,
	       bool allowDecryption=FALSE, bool isModified=FALSE);

   void disableWordWrap();

  /** Don't check if there are too many recipients for a mail,
   * eg. when sending out invitations.
   */
  void disableRecipientNumberCheck();

  /** Don't check for forgotten attachments for a mail,
   * eg. when sending out invitations.
   */
  void disableForgottenAttachmentsCheck();

  /**
   * Ignore the "sticky" setting of the transport combo box and prefer the X-KMail-Transport
   * header field of the message instead.
   * Do the same for the identity combo box, don't obey the "sticky" setting but use the
   * X-KMail-Identity header field instead.
   *
   * This is useful when sending out invitations, since you don't see the GUI and want the
   * identity and transport to be set to the values stored in the messages.
   */
  void ignoreStickyFields();

   /**
    * Returns @c true while the message composing is in progress.
    */
   bool isComposing() const { return mComposer != 0; }

private: // kmedit
  /**
   * Returns message of the composer. To apply the user changes to the
   * message, call applyChanges() first.
   */
   KMMessage* msg() const { return mMsg; }

public: // kmkernel
  /**
   * Set the filename which is used for autosaving.
   */
  void setAutoSaveFilename( const TQString & filename );

private:
  /**
   * Returns true if the message was modified by the user.
   */
  bool isModified() const;

  /**
   * Set whether the message should be treated as modified or not.
   */
  void setModified( bool modified );

public: // kmkernel, callback
  /**
   * If this flag is set the message of the composer is deleted when
   * the composer is closed and the message was not sent. Default: FALSE
   */
   inline void setAutoDelete(bool f) { mAutoDeleteMsg = f; }

  /**
   * If this flag is set, the compose window will delete itself after
   * the window has been closed.
   */
  void setAutoDeleteWindow( bool f );

public: // kmcommand
  /**
   * If this folder is set, the original message is inserted back after
   * cancelling
   */
   void setFolder(KMFolder* aFolder) { mFolder = aFolder; }
public: // kmkernel, kmcommand, mailserviceimpl
  /**
   * Recode to the specified charset
   */
   void setCharset(const TQCString& aCharset, bool forceDefault = FALSE);

public: // kmcommand
  /**
   * Sets the focus to the edit-widget and the cursor below the
   * "On ... you wrote" line when hasMessage is true.
   * Make sure you call this _after_ setMsg().
   */
   void setReplyFocus( bool hasMessage = true );

  /**
   * Sets the focus to the subject line edit. For use when creating a
   * message to a known recipient.
   */
   void setFocusToSubject();

private:
  /**
   * determines whether inline signing/encryption is selected
   */
   bool inlineSigningEncryptionSelected();

   /**
    * Tries to find the given mimetype @p type in the KDE Mimetype registry.
    * If found, returns its localized description, otherwise the @p type
    * in lowercase.
    */
   static TQString prettyMimeType( const TQString& type );
    TQString quotePrefixName() const;

private: // kmedit:
  KMLineEditSpell *sujectLineWidget() const { return mEdtSubject;}
  void setSubjectTextWasSpellChecked( bool _spell ) {
    mSubjectTextWasSpellChecked = _spell;
  }
  bool subjectTextWasSpellChecked() const { return mSubjectTextWasSpellChecked; }

  void paste( QClipboard::Mode mode );

public: // callback
  /** Disabled signing and encryption completely for this composer window. */
  void setSigningAndEncryptionDisabled( bool v )
  {
    mSigningAndEncryptionExplicitlyDisabled = v;
  }

private slots:
  void polish();
  /**
   * Actions:
   */
  void slotPrint();
  void slotAttachFile();
  void slotInsertRecentFile(const KURL&);
  void slotAttachedFile(const KURL&);
public slots: // kmkernel, callback
  void slotSendNow();
private slots:
  void slotSendNowVia( int item );
  void slotSendLater();
  void slotSendLaterVia( int item );

  void getTransportMenu();

  /**
   * Returns true when saving was successful.
   */
  void slotSaveDraft();
  void slotSaveTemplate();
  void slotNewComposer();
  void slotNewMailReader();
  void slotClose();
  void slotHelp();

  void slotFind();
  void slotSearchAgain();
  void slotReplace();
  void slotUndo();
  void slotRedo();
  void slotCut();
  void slotCopy();
  void slotPasteClipboard();
  void slotPasteClipboardAsQuotation();
  void slotPasteClipboardAsAttachment();
  void slotAddQuotes();
  void slotRemoveQuotes();
  void slotAttachPNGImageData(const TQByteArray &image);

  void slotMarkAll();

  void slotFolderRemoved(KMFolder*);

  void slotEditDone( KMail::EditorWatcher* watcher );

public slots: // kmkernel
  /**
     Tell the composer to always send the message, even if the user
     hasn't changed the next. This is useful if a message is
     autogenerated (e.g., via a DCOP call), and the user should
     simply be able to confirm the message and send it.
  */
  void slotSetAlwaysSend( bool bAlwaysSend );
private slots:
  /**
   * toggle fixed width font.
   */
  void slotUpdateFont();

  /**
   * Open addressbook editor dialog.
   */
  void slotAddrBook();
  /**
   * Insert a file to the end of the text in the editor.
   */
  void slotInsertFile();

  void slotSetCharset();
  /**
   * Check spelling of text.
   */
  void slotSpellcheck();
  void slotSpellcheckConfig();
  void slotSubjectTextSpellChecked();

  /**
   * Change crypto plugin to be used for signing/encrypting messages,
   * or switch to built-in OpenPGP code.
   */
  void slotSelectCryptoModule( bool init = false );

  /**
   * XML-GUI stuff
   */
  void slotStatusMessage(const TQString &message);
  void slotEditToolbars();
  void slotUpdateToolbars();
  void slotEditKeys();
  /**
   * Read settings from app's config file.
   */
  void readConfig( bool reload = false );
  /**
   * Change window title to given string.
   */
  void slotUpdWinTitle(const TQString& );

  /**
   * Switch the icon to lock or unlock respectivly.
   * Change states of all encrypt check boxes in the attachments listview
   */
  void slotEncryptToggled(bool);

  /**
   * Change states of all sign check boxes in the attachments listview
   */
  void slotSignToggled(bool);

public slots: // kmkernel, callback
  /**
   * Switch wordWrap on/off
   */
  void slotWordWrapToggled(bool);

private slots:
  /**
   * Append signature file to the end of the text in the editor.
   */
  void slotAppendSignature();

  /**
   * Prepend signature file at the beginning of the text in the editor.
   */
  void slotPrependSignature();

  /**
   * Insert signature file at the cursor position of the text in the editor.
   */
  void slotInsertSignatureAtCursor();

  /**
   * Attach sender's public key.
   */
  void slotInsertMyPublicKey();

  /**
   * Insert arbitary public key from public keyring in the editor.
   */
  void slotInsertPublicKey();

  /**
   * Enable/disable some actions in the Attach menu
   */
  void slotUpdateAttachActions();

  /**
   * Open a popup-menu in the attachments-listbox.
   */
  void slotAttachPopupMenu(TQListViewItem *, const TQPoint &, int);

  /**
   * Returns the number of the current attachment in the listbox,
   * or -1 if there is no current attachment
   */
  int currentAttachmentNum();

  /**
   * Attachment operations.
   */
  void slotAttachOpen();
  void slotAttachView();
  void slotAttachRemove();
  void slotAttachSave();
  void slotAttachProperties();
  void slotAttachOpenWith();
  void slotAttachEdit();
  void slotAttachEditWith();
  void slotAttachmentDragStarted();

  /**
   * Select an email from the addressbook and add it to the line
   * the pressed button belongs to.
   */
  void slotAddrBookTo();
  void slotAddrBookFrom();
  void slotAddrBookReplyTo();

  void slotCleanSpace();

  void slotToggleMarkup();
  void toggleMarkup(bool markup);
  void htmlToolBarVisibilityChanged( bool visible );

//  void slotSpellConfigure();
  void slotSpellcheckDone(int result);
  void slotSpellcheckDoneClearStatus();

public slots: // kmkernel
  void autoSaveMessage();

private slots:
  void updateCursorPosition();

  void slotView();

  /**
   * Update composer field to reflect new identity
   */
  void slotIdentityChanged(uint);

  /**
   * KIO slots for attachment insertion
   */
  void slotAttachFileData(KIO::Job *, const TQByteArray &);
  void slotAttachFileResult(KIO::Job *);

  void slotListAction(const TQString &);
  void slotFontAction(const TQString &);
  void slotSizeAction(int);
  void slotAlignLeft();
  void slotAlignCenter();
  void slotAlignRight();
  void slotTextBold();
  void slotTextItalic();
  void slotTextUnder();
  void slotFormatReset();
  void slotTextColor();
  void fontChanged( const TQFont & );
  void alignmentChanged( int );

public: // kmkernel, attachmentlistview
  bool addAttach(const KURL url);

public: // kmcommand
  /**
   * Add an attachment to the list.
   */
  void addAttach(const KMMessagePart* msgPart);

private:
  const KPIM::Identity & identity() const;
  uint identityUid() const;
  Kleo::CryptoMessageFormat cryptoMessageFormat() const;
  bool encryptToSelf() const;

signals:
  void applyChangesDone( bool );
  void attachmentAdded( const KURL&, bool success );

private:
  /**
   * Applies the user changes to the message object of the composer
   * and signs/encrypts the message if activated. Returns FALSE in
   * case of an error (e.g. if PGP encryption fails).
   * Disables the controls of the composer window unless @dontDisable
   * is true.
   */
  void applyChanges( bool dontSignNorEncrypt, bool dontDisable=false );

  /**
   * Install grid management and header fields. If fields exist that
   * should not be there they are removed. Those that are needed are
   * created if necessary.
   */
  void rethinkFields(bool fromslot=false);

  /**
    Connect signals for moving focus by arrow keys. Returns next edit.
  */
  TQWidget *connectFocusMoving( TQWidget *prev, TQWidget *next );

  /**
   * Show or hide header lines
   */

  void rethinkHeaderLine( int aValue, int aMask, int& aRow,
                          TQLabel* aLbl,
                          TQLineEdit* aEdt, TQPushButton* aBtn = 0,
                          const TQString &toolTip = TQString::null,
                          const TQString &whatsThis = TQString::null );

  void rethinkHeaderLine( int value, int mask, int& row,
                          TQLabel* lbl, TQComboBox* cbx, TQCheckBox *chk );

  /**
   * Checks how many recipients are and warns if there are too many.
   * @return true, if the user accepted the warning and the message should be sent
   */
  bool checkRecipientNumber() const;


  bool checkTransport() const;

  /**
   * Initialization methods
   */
  void setupActions();
  void setupStatusBar();
  void setupEditor();


  /**
   * Header fields.
   */
  TQString subject() const;
  TQString to() const;
  TQString cc() const;
  TQString bcc() const;
  TQString from() const;
  TQString replyTo() const;

  /**
   * Use the given folder as sent-mail folder if the given folder exists.
   * Else show an error message and use the default sent-mail folder as
   * sent-mail folder.
   */
  void setFcc( const TQString &idString );

  /**
   * Ask for confirmation if the message was changed before close.
   */
  virtual bool queryClose ();
  /**
   * prevent kmail from exiting when last window is deleted (kernel rules)
  */
  virtual bool queryExit ();

  /**
   * Open the attachment with the given index and with ("Open with")
   */
  void openAttach( int index, bool with );

  /**
   * View the attachment with the given index.
   */
  void viewAttach( int index );

  /**
    Edit the attachment with the given index.
  */
  void editAttach( int index, bool openWith );

  /**
   * Remove an attachment from the list.
   */
   void removeAttach(const TQString &url);
   void removeAttach(int idx);

   /**
    * Updates an item in the TQListView to represnet a given message part
    */
   void msgPartToItem(const KMMessagePart* msgPart, KMAtmListViewItem *lvi,
        bool loadDefaults = true );

  /**
   * Open addressbook and append selected addresses to the given
   * edit field.
   */
  void addrBookSelInto();

  void addrBookSelIntoOld();
  void addrBookSelIntoNew();

private:
  /**
   * Turn encryption on/off. If setByUser is true then a message box is shown
   * in case encryption isn't possible.
   */
  void setEncryption( bool encrypt, bool setByUser = false );

  /**
   * Turn signing on/off. If setByUser is true then a message box is shown
   * in case signing isn't possible.
   */
  void setSigning( bool sign, bool setByUser = false );

  /**
     Returns true if the user forgot to attach something.
  */
  bool userForgotAttachment();

  /**
   * Retrieve encrypt flag of an attachment
   * ( == state of it's check box in the attachments list view )
   */
  bool encryptFlagOfAttachment(int idx);

  /**
   * Retrieve sign flag of an attachment
   * ( == state of it's check box in the attachments list view )
   */
  bool signFlagOfAttachment(int idx);


  /**
   * Decrypt an OpenPGP block or strip off the OpenPGP envelope of a text
   * block with a clear text signature. This is only done if the given
   * string contains exactly one OpenPGP block.
   * This function is for example used to restore the unencrypted/unsigned
   * message text for editting.
   */
  static void decryptOrStripOffCleartextSignature( TQCString& );

  /**
   * Save the message into the Drafts or Templates folder.
   */
  bool saveDraftOrTemplate( const TQString &folderName, KMMessage *msg );

  /**
   * Send the message. Returns true if the message was sent successfully.
   */
  enum SaveIn { None, Drafts, Templates };
  void doSend( KMail::MessageSender::SendMethod method=KMail::MessageSender::SendDefault,
               KMComposeWin::SaveIn saveIn = KMComposeWin::None );

  /**
   * Returns the autosave interval in milliseconds (as needed for TQTimer).
   */
  int autoSaveInterval() const;

  /**
   * Initialize autosaving (timer and filename).
   */
  void initAutoSave();

  /**
   * Enables/disables autosaving depending on the value of the autosave
   * interval.
   */
  void updateAutoSave();

  /**
   * Stop autosaving and delete the autosaved message.
   */
  void cleanupAutoSave();

  /**
   * Validates a list of email addresses.
   * @return true if all addresses are valid.
   * @return false if one or several addresses are invalid.
   */
  static bool validateAddresses( TQWidget * parent, const TQString & addresses );

  /**
   * Sets the transport combobox to @p transport. If @p transport is empty
   * then the combobox remains unchanged. If @p transport is neither a known transport
   * nor a custom transport then the combobox is set to the default transport.
   * @param transport the transport the combobox should be set to
   */
  void setTransport( const TQString & transport );

  enum SignaturePlacement { Append, Prepend, AtCursor };

  /**
   * Helper to insert the signature of the current identy at the
   * beginning or end of the editor.
   */
  void insertSignature( SignaturePlacement placement = Append );
private slots:
   /**
    * Compress an attachemnt with the given index
    */
    void compressAttach(int idx);
    void uncompressAttach(int idx);
    void editorFocusChanged(bool gained);
    void recipientEditorSizeHintChanged();
    void setMaximumHeaderSize();

private:
  TQWidget   *mMainWidget;
  TQComboBox *mTransport;
  KMail::DictionaryComboBox *mDictionaryCombo;
  KPIM::IdentityCombo    *mIdentity;
  KMFolderComboBox *mFcc;
  KMLineEdit *mEdtFrom, *mEdtReplyTo, *mEdtTo, *mEdtCc, *mEdtBcc;
  KMLineEditSpell *mEdtSubject;
  TQLabel    *mLblIdentity, *mLblTransport, *mLblFcc;
  TQLabel    *mLblFrom, *mLblReplyTo, *mLblTo, *mLblCc, *mLblBcc, *mLblSubject;
  TQLabel    *mDictionaryLabel;
  TQCheckBox *mBtnIdentity, *mBtnDictionary, *mBtnTransport, *mBtnFcc;
  TQPushButton *mBtnTo, *mBtnCc, *mBtnBcc, /* *mBtnFrom, */ *mBtnReplyTo;
  bool mSpellCheckInProgress;
  bool mDone;
  bool mAtmModified;
  TQListViewItem *mAtmSelectNew;

  KMEdit* mEditor;
  TQGridLayout* mGrid;
  KMMessage *mMsg;
  TQValueVector<KMMessage*> mComposedMessages;
  KMail::AttachmentListView* mAtmListView;
  int mAtmColEncrypt;
  int mAtmColSign;
  int mAtmColCompress;
  int mAtmEncryptColWidth;
  int mAtmSignColWidth;
  int mAtmCompressColWidth;
  TQPtrList<TQListViewItem> mAtmItemList;
  TQPtrList<KMMessagePart> mAtmList;
  TQPopupMenu *mAttachMenu;
  int mOpenId, mOpenWithId, mViewId, mRemoveId, mSaveAsId, mPropertiesId, mEditId, mEditWithId;
  bool mAutoDeleteMsg;
  bool mSigningAndEncryptionExplicitlyDisabled;
  bool mLastSignActionState, mLastEncryptActionState;
  bool mLastIdentityHasSigningKey, mLastIdentityHasEncryptionKey;
  KMFolder *mFolder;
  long mShowHeaders;
  bool mConfirmSend;
  bool mDisableBreaking; // Move
  int mNumHeaders;
  bool mUseHTMLEditor;
  bool mHtmlMarkup;
  TQFont mBodyFont, mFixedFont;
  TQPtrList<KTempFile> mAtmTempList;
  TQPalette mPalette;
  uint mId;
  TQString mOldSigText;

  KAction *mAttachPK, *mAttachMPK,
          *mAttachRemoveAction, *mAttachSaveAction, *mAttachPropertiesAction,
          *mPasteQuotation, *mAddQuoteChars, *mRemQuoteChars;
  KRecentFilesAction *mRecentAction;

  KAction *mAppendSignatureAction, *mPrependSignatureAction, *mInsertSignatureAction;

  KToggleAction *mSignAction, *mEncryptAction, *mRequestMDNAction;
  KToggleAction *mUrgentAction, *mAllFieldsAction, *mFromAction;
  KToggleAction *mReplyToAction, *mToAction, *mCcAction, *mBccAction;
  KToggleAction *mSubjectAction;
  KToggleAction *mIdentityAction, *mTransportAction, *mFccAction;
  KToggleAction *mWordWrapAction, *mFixedFontAction, *mAutoSpellCheckingAction;
  KToggleAction *mDictionaryAction, *mSnippetAction;

  KSelectAction *listAction;
  KFontAction *fontAction;
  KFontSizeAction *fontSizeAction;
  KToggleAction *alignLeftAction, *alignCenterAction, *alignRightAction;
  KToggleAction *textBoldAction, *textItalicAction, *textUnderAction;
  KToggleAction *plainTextAction, *markupAction;
  KAction *actionFormatColor, *actionFormatReset;
  KAction *mHtmlToolbar;

  KSelectAction *mEncodingAction;
  KSelectAction *mCryptoModuleAction;

  TQCString mCharset;
  TQCString mDefCharset;
  TQStringList mCharsets;
  bool mAutoCharset;

  bool mAlwaysSend;

  TQStringList mFolderNames;
  TQValueList<TQGuardedPtr<KMFolder> > mFolderList;
  TQMap<KIO::Job*, KURL> mAttachJobs;
  KURL::List mAttachFilesPending;
  int mAttachFilesSend;

private:
  // helper method for slotInsert(My)PublicKey()
  void startPublicKeyExport();
  bool canSignEncryptAttachments() const {
    return cryptoMessageFormat() != Kleo::InlineOpenPGPFormat;
  }

  bool mSubjectTextWasSpellChecked;

  TQString addQuotesToText( const TQString &inputText );
  TQString removeQuotesFromText( const TQString &inputText );
  // helper method for rethinkFields
  int calcColumnWidth(int which, long allShowing, int width);

private slots:
  void slotCompletionModeChanged( KGlobalSettings::Completion );
  void slotConfigChanged();

  void slotComposerDone( bool );

  void slotContinueDoSend( bool );
  void slotContinuePrint( bool );
  void slotContinueAutoSave();

  void slotEncryptChiasmusToggled( bool );

  /**
   * Helper method (you could call is a bottom-half :) for
   * startPublicKeyExport()
   */
  void slotPublicKeyExportResult( const GpgME::Error & err, const TQByteArray & keydata );

  /**
   *  toggle automatic spellchecking
   */
  void slotAutoSpellCheckingToggled(bool);

  /**
   *  Updates signature actions when identity changes.
   */
  void slotUpdateSignatureActions();

  /**
   * Updates the visibility and text of the signature and encryption state indicators.
   */
  void slotUpdateSignatureAndEncrypionStateIndicators();
private:
  TQColor mForeColor,mBackColor;
  TQFont mSaveFont;
  TQSplitter *mHeadersToEditorSplitter;
  TQWidget* mHeadersArea;
  TQSplitter *mSplitter;
  TQSplitter *mSnippetSplitter;
  struct atmLoadData
  {
    KURL url;
    TQByteArray data;
    bool insert;
    TQCString encoding;
  };
  TQMap<KIO::Job *, atmLoadData> mMapAtmLoadData;

  // These are for passing on methods over the applyChanges calls
  KMail::MessageSender::SendMethod mSendMethod;
  KMComposeWin::SaveIn mSaveIn;

  KToggleAction *mEncryptChiasmusAction;
  bool mEncryptWithChiasmus;

  // This is the temporary object that constructs the message out of the
  // window
  MessageComposer* mComposer;

  // Temp var for slotPrint:
  bool mMessageWasModified;

  // Temp var for slotInsert(My)PublicKey():
  TQString mFingerprint;

  // Temp ptr for saving image from clipboard
  KTempDir *mTempDir;

  bool mClassicalRecipients;

  RecipientsEditor *mRecipientsEditor;
  int mLabelWidth;

  TQTimer *mAutoSaveTimer;
  TQString mAutoSaveFilename;
  int mLastAutoSaveErrno; // holds the errno of the last try to autosave

  TQPopupMenu *mActNowMenu;
  TQPopupMenu *mActLaterMenu;

  TQMap<KMail::EditorWatcher*, KMMessagePart*> mEditorMap;
  TQMap<KMail::EditorWatcher*, KTempFile*> mEditorTempFiles;

  TQLabel *mSignatureStateIndicator;
  TQLabel *mEncryptionStateIndicator;

  SnippetWidget *mSnippetWidget;
  std::set<KTempDir*> mTempDirs;

  /** If the message in this composer has a cursor position set (for
   *   instance because it comes from a template containing %CURSOR)
   *   then we need to preserve that cursor position even when auto-
   *   appending (or prepending) the signature during composer setup.
   *   Set to true *once* (and only in setMsg() at that) to avoid
   *   accidentally moving the cursor.
   */
  bool mPreserveUserCursorPosition;

  bool mPreventFccOverwrite;
  bool mCheckForRecipients;
  bool mCheckForForgottenAttachments;
  bool mIgnoreStickyFields;
};

#endif

