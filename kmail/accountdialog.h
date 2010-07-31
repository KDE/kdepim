/*   -*- c++ -*-
 *   accountdialog.h
 *
 *   kmail: KDE mail client
 *   This file: Copyright (C) 2000 Espen Sand, espen@kde.org
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef _ACCOUNT_DIALOG_H_
#define _ACCOUNT_DIALOG_H_

#include <kdialogbase.h>
#include <klistview.h>
#include <klineedit.h>
#include <tqguardedptr.h>
#include "imapaccountbase.h"

class QRegExpValidator;
class QCheckBox;
class QComboBox;
class QPushButton;
class QLabel;
class QLineEdit;
class QRadioButton;
class QToolButton;
class KIntNumInput;
class KMAccount;
class KMFolder;
class KMServerTest;
class QButtonGroup;

namespace KPIM {
class IdentityCombo;
}

namespace KMail {

class SieveConfigEditor;
class FolderRequester;

class AccountDialog : public KDialogBase
{
  Q_OBJECT

  public:
    AccountDialog( const TQString & caption, KMAccount *account,
		   TQWidget *parent=0, const char *name=0, bool modal=true );
    virtual ~AccountDialog();
  private:
    struct LocalWidgets
    {
      TQLabel       *titleLabel;
      TQLineEdit    *nameEdit;
      TQComboBox    *locationEdit;
      TQRadioButton *lockMutt;
      TQRadioButton *lockMuttPriv;
      TQRadioButton *lockProcmail;
      TQComboBox    *procmailLockFileName;
      TQRadioButton *lockFcntl;
      TQRadioButton *lockNone;
      TQLineEdit    *precommand;
#if 0
      TQCheckBox    *resourceCheck;
      TQPushButton  *resourceClearButton;
      TQPushButton  *resourceClearPastButton;
#endif
      TQCheckBox    *includeInCheck;
      TQCheckBox    *intervalCheck;
      TQLabel       *intervalLabel;
      KIntNumInput *intervalSpin;
      TQComboBox    *folderCombo;
      //TQComboBox    *identityCombo;
      KPIM::IdentityCombo    *identityCombo;
      TQLabel       *identityLabel;
    };

    struct MaildirWidgets
    {
      TQLabel       *titleLabel;
      TQLineEdit    *nameEdit;
      TQComboBox    *locationEdit;
      TQLineEdit    *precommand;
#if 0
      TQCheckBox    *resourceCheck;
      TQPushButton  *resourceClearButton;
      TQPushButton  *resourceClearPastButton;
#endif
      TQCheckBox    *includeInCheck;
      TQCheckBox    *intervalCheck;
      TQLabel       *intervalLabel;
      KIntNumInput *intervalSpin;
      TQComboBox    *folderCombo;
      //TQComboBox    *identityCombo;
      KPIM::IdentityCombo    *identityCombo;
      TQLabel       *identityLabel;
    };

    struct PopWidgets
    {
      TQLabel       *titleLabel;
      TQLineEdit    *nameEdit;
      TQLineEdit    *loginEdit;
      TQLineEdit    *passwordEdit;
      TQLineEdit    *hostEdit;
      TQLineEdit    *portEdit;
      TQLineEdit    *precommand;
      TQButtonGroup *encryptionGroup;
      TQRadioButton *encryptionNone;
      TQRadioButton *encryptionSSL;
      TQRadioButton *encryptionTLS;
      TQButtonGroup *authGroup;
      TQRadioButton *authUser;
      TQRadioButton *authPlain;
      TQRadioButton *authLogin;
      TQRadioButton *authCRAM_MD5;
      TQRadioButton *authDigestMd5;
      TQRadioButton *authNTLM;
      TQRadioButton *authGSSAPI;
      TQRadioButton *authAPOP;

      TQPushButton  *checkCapabilities;
      TQCheckBox    *usePipeliningCheck;
      TQCheckBox    *storePasswordCheck;
      TQCheckBox    *leaveOnServerCheck;
      TQCheckBox    *leaveOnServerDaysCheck;
      KIntNumInput *leaveOnServerDaysSpin;
      TQCheckBox    *leaveOnServerCountCheck;
      KIntNumInput *leaveOnServerCountSpin;
      TQCheckBox    *leaveOnServerSizeCheck;
      KIntNumInput *leaveOnServerSizeSpin;
#if 0
      TQCheckBox    *resourceCheck;
      TQPushButton  *resourceClearButton;
      TQPushButton  *resourceClearPastButton;
#endif
      TQCheckBox    *includeInCheck;
      TQCheckBox    *intervalCheck;
      TQCheckBox    *filterOnServerCheck;
      TQLabel       *intervalLabel;
      KIntNumInput *intervalSpin;
      KIntNumInput *filterOnServerSizeSpin;
      TQComboBox    *folderCombo;
      //TQComboBox    *identityCombo;
      KPIM::IdentityCombo    *identityCombo;
      TQLabel       *identityLabel;
    };

    struct ImapWidgets
    {
      TQLabel       *titleLabel;
      TQLineEdit    *nameEdit;
      TQLineEdit    *loginEdit;
      TQLineEdit    *passwordEdit;
      TQLineEdit    *hostEdit;
      TQLineEdit    *portEdit;
#if 0
      TQCheckBox    *resourceCheck;
      TQPushButton  *resourceClearButton;
      TQPushButton  *resourceClearPastButton;
#endif
      TQCheckBox    *autoExpungeCheck;     // only used by normal (online) IMAP
      TQCheckBox    *hiddenFoldersCheck;
      TQCheckBox    *subscribedFoldersCheck;
      TQCheckBox    *locallySubscribedFoldersCheck;
      TQCheckBox    *loadOnDemandCheck;
      TQCheckBox    *storePasswordCheck;
      TQCheckBox    *progressDialogCheck;  // only used by Disconnected IMAP
      TQCheckBox    *includeInCheck;
      TQCheckBox    *intervalCheck;
      TQCheckBox    *listOnlyOpenCheck;
      TQLabel       *intervalLabel;
      KIntNumInput *intervalSpin;
      TQButtonGroup *encryptionGroup;
      TQRadioButton *encryptionNone;
      TQRadioButton *encryptionSSL;
      TQRadioButton *encryptionTLS;
      TQButtonGroup *authGroup;
      TQRadioButton *authUser;
      TQRadioButton *authPlain;
      TQRadioButton *authLogin;
      TQRadioButton *authCramMd5;
      TQRadioButton *authDigestMd5;
      TQRadioButton *authGSSAPI;
      TQRadioButton *authNTLM;
      TQRadioButton *authAnonymous;
      TQPushButton  *checkCapabilities;
      FolderRequester *trashCombo;
      KLineEdit    *personalNS;
      KLineEdit    *otherUsersNS;
      KLineEdit    *sharedNS;
      TQToolButton  *editPNS;
      TQToolButton  *editONS;
      TQToolButton  *editSNS;
      ImapAccountBase::nsDelimMap nsMap;
      KPIM::IdentityCombo    *identityCombo;
      TQLabel       *identityLabel;
    };

  private slots:
    virtual void slotOk();
    void slotLocationChooser();
    void slotMaildirChooser();
    void slotEnablePopInterval( bool state );
    void slotEnableImapInterval( bool state );
    void slotEnableLocalInterval( bool state );
    void slotEnableMaildirInterval( bool state );
    void slotFontChanged();
    void slotLeaveOnServerClicked();
    void slotEnableLeaveOnServerDays( bool state );
    void slotEnableLeaveOnServerCount( bool state );
    void slotEnableLeaveOnServerSize( bool state );
    void slotFilterOnServerClicked();
    void slotPipeliningClicked();
    void slotPopEncryptionChanged(int);
    void slotImapEncryptionChanged(int);
    void slotCheckPopCapabilities();
    void slotCheckImapCapabilities();
    void slotPopCapabilities( const TQStringList &, const TQStringList & );
    void slotImapCapabilities( const TQStringList &, const TQStringList & );
    void slotReloadNamespaces();
    void slotSetupNamespaces( const ImapAccountBase::nsDelimMap& map );
    void slotEditPersonalNamespace();
    void slotEditOtherUsersNamespace();
    void slotEditSharedNamespace();
    void slotConnectionResult( int errorCode, const TQString& );
    void slotLeaveOnServerDaysChanged( int value );
    void slotLeaveOnServerCountChanged( int value );
    void slotFilterOnServerSizeChanged( int value );
#if 0
    // Moc doesn't understand #if 0, so they are also commented out
    // void slotClearResourceAllocations();
    // void slotClearPastResourceAllocations();
#endif

  private:
    void makeLocalAccountPage();
    void makeMaildirAccountPage();
    void makePopAccountPage();
    void makeImapAccountPage( bool disconnected = false );
    void setupSettings();
    void saveSettings();
    void checkHighest( TQButtonGroup * );
    static unsigned int popCapabilitiesFromStringList( const TQStringList & );
    static unsigned int imapCapabilitiesFromStringList( const TQStringList & );
    void enablePopFeatures( unsigned int );
    void enableImapAuthMethods( unsigned int );
    void initAccountForConnect();
    const TQString namespaceListToString( const TQStringList& list );

  private:
    LocalWidgets mLocal;
    MaildirWidgets mMaildir;
    PopWidgets   mPop;
    ImapWidgets  mImap;
    KMAccount    *mAccount;
    TQValueList<TQGuardedPtr<KMFolder> > mFolderList;
    TQStringList  mFolderNames;
    KMServerTest *mServerTest;
    enum EncryptionMethods {
      NoEncryption = 0,
      SSL = 1,
      TLS = 2
    };
    enum Capabilities {
      Plain      =   1,
      Login      =   2,
      CRAM_MD5   =   4,
      Digest_MD5 =   8,
      Anonymous  =  16,
      APOP       =  32,
      Pipelining =  64,
      TOP        = 128,
      UIDL       = 256,
      STLS       = 512, // TLS for POP
      STARTTLS   = 512, // TLS for IMAP
      GSSAPI     = 1024,
      NTLM       = 2048,
      AllCapa    = 0xffffffff
    };
    unsigned int mCurCapa;
    unsigned int mCapaNormal;
    unsigned int mCapaSSL;
    unsigned int mCapaTLS;
    KMail::SieveConfigEditor *mSieveConfigEditor;
    TQRegExpValidator *mValidator;
};

class NamespaceLineEdit: public KLineEdit
{
  Q_OBJECT

  public:
    NamespaceLineEdit( TQWidget* parent );

    const TQString& lastText() { return mLastText; }

  public slots:
    virtual void setText ( const TQString & );

  private:
    TQString mLastText;
};

class NamespaceEditDialog: public KDialogBase
{
  Q_OBJECT

  public:
    NamespaceEditDialog( TQWidget* parent, ImapAccountBase::imapNamespace type,
        ImapAccountBase::nsDelimMap* map );

  protected slots:
    void slotOk();
    void slotRemoveEntry( int );

  private:
    ImapAccountBase::imapNamespace mType;
    ImapAccountBase::nsDelimMap* mNamespaceMap;
    ImapAccountBase::namespaceDelim mDelimMap;
    TQMap<int, NamespaceLineEdit*> mLineEditMap;
    TQButtonGroup* mBg;
};

} // namespace KMail

#endif
