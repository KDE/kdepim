/*
 * kmtransport.h
 *
 * Copyright (c) 2001-2002 Michael Haeckel <haeckel@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KMTRANSPORT_H_
#define _KMTRANSPORT_H_

#include <kdialogbase.h>

class TQCheckBox;
class TQLabel;
class TQLineEdit;
class TQRadioButton;
class KMServerTest;
class TQButtonGroup;

class KMTransportInfo : public QObject
{
public:
  KMTransportInfo();
  virtual ~KMTransportInfo();
  void readConfig(int id);
  void writeConfig(int id);
  static int findTransport(const TQString &name);
  static TQStringList availableTransports();
  uint id() const { return mId; }

  /** Get/set password for this account */
  TQString passwd() const;
  void setPasswd( const TQString& passwd );

  /** Get/set password storage flag */
  bool storePasswd() const { return mStorePasswd; }
  void setStorePasswd( bool store );

  /** Read password from wallet */
  void readPassword() const;

  TQString type, name, host, port, user, precommand, encryption, authType;
  TQString localHostname;
  bool auth, specifyHostname;

  private:
    mutable TQString mPasswd;
    bool mPasswdDirty, mStorePasswd, mStorePasswdInConfig;
    uint mId;
};

class KMTransportSelDlg : public KDialogBase
{
  Q_OBJECT

public:
  KMTransportSelDlg( TQWidget *parent=0, const char *name=0, bool modal=TRUE );
  int selected() const;

private slots:
  void buttonClicked( int id );

private:
  int mSelectedButton;
};

class KMTransportDialog : public KDialogBase
{
  Q_OBJECT

public:
  KMTransportDialog( const TQString & caption, KMTransportInfo *transportInfo,
		     TQWidget *parent=0, const char *name=0, bool modal=TRUE );
  virtual ~KMTransportDialog();

private slots:
  virtual void slotOk();
  void slotSendmailChooser();
  void slotRequiresAuthClicked();
  void slotSmtpEncryptionChanged(int);
  void slotCheckSmtpCapabilities();
  void slotSmtpCapabilities( const TQStringList &, const TQStringList &,
                             const TQString &, const TQString &,
                             const TQString & );
  void slotSendmailEditPath(const TQString &);
private:
  struct SendmailWidgets
  {
    TQLabel       *titleLabel;
    TQLineEdit    *nameEdit;
    TQLineEdit    *locationEdit;
    TQPushButton  *chooseButton;
  };
  struct SmtpWidgets
  {
    TQLabel       *titleLabel;
    TQLineEdit    *nameEdit;
    TQLineEdit    *hostEdit;
    TQLineEdit    *portEdit;
    TQCheckBox    *authCheck;
    TQLabel       *loginLabel;
    TQLineEdit    *loginEdit;
    TQLabel       *passwordLabel;
    TQLineEdit    *passwordEdit;
    TQLineEdit    *precommand;
    TQButtonGroup *encryptionGroup;
    TQRadioButton *encryptionNone;
    TQRadioButton *encryptionSSL;
    TQRadioButton *encryptionTLS;
    TQButtonGroup *authGroup;
    TQRadioButton *authPlain;
    TQRadioButton *authLogin;
    TQRadioButton *authCramMd5;
    TQRadioButton *authDigestMd5;
    TQRadioButton *authNTLM;
    TQRadioButton *authGSSAPI;
    TQPushButton  *checkCapabilities;
    TQCheckBox    *storePasswordCheck;
    TQCheckBox    *specifyHostnameCheck;
    TQLineEdit    *localHostnameEdit;
    TQLabel       *localHostnameLabel;
  };

  void makeSendmailPage();
  void makeSmtpPage();
  void setupSettings();
  void saveSettings();
  void checkHighest( TQButtonGroup * );
  void enableAuthMethods( unsigned int which );
  bool sanityCheckSmtpInput();
  static unsigned int authMethodsFromString( const TQString & s );
  static unsigned int authMethodsFromStringList( const TQStringList & sl );

  KMServerTest    *mServerTest;
  SmtpWidgets     mSmtp;
  SendmailWidgets mSendmail;
  KMTransportInfo *mTransportInfo;
  enum EncryptionMethods {
    NoEncryption = 0,
    SSL = 1,
    TLS = 2
  };
  enum AuthMethods {
    NoAuth = 0,
    LOGIN = 1,
    PLAIN = 2,
    CRAM_MD5 = 4,
    DIGEST_MD5 = 8,
    NTLM = 16,
    GSSAPI = 32,
    AllAuth = 0xffffffff
  };
  unsigned int mAuthNone, mAuthSSL, mAuthTLS;
};


#endif
