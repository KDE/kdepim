/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kolabconfig.h"

#include <kconfigwizard.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <klineedit.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>

class KolabPropagator : public KConfigPropagator
{
  public:
    KolabPropagator()
      : KConfigPropagator( KolabConfig::self(), "kolab.kcfg" )
    {
    }

  protected:
    void addCustomChanges( Change::List &changes )
    {
      Change c;
      c.file = "korganizerrc";
      c.group = "Freebusy";
      c.name = "FreeBusyPublishUrl";
      c.label = "";

      c.value = "webdavs://" + KolabConfig::self()->server() + "/freebusy/";

      QString user = KolabConfig::self()->user();
      int pos = user.find( "@" );
      if ( pos > 0 ) user = user.left( pos );

      c.value += user + ".vfb";

      changes.append( c );
    }
};

class KolabWizard : public KConfigWizard
{
  public:
    KolabWizard() : KConfigWizard( new KolabPropagator )
    {
      QFrame *page = createWizardPage( "Kolab Server" );

      QGridLayout *topLayout = new QGridLayout( page );
      topLayout->setSpacing( spacingHint() );

      QLabel *label = new QLabel( i18n("Server Name"), page );
      topLayout->addWidget( label, 0, 0 );
      mServerEdit = new KLineEdit( page );
      topLayout->addWidget( mServerEdit, 0, 1 );
      
      label = new QLabel( i18n("User Name"), page );
      topLayout->addWidget( label, 1, 0 );
      mUserEdit = new KLineEdit( page );
      topLayout->addWidget( mUserEdit, 1, 1 );
      
      label = new QLabel( i18n("Password"), page );
      topLayout->addWidget( label, 2, 0 );
      mPasswordEdit = new KLineEdit( page );
      mPasswordEdit->setEchoMode( KLineEdit::Password );
      topLayout->addWidget( mPasswordEdit, 2, 1 );

      mSavePasswordCheck = new QCheckBox( i18n("Save Password"), page );
      topLayout->addMultiCellWidget( mSavePasswordCheck, 3, 3, 0, 1 );

      topLayout->setRowStretch( 4, 1 );

      setupRulesPage();
      setupChangesPage();

      resize( 400, 300 );
    }
    
    ~KolabWizard()
    {
    }

    void usrReadConfig()
    {
      mServerEdit->setText( KolabConfig::self()->server() );
      mUserEdit->setText( KolabConfig::self()->user() );
      mPasswordEdit->setText( KolabConfig::self()->password() );
      mSavePasswordCheck->setChecked( KolabConfig::self()->savePassword() );
    }

    void usrWriteConfig()
    {
      KolabConfig::self()->setServer( mServerEdit->text() );
      KolabConfig::self()->setUser( mUserEdit->text() );
      KolabConfig::self()->setPassword( mPasswordEdit->text() );
      KolabConfig::self()->setSavePassword( mSavePasswordCheck->isChecked() );    
    }

  private:
    KLineEdit *mServerEdit;
    KLineEdit *mUserEdit;
    KLineEdit *mPasswordEdit;
    QCheckBox *mSavePasswordCheck;
};

static const KCmdLineOptions options[] =
{
  {"verbose", "Verbose output", 0},
  KCmdLineLastOption
};

int main(int argc,char **argv)
{
  KAboutData aboutData( "kolabwizard", "Kolab Configuration Wizard", "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  bool verbose = false;
  if ( args->isSet( "verbose" ) ) verbose = true;

  KolabWizard wizard;
  
  wizard.exec();
}
