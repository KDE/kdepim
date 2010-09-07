// -*- c++ -*-
// configuredialog_p.h: classes internal to ConfigureDialog
// see configuredialog.h for details.

#ifndef _CONFIGURE_DIALOG_PRIVATE_H_
#define _CONFIGURE_DIALOG_PRIVATE_H_

#include <klineedit.h>
#include <tqcombobox.h>
#include <tqguardedptr.h>
#include <tqptrlist.h>
#include <tqstring.h>
#include <tqvaluelist.h>
#include <tqstringlist.h>
#include <dcopobject.h>

#include <kdialogbase.h>
#include <klistview.h>
#include <kcmodule.h>
#include <klocale.h>
#include <kdepimmacros.h>

class TQPushButton;
class TQLabel;
class TQCheckBox;
class KURLRequester;
class KFontChooser;
class TQRadioButton;
class ColorListBox;
class TQFont;
class TQListViewItem;
class TQTabWidget;
class TQListBox;
class TQButtonGroup;
class TQRegExpValidator;
class TQVBox;
class KMAccount;
class KMTransportInfo;
class ListView;
class ConfigureDialog;
class KIntSpinBox;
class SimpleStringListEditor;
class KConfig;
class TQPoint;
class ComposerCryptoConfiguration;
class WarningConfiguration;
class SMimeConfiguration;
class TemplatesConfiguration;
class CustomTemplates;
class TQGroupBox;
class TQVGroupBox;
#include <tqdict.h>
class TQLineEdit;
class KMMsgTagDesc;
class KListBox;
class KColorCombo;
class KFontRequester;
class KIconButton;
class KKeyButton;
class TQSpinBox;
class KComboBox;

namespace Kpgp {
  class Config;
}
namespace KMail {
  class IdentityDialog;
  class IdentityListView;
  class AccountComboBox;
  class FolderRequester;
}
namespace Kleo {
  class BackendConfigWidget;
  class CryptoConfig;
  class CryptoConfigEntry;
}

class NewIdentityDialog : public KDialogBase
{
  Q_OBJECT

public:
  enum DuplicateMode { Empty, ControlCenter, ExistingEntry };

  NewIdentityDialog( const TQStringList & identities,
                     TQWidget *parent=0, const char *name=0, bool modal=true );

  TQString identityName() const { return mLineEdit->text(); }
  TQString duplicateIdentity() const { return mComboBox->currentText(); }
  DuplicateMode duplicateMode() const;

protected slots:
  virtual void slotEnableOK( const TQString & );

private:
  TQLineEdit  *mLineEdit;
  TQComboBox  *mComboBox;
  TQButtonGroup *mButtonGroup;
};


//
//
// Language item handling
//
//

struct LanguageItem
{
  LanguageItem() {}
  LanguageItem( const TQString & language, const TQString & reply=TQString::null,
                const TQString & replyAll=TQString::null,
                const TQString & forward=TQString::null,
                const TQString & indentPrefix=TQString::null ) :
    mLanguage( language ), mReply( reply ), mReplyAll( replyAll ),
    mForward( forward ), mIndentPrefix( indentPrefix ) {}

  TQString mLanguage, mReply, mReplyAll, mForward, mIndentPrefix;
};

typedef TQValueList<LanguageItem> LanguageItemList;

class NewLanguageDialog : public KDialogBase
{
  Q_OBJECT

  public:
    NewLanguageDialog( LanguageItemList & suppressedLangs, TQWidget *parent=0,
                       const char *name=0, bool modal=true );
    TQString language() const;

  private:
    TQComboBox *mComboBox;
};


class LanguageComboBox : public QComboBox
{
  Q_OBJECT

  public:
    LanguageComboBox( bool rw, TQWidget *parent=0, const char *name=0 );
    int insertLanguage( const TQString & language );
    TQString language() const;
    void setLanguage( const TQString & language );
};

//
//
// Profile dialog
//
//

class ProfileDialog : public KDialogBase {
  Q_OBJECT
public:
  ProfileDialog( TQWidget * parent=0, const char * name=0, bool modal=false );

signals:
  void profileSelected( KConfig * profile );

private slots:
  void slotSelectionChanged();
  void slotOk();

private:
  void setup();

private:
  KListView   *mListView;
  TQStringList mProfileList;
};

#include <kdialog.h>
class ConfigModule : public KCModule {
  Q_OBJECT
public:
  ConfigModule( TQWidget * parent=0, const char * name=0 )
     : KCModule ( parent, name )
     {}
  ~ConfigModule() {}

  virtual void load() = 0;
  virtual void save() = 0;
  virtual void defaults() {}

  /** Should return the help anchor for this page or tab */
  virtual TQString helpAnchor() const = 0;

signals:
  /** Emitted when the installation of a profile is
      requested. All connected kcms should load the values
      from the profile only for those entries that
      really have keys defined in the profile.
   */
   void installProfile( KConfig * profile );

};


// Individual tab of a ConfigModuleWithTabs
class ConfigModuleTab : public TQWidget {
  Q_OBJECT
public:
   ConfigModuleTab( TQWidget *parent=0, const char* name=0 )
      :TQWidget( parent, name )
      {}
  ~ConfigModuleTab() {}
  void load();
  virtual void save() = 0;
  void defaults();
  // the below are optional
  virtual void installProfile(){}
signals:
   // forwarded to the ConfigModule
  void changed(bool);
public slots:
  void slotEmitChanged();
private:
  // reimplement this for loading values of settings which are available
  // via GlobalSettings
  virtual void doLoadFromGlobalSettings() {}
  // reimplement this for loading values of settings which are not available
  // via GlobalSettings
  virtual void doLoadOther() {}
  // reimplement this for loading default values of settings which are
  // not available via GlobalSettings (KConfigXT).
  virtual void doResetToDefaultsOther() {}
};


/*
 * ConfigModuleWithTabs represents a kcm with several tabs.
 * It simply forwards load and save operations to all tabs.
 */
class ConfigModuleWithTabs : public ConfigModule {
  Q_OBJECT
public:
  ConfigModuleWithTabs( TQWidget * parent=0, const char * name=0 );
   ~ConfigModuleWithTabs() {}

  // don't reimplement any of those methods
  virtual void load();
  virtual void save();
  virtual void defaults();
  virtual void installProfile( KConfig * profile );

protected:
  void addTab( ConfigModuleTab* tab, const TQString & title );

private:
  TQTabWidget *mTabWidget;

};


//
//
// IdentityPage
//
//

class KDE_EXPORT IdentityPage : public ConfigModule {
  Q_OBJECT
public:
  IdentityPage( TQWidget * parent=0, const char * name=0 );
  ~IdentityPage() {}

  TQString helpAnchor() const;

  void load();
  void save();

public slots:
  void slotUpdateTransportCombo( const TQStringList & );

private slots:
  void slotNewIdentity();
  void slotModifyIdentity();
  void slotRemoveIdentity();
  /** Connected to @p mRenameButton's clicked() signal. Just does a
      KListView::rename on the selected item */
  void slotRenameIdentity();
  /** connected to @p mIdentityList's renamed() signal. Validates the
      new name and sets it in the KPIM::IdentityManager */
  void slotRenameIdentity( TQListViewItem *, const TQString &, int );
  void slotContextMenu( KListView*, TQListViewItem *, const TQPoint & );
  void slotSetAsDefault();
  void slotIdentitySelectionChanged();

private: // methods
  void refreshList();

private: // data members
  KMail::IdentityDialog   * mIdentityDialog;
  int            mOldNumberOfIdentities;

  KMail::IdentityListView * mIdentityList;
  TQPushButton             * mModifyButton;
  TQPushButton             * mRenameButton;
  TQPushButton             * mRemoveButton;
  TQPushButton             * mSetAsDefaultButton;
};


//
//
// AccountsPage
//
//

// subclasses: one class per tab:
class AccountsPageSendingTab : public ConfigModuleTab {
  Q_OBJECT
public:
  AccountsPageSendingTab( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;
  void save();

signals:
  void transportListChanged( const TQStringList & );

private slots:
  void slotTransportSelected();
  void slotAddTransport();
  void slotModifySelectedTransport();
  void slotRemoveSelectedTransport();
  void slotSetDefaultTransport();

private:
  virtual void doLoadFromGlobalSettings();
  virtual void doLoadOther();
  //FIXME virtual void doResetToDefaultsOther();

private:
  ListView    *mTransportList;
  TQPushButton *mModifyTransportButton;
  TQPushButton *mRemoveTransportButton;
  TQPushButton *mSetDefaultTransportButton;
  TQCheckBox   *mConfirmSendCheck;
  TQComboBox   *mSendOnCheckCombo;
  TQComboBox   *mSendMethodCombo;
  TQComboBox   *mMessagePropertyCombo;
  TQLineEdit   *mDefaultDomainEdit;

  TQPtrList< KMTransportInfo > mTransportInfoList;
};


class AccountsPageReceivingTab : public ConfigModuleTab {
  Q_OBJECT
public:
  AccountsPageReceivingTab( TQWidget * parent=0, const char * name=0 );
  ~AccountsPageReceivingTab();
  TQString helpAnchor() const;
  void save();

signals:
  void accountListChanged( const TQStringList & );

private slots:
  void slotAccountSelected();
  void slotAddAccount();
  void slotModifySelectedAccount();
  void slotRemoveSelectedAccount();
  void slotEditNotifications();
  void slotTweakAccountList();

private:
  virtual void doLoadFromGlobalSettings();
  virtual void doLoadOther();
  //FIXME virtual void doResetToDefaultsOther();
  TQStringList occupiedNames();

private:
  ListView      *mAccountList;
  TQPushButton   *mModifyAccountButton;
  TQPushButton   *mRemoveAccountButton;
  TQCheckBox     *mBeepNewMailCheck;
  TQCheckBox     *mVerboseNotificationCheck;
  TQCheckBox     *mCheckmailStartupCheck;
  TQPushButton   *mOtherNewMailActionsButton;

  TQValueList< TQGuardedPtr<KMAccount> > mAccountsToDelete;
  TQValueList< TQGuardedPtr<KMAccount> > mNewAccounts;
  struct ModifiedAccountsType {
    TQGuardedPtr< KMAccount > oldAccount;
    TQGuardedPtr< KMAccount > newAccount;
  };
  // ### make this value-based:
  TQValueList< ModifiedAccountsType* >  mModifiedAccounts;
};

class KDE_EXPORT AccountsPage : public ConfigModuleWithTabs {
  Q_OBJECT
public:
  AccountsPage( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;


  // hrmpf. moc doesn't like nested classes with slots/signals...:
  typedef AccountsPageSendingTab SendingTab;
  typedef AccountsPageReceivingTab ReceivingTab;

signals:
  void transportListChanged( const TQStringList & );
  void accountListChanged( const TQStringList & );

private:
  SendingTab   *mSendingTab;
  ReceivingTab *mReceivingTab;
};


//
//
// AppearancePage
//
//

class AppearancePageFontsTab : public ConfigModuleTab {
  Q_OBJECT
public:
  AppearancePageFontsTab( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;
  void save();

  void installProfile( KConfig * profile );

private slots:
  void slotFontSelectorChanged( int );

private:
  //virtual void doLoadFromGlobalSettings();
  virtual void doLoadOther();
  //FIXME virtual void doResetToDefaultsOther();
  void updateFontSelector();

private:
  TQCheckBox    *mCustomFontCheck;
  TQComboBox    *mFontLocationCombo;
  KFontChooser *mFontChooser;

  int          mActiveFontIndex;
  TQFont        mFont[14];
};

class AppearancePageColorsTab : public ConfigModuleTab {
  Q_OBJECT
public:
  AppearancePageColorsTab( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;
  void save();

  void installProfile( KConfig * profile );

private:
  //virtual void doLoadFromGlobalSettings();
  virtual void doLoadOther();
  //FIXME virtual void doResetToDefaultsOther();

private:
  TQCheckBox    *mCustomColorCheck;
  ColorListBox *mColorList;
  TQCheckBox    *mRecycleColorCheck;
  TQSpinBox     *mCloseToQuotaThreshold;
};

class AppearancePageLayoutTab : public ConfigModuleTab {
  Q_OBJECT
public:
  AppearancePageLayoutTab( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;

  void save();
  void installProfile( KConfig * profile );

private:
  //virtual void doLoadFromGlobalSettings();
  virtual void doLoadOther();
  //FIXME virtual void doResetToDefaultsOther();

private: // data
  TQButtonGroup *mFolderListGroup;
  TQButtonGroup *mMIMETreeLocationGroup;
  TQButtonGroup *mMIMETreeModeGroup;
  TQButtonGroup *mReaderWindowModeGroup;
  TQCheckBox *mFavoriteFolderViewCB;
};

class AppearancePageHeadersTab : public ConfigModuleTab {
  Q_OBJECT
public:
  AppearancePageHeadersTab( TQWidget * parent=0, const char * name=0 );

  TQString helpAnchor() const;

  void save();
  void installProfile( KConfig * profile );

private: // methods
  //virtual void doLoadFromGlobalSettings();
  virtual void doLoadOther();
  //FIXME virtual void doResetToDefaultsOther();
  void setDateDisplay( int id, const TQString & format );

private: // data
  TQCheckBox    *mMessageSizeCheck;
  TQCheckBox    *mAttachmentCheck;
  TQCheckBox    *mNestedMessagesCheck;
  TQCheckBox    *mCryptoIconsCheck;
  TQButtonGroup *mNestingPolicy;
  TQButtonGroup *mDateDisplay;
  TQLineEdit    *mCustomDateFormatEdit;
};

class AppearancePageReaderTab : public ConfigModuleTab {
  Q_OBJECT
public:
  AppearancePageReaderTab( TQWidget * parent=0, const char * name=0 );

  TQString helpAnchor() const;

  void save();
  void installProfile( KConfig * profile );

private:
  virtual void doLoadFromGlobalSettings();
  virtual void doLoadOther();
  //FIXME virtual void doResetToDefaultsOther();
  void readCurrentFallbackCodec();
  void readCurrentOverrideCodec();

private: // data
  TQCheckBox *mCloseAfterReplyOrForwardCheck;
  TQCheckBox *mShowColorbarCheck;
  TQCheckBox *mShowSpamStatusCheck;
  TQCheckBox *mShowEmoticonsCheck;
  TQCheckBox *mShowExpandQuotesMark;
  KIntSpinBox  *mCollapseQuoteLevelSpin;
  TQCheckBox *mShrinkQuotesCheck;
  TQComboBox *mCharsetCombo;
  TQComboBox *mOverrideCharsetCombo;
  TQCheckBox *mShowCurrentTimeCheck;
};


class AppearancePageSystemTrayTab : public ConfigModuleTab {
  Q_OBJECT
public:
  AppearancePageSystemTrayTab( TQWidget * parent=0, const char * name=0 );

  TQString helpAnchor() const;

  void save();
  void installProfile( KConfig * profile );

private:
  virtual void doLoadFromGlobalSettings();

private: // data
  TQCheckBox    *mSystemTrayCheck;
  TQButtonGroup *mSystemTrayGroup;
};

class KDE_EXPORT AppearancePage : public ConfigModuleWithTabs {
  Q_OBJECT
public:
  AppearancePage( TQWidget * parent=0, const char * name=0 );

  TQString helpAnchor() const;

  // hrmpf. moc doesn't like nested classes with slots/signals...:
  typedef AppearancePageFontsTab FontsTab;
  typedef AppearancePageColorsTab ColorsTab;
  typedef AppearancePageLayoutTab LayoutTab;
  typedef AppearancePageHeadersTab HeadersTab;
  typedef AppearancePageReaderTab ReaderTab;
  typedef AppearancePageSystemTrayTab SystemTrayTab;

private:
  FontsTab      *mFontsTab;
  ColorsTab     *mColorsTab;
  LayoutTab     *mLayoutTab;
  HeadersTab    *mHeadersTab;
  ReaderTab     *mReaderTab;
  SystemTrayTab *mSystemTrayTab;
};

//
//
// Composer Page
//
//

class ComposerPageGeneralTab : public ConfigModuleTab {
  Q_OBJECT
public:
  ComposerPageGeneralTab( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;

  void save();
  void installProfile( KConfig * profile );
protected slots:
  void slotConfigureRecentAddresses();
  void slotConfigureCompletionOrder();

private:
  virtual void doLoadFromGlobalSettings();

private:
  TQCheckBox     *mAutoAppSignFileCheck;
  TQCheckBox     *mTopQuoteCheck;
  TQCheckBox     *mSmartQuoteCheck;
  TQCheckBox     *mStripSignatureCheck;
  TQCheckBox     *mQuoteSelectionOnlyCheck;
  TQCheckBox     *mAutoRequestMDNCheck;
  TQCheckBox	*mShowRecentAddressesInComposer;
  TQCheckBox     *mWordWrapCheck;
  KIntSpinBox   *mWrapColumnSpin;
  TQCheckBox     *mRecipientCheck;
  KIntSpinBox   *mRecipientSpin;
  KIntSpinBox   *mAutoSave;
  TQCheckBox     *mExternalEditorCheck;
  KURLRequester *mEditorRequester;
  KComboBox     *mForwardTypeCombo;
};

class ComposerPagePhrasesTab : public ConfigModuleTab {
  Q_OBJECT
public:
  ComposerPagePhrasesTab( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;

  void save();

private slots:
  void slotNewLanguage();
  void slotRemoveLanguage();
  void slotLanguageChanged( const TQString& );
  void slotAddNewLanguage( const TQString& );

private:
  virtual void doLoadFromGlobalSettings();
  void setLanguageItemInformation( int index );
  void saveActiveLanguageItem();

private:
  LanguageComboBox *mPhraseLanguageCombo;
  TQPushButton      *mRemoveButton;
  TQLineEdit        *mPhraseReplyEdit;
  TQLineEdit        *mPhraseReplyAllEdit;
  TQLineEdit        *mPhraseForwardEdit;
  TQLineEdit        *mPhraseIndentPrefixEdit;

  int              mActiveLanguageItem;
  LanguageItemList mLanguageList;
};

class ComposerPageTemplatesTab : public ConfigModuleTab {
  Q_OBJECT
public:
  ComposerPageTemplatesTab( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;

  void save();

private slots:

private:
  virtual void doLoadFromGlobalSettings();

private:
    TemplatesConfiguration* mWidget;
};

class ComposerPageCustomTemplatesTab : public ConfigModuleTab {
  Q_OBJECT
public:
  ComposerPageCustomTemplatesTab( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;

  void save();

private slots:

private:
  virtual void doLoadFromGlobalSettings();

private:
    CustomTemplates* mWidget;
};

class ComposerPageSubjectTab : public ConfigModuleTab {
  Q_OBJECT
public:
  ComposerPageSubjectTab( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;

  void save();

private:
  virtual void doLoadFromGlobalSettings();

private:
  SimpleStringListEditor *mReplyListEditor;
  TQCheckBox              *mReplaceReplyPrefixCheck;
  SimpleStringListEditor *mForwardListEditor;
  TQCheckBox              *mReplaceForwardPrefixCheck;
};

class ComposerPageCharsetTab : public ConfigModuleTab {
  Q_OBJECT
public:
  ComposerPageCharsetTab( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;

  void save();

private slots:
  void slotVerifyCharset(TQString&);

private:
  //virtual void doLoadFromGlobalSettings();
  virtual void doLoadOther();
  //FIXME virtual void doResetToDefaultsOther();

private:
  SimpleStringListEditor *mCharsetListEditor;
  TQCheckBox              *mKeepReplyCharsetCheck;
};

class ComposerPageHeadersTab : public ConfigModuleTab {
  Q_OBJECT
public:
  ComposerPageHeadersTab( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;

  void save();

private slots:
  void slotMimeHeaderSelectionChanged();
  void slotMimeHeaderNameChanged( const TQString & );
  void slotMimeHeaderValueChanged( const TQString & );
  void slotNewMimeHeader();
  void slotRemoveMimeHeader();

private:
  //virtual void doLoadFromGlobalSettings();
  virtual void doLoadOther();
  //FIXME virtual void doResetToDefaultsOther();

private:
  TQCheckBox   *mCreateOwnMessageIdCheck;
  TQLineEdit   *mMessageIdSuffixEdit;
  TQRegExpValidator *mMessageIdSuffixValidator;
  TQListView   *mTagList;
  TQPushButton *mRemoveHeaderButton;
  TQLineEdit   *mTagNameEdit;
  TQLineEdit   *mTagValueEdit;
  TQLabel      *mTagNameLabel;
  TQLabel      *mTagValueLabel;
};

class ComposerPageAttachmentsTab : public ConfigModuleTab {
  Q_OBJECT
public:
  ComposerPageAttachmentsTab( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;

  void save();

private slots:
  void slotOutlookCompatibleClicked();

private:
  virtual void doLoadFromGlobalSettings();
  //FIXME virtual void doResetToDefaultsOther();

private:
  TQCheckBox   *mOutlookCompatibleCheck;
  TQCheckBox   *mMissingAttachmentDetectionCheck;
  SimpleStringListEditor *mAttachWordsListEditor;
};

class KDE_EXPORT ComposerPage : public ConfigModuleWithTabs {
  Q_OBJECT
public:
  ComposerPage( TQWidget * parent=0, const char * name=0 );

  TQString helpAnchor() const;

  // hrmpf. moc doesn't like nested classes with slots/signals...:
  typedef ComposerPageGeneralTab GeneralTab;
  typedef ComposerPagePhrasesTab PhrasesTab;
  typedef ComposerPageTemplatesTab TemplatesTab;
  typedef ComposerPageCustomTemplatesTab CustomTemplatesTab;
  typedef ComposerPageSubjectTab SubjectTab;
  typedef ComposerPageCharsetTab CharsetTab;
  typedef ComposerPageHeadersTab HeadersTab;
  typedef ComposerPageAttachmentsTab AttachmentsTab;

private:
  GeneralTab  *mGeneralTab;
  PhrasesTab  *mPhrasesTab;
  TemplatesTab  *mTemplatesTab;
  CustomTemplatesTab  *mCustomTemplatesTab;
  SubjectTab  *mSubjectTab;
  CharsetTab  *mCharsetTab;
  HeadersTab  *mHeadersTab;
  AttachmentsTab  *mAttachmentsTab;
};

//
//
// SecurityPage
//
//

class SecurityPageGeneralTab : public ConfigModuleTab {
  Q_OBJECT
public:
  SecurityPageGeneralTab( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;

  void save();
  void installProfile( KConfig * profile );

private:
  //virtual void doLoadFromGlobalSettings();
  virtual void doLoadOther();
  //FIXME virtual void doResetToDefaultsOther();

private:
  TQCheckBox    *mExternalReferences;
  TQCheckBox    *mHtmlMailCheck;
  TQCheckBox    *mNoMDNsWhenEncryptedCheck;
  TQButtonGroup *mMDNGroup;
  TQButtonGroup *mOrigQuoteGroup;
  TQCheckBox    *mAutomaticallyImportAttachedKeysCheck;
  TQCheckBox    *mAlwaysDecrypt;
};


class SecurityPageComposerCryptoTab : public ConfigModuleTab {
  Q_OBJECT
public:
  SecurityPageComposerCryptoTab( TQWidget * parent=0, const char * name=0 );

  TQString helpAnchor() const;

  void save();
  void installProfile( KConfig * profile );

private:
  //virtual void doLoadFromGlobalSettings();
  virtual void doLoadOther();
  //FIXME virtual void doResetToDefaultsOther();

private:
  ComposerCryptoConfiguration* mWidget;
};

class SecurityPageWarningTab : public ConfigModuleTab {
  Q_OBJECT
public:
  SecurityPageWarningTab( TQWidget * parent=0, const char * name=0 );

  TQString helpAnchor() const;

  void save();
  void installProfile( KConfig * profile );

private slots:
  void slotReenableAllWarningsClicked();

private:
  //virtual void doLoadFromGlobalSettings();
  virtual void doLoadOther();
  //FIXME virtual void doResetToDefaultsOther();

private:
  WarningConfiguration* mWidget;
};

class SecurityPageSMimeTab : public ConfigModuleTab, public DCOPObject {
  Q_OBJECT
  K_DCOP
public:
  SecurityPageSMimeTab( TQWidget * parent=0, const char * name=0 );
  ~SecurityPageSMimeTab();

  TQString helpAnchor() const;

  // Can't use k_dcop here. dcopidl can't parse this file, dcopidlng has a namespace bug.
  void save();
  void installProfile( KConfig * profile );

private slots:
  void slotUpdateHTTPActions();

private:
  //virtual void doLoadFromGlobalSettings();
  virtual void doLoadOther();
  //FIXME virtual void doResetToDefaultsOther();

private:
  SMimeConfiguration* mWidget;
  Kleo::CryptoConfig* mConfig;
};

class SecurityPageCryptPlugTab : public ConfigModuleTab
{
  Q_OBJECT
public:
  SecurityPageCryptPlugTab( TQWidget * parent = 0, const char* name = 0 );
  ~SecurityPageCryptPlugTab();

  TQString helpAnchor() const;

  void save();

private:
  virtual void doLoadOther();
  //virtual void doResetToDefaultsOther();

private:
  Kleo::BackendConfigWidget * mBackendConfig;
};

class KDE_EXPORT SecurityPage : public ConfigModuleWithTabs {
  Q_OBJECT
public:
  SecurityPage( TQWidget * parent=0, const char * name=0 );

  TQString helpAnchor() const;

  // OpenPGP tab is special:
  void installProfile( KConfig * profile );

  typedef SecurityPageGeneralTab GeneralTab;
  typedef SecurityPageComposerCryptoTab ComposerCryptoTab;
  typedef SecurityPageWarningTab WarningTab;
  typedef SecurityPageSMimeTab SMimeTab;
  typedef SecurityPageCryptPlugTab CryptPlugTab;

private:
  GeneralTab    *mGeneralTab;
  ComposerCryptoTab *mComposerCryptoTab;
  WarningTab    *mWarningTab;
  SMimeTab      *mSMimeTab;
  CryptPlugTab  *mCryptPlugTab;
};


//
//
// MiscPage
//
//

class MiscPageFolderTab : public ConfigModuleTab {
  Q_OBJECT
public:
  MiscPageFolderTab( TQWidget * parent=0, const char * name=0 );

  void save();
 TQString helpAnchor() const;

private:
  virtual void doLoadFromGlobalSettings();
  virtual void doLoadOther();
  //FIXME virtual void doResetToDefaultsOther();

private:
  TQCheckBox    *mEmptyFolderConfirmCheck;
  TQCheckBox    *mExcludeImportantFromExpiry;
  TQComboBox    *mLoopOnGotoUnread;
  TQComboBox    *mMailboxPrefCombo;
  TQComboBox    *mActionEnterFolder;
  TQCheckBox    *mEmptyTrashCheck;
#ifdef HAVE_INDEXLIB
  TQCheckBox    *mIndexingEnabled;
#endif
  TQCheckBox    *mDelayedMarkAsRead;
  KIntSpinBox  *mDelayedMarkTime;
  TQCheckBox    *mShowPopupAfterDnD;
  KMail::FolderRequester *mOnStartupOpenFolder;
  TQComboBox    *mQuotaCmbBox;
};

class MiscPageGroupwareTab : public ConfigModuleTab  {
  Q_OBJECT
public:
  MiscPageGroupwareTab( TQWidget * parent=0, const char * name=0 );
  void save();
  TQString helpAnchor() const;

private slots:
  void slotStorageFormatChanged( int );
  void slotLegacyBodyInvitesToggled( bool on );

private:
  virtual void doLoadFromGlobalSettings();

private:
  TQCheckBox* mEnableGwCB;
  TQCheckBox* mEnableImapResCB;

  TQWidget* mBox;
  TQVBox* gBox;

  TQComboBox* mStorageFormatCombo;
  TQComboBox* mLanguageCombo;

  TQLabel* mFolderComboLabel;
  TQWidgetStack* mFolderComboStack;
  KMail::FolderRequester* mFolderCombo; // in the widgetstack
  KMail::AccountComboBox* mAccountCombo; // in the widgetstack

  TQCheckBox* mHideGroupwareFolders;
  TQCheckBox* mOnlyShowGroupwareFolders;
  TQCheckBox* mSyncImmediately;
  TQCheckBox* mDeleteInvitations;

  TQCheckBox* mLegacyMangleFromTo;
  TQCheckBox* mLegacyBodyInvites;
  TQCheckBox* mExchangeCompatibleInvitations;
  TQCheckBox* mOutlookCompatibleInvitationComments;
  TQCheckBox* mAutomaticSending;
};

class KDE_EXPORT MiscPage : public ConfigModuleWithTabs {
  Q_OBJECT
public:
  MiscPage( TQWidget * parent=0, const char * name=0 );
  TQString helpAnchor() const;

  typedef MiscPageFolderTab FolderTab;
  typedef MiscPageGroupwareTab GroupwareTab;

private:
  FolderTab * mFolderTab;
  GroupwareTab * mGroupwareTab;
};

//
//
// further helper classes:
//
//

class ListView : public KListView {
  Q_OBJECT
public:
  ListView( TQWidget *parent=0, const char *name=0, int visibleItem=10 );
  void resizeColums();

  void setVisibleItem( int visibleItem, bool updateSize=true );
  virtual TQSize sizeHint() const;

protected:
  virtual void resizeEvent( TQResizeEvent *e );
  virtual void showEvent( TQShowEvent *e );

private:
  int mVisibleItem;
};


#endif // _CONFIGURE_DIALOG_PRIVATE_H_
