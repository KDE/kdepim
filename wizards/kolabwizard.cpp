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

#include "kolabwizard.h"
#include "kolabconfig.h"

#include "kresources/imap/kcal/resourceimap.h"

#include <libkcal/resourcecalendar.h>

#include <klineedit.h>
#include <klocale.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>


class CreateImapResource : public KConfigPropagator::Change
{
  public:
    CreateImapResource()
      : KConfigPropagator::Change( i18n("Create IMAP Resource") )
    {
    }

    void apply()
    {
      kdDebug() << "Create IMAP Resource" << endl;

      KCal::CalendarResourceManager m( "calendar" );
      m.readConfig();
      KCal::ResourceIMAP *r = new KCal::ResourceIMAP( "FIXME" );
      r->setResourceName( i18n("Kolab") );
      m.add( r );
      m.writeConfig();
    }
};

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
      QString freeBusyBaseUrl = "webdavs://" + KolabConfig::self()->server() +
                                "/freebusy/";

      ChangeConfig *c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";

      c->name = "FreeBusyPublishUrl";
      c->label = "";

      QString user = KolabConfig::self()->user();
      int pos = user.find( "@" );
      if ( pos > 0 ) user = user.left( pos );

      c->value = freeBusyBaseUrl + user + ".vfb";

      changes.append( c );

      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";

      c->name = "FreeBusyRetrieveUrl";
      c->value = freeBusyBaseUrl;

      changes.append( c );

      KCal::CalendarResourceManager m( "calendar" );
      m.readConfig();
      KCal::CalendarResourceManager::Iterator it;
      for ( it = m.begin(); it != m.end(); ++it ) {
        if ( (*it)->type() == "imap" ) break;
      }
      if ( it == m.end() ) {
        changes.append( new CreateImapResource );
      }
    }
};

KolabWizard::KolabWizard() : KConfigWizard( new KolabPropagator )
{
  QFrame *page = createWizardPage( i18n("Kolab Server") );

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18n("Server name:"), page );
  topLayout->addWidget( label, 0, 0 );
  mServerEdit = new KLineEdit( page );
  topLayout->addWidget( mServerEdit, 0, 1 );

  label = new QLabel( i18n("User name:"), page );
  topLayout->addWidget( label, 1, 0 );
  mUserEdit = new KLineEdit( page );
  topLayout->addWidget( mUserEdit, 1, 1 );

  label = new QLabel( i18n("Password:"), page );
  topLayout->addWidget( label, 2, 0 );
  mPasswordEdit = new KLineEdit( page );
  mPasswordEdit->setEchoMode( KLineEdit::Password );
  topLayout->addWidget( mPasswordEdit, 2, 1 );

  mSavePasswordCheck = new QCheckBox( i18n("Save password"), page );
  topLayout->addMultiCellWidget( mSavePasswordCheck, 3, 3, 0, 1 );

  topLayout->setRowStretch( 4, 1 );

  setupRulesPage();
  setupChangesPage();

  resize( 400, 300 );
}

KolabWizard::~KolabWizard()
{
}

void KolabWizard::usrReadConfig()
{
  mServerEdit->setText( KolabConfig::self()->server() );
  mUserEdit->setText( KolabConfig::self()->user() );
  mPasswordEdit->setText( KolabConfig::self()->password() );
  mSavePasswordCheck->setChecked( KolabConfig::self()->savePassword() );
}

void KolabWizard::usrWriteConfig()
{
  KolabConfig::self()->setServer( mServerEdit->text() );
  KolabConfig::self()->setUser( mUserEdit->text() );
  KolabConfig::self()->setPassword( mPasswordEdit->text() );
  KolabConfig::self()->setSavePassword( mSavePasswordCheck->isChecked() );
}
