/*******************************************************************************
**
** Filename   : accountwizard.h
** Created on : 07 February, 2005
** Copyright  : (c) 2005 Tobias Koenig
** Email      : tokoe@kde.org
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   In addition, as a special exception, the copyright holders give
**   permission to link the code of this program with any edition of
**   the Qt library by Trolltech AS, Norway (or with modified versions
**   of Qt that use the same license as Qt), and distribute linked
**   combinations including the two.  You must obey the GNU General
**   Public License in all respects for all of the code used other than
**   Qt.  If you modify this file, you may extend this exception to
**   your version of the file, but you are not obligated to do so.  If
**   you do not wish to do so, delete this exception statement from
**   your version.
*******************************************************************************/

#ifndef KMWIZARD_H
#define KMWIZARD_H

#include <kwizard.h>

class KLineEdit;
class QCheckBox;
class QLabel;
class QPushButton;

class KMAccount;
class KMKernel;
class KMServerTest;
class AccountTypeBox;
class KMTransportInfo;

class AccountWizard : public KWizard
{
  Q_OBJECT

  public:
    /**
      Starts the wizard. The wizard is only shown when it has not be
      run successfully before.

      @param kernel The mail kernel the wizard should work on.
      @param parent The parent widget of the dialog.
     */
    static void start( KMKernel *kernel, TQWidget *parent = 0 );

    /**
      Reimplemented
     */
    void showPage( TQWidget *page );

  protected:
    AccountWizard( KMKernel *kernel, TQWidget *parent );
    ~AccountWizard() {};

    void setupWelcomePage();
    void setupAccountTypePage();
    void setupAccountInformationPage();
    void setupLoginInformationPage();
    void setupServerInformationPage();

  protected slots:
    void chooseLocation();
    virtual void accept();
    void createTransport();
    void transportCreated();
    void createAccount();
    void accountCreated();
    void finished();

  private slots:
    void popCapabilities( const TQStringList&, const TQStringList& );
    void imapCapabilities( const TQStringList&, const TQStringList& );
    void smtpCapabilities( const TQStringList&, const TQStringList&,
                           const TQString&, const TQString&, const TQString& );

  private:
    TQString accountName() const;
    TQLabel *createInfoLabel( const TQString &msg );

    void checkPopCapabilities( const TQString&, int );
    void checkImapCapabilities( const TQString&, int );
    void checkSmtpCapabilities( const TQString&, int );
    uint popCapabilitiesFromStringList( const TQStringList& );
    uint imapCapabilitiesFromStringList( const TQStringList& );
    uint authMethodsFromString( const TQString& );
    uint authMethodsFromStringList( const TQStringList& );

    TQWidget *mWelcomePage;

    TQWidget *mAccountTypePage;
    AccountTypeBox *mTypeBox;

    TQWidget *mAccountInformationPage;
    KLineEdit *mRealName;
    KLineEdit *mEMailAddress;
    KLineEdit *mOrganization;

    TQWidget *mLoginInformationPage;
    KLineEdit *mLoginName;
    KLineEdit *mPassword;

    TQWidget *mServerInformationPage;
    TQLabel *mIncomingLabel;
    KLineEdit *mIncomingServer;
    TQCheckBox *mIncomingUseSSL;
    KLineEdit *mIncomingLocation;

    TQPushButton *mChooseLocation;
    KLineEdit *mOutgoingServer;
    TQCheckBox *mOutgoingUseSSL;
    TQCheckBox *mLocalDelivery;

    TQWidget *mIncomingServerWdg;
    TQWidget *mIncomingLocationWdg;

    TQLabel *mAuthInfoLabel;

    KMKernel *mKernel;
    KMAccount *mAccount;
    KMTransportInfo *mTransportInfo;
    TQPtrList<KMTransportInfo> mTransportInfoList;
    KMServerTest *mServerTest;
};

#endif
