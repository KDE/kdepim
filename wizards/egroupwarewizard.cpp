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

#include "egroupwarewizard.h"

#include "actionpage.h"
#include "egroupwarehandler.h"

#include <klocale.h>

#include <qlayout.h>

class EGroupwarePropagator : public KConfigPropagator
{
  public:
    EGroupwarePropagator( ServerType *h )
      : KConfigPropagator(), mHandler( h )
    {
    }

  protected:
    void addCustomChanges( Change::List &changes )
    {
      Change::List c = mHandler->changes();
      Change *change;
      for( change = c.first(); change; change = c.next() ) {
        changes.append( change );
      }
    }

  private:
    ServerType *mHandler;
};

EGroupwareWizard::EGroupwareWizard() : KConfigWizard()
{
  QFrame *page = createWizardPage( i18n("eGroupware Server") );

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setSpacing( spacingHint() );

  ActionPage *actionPage = new ActionPage( page );
  actionPage->setServerType( "egroupware" );

  ServerType *serverType = actionPage->serverType();
  EGroupwarePropagator *propagator = new EGroupwarePropagator( serverType );
  setPropagator( propagator );

  topLayout->addWidget( actionPage, 0, 0 );

  setupChangesPage();

  resize( 400, 300 );
}

EGroupwareWizard::~EGroupwareWizard()
{
}

void EGroupwareWizard::usrReadConfig()
{
}

void EGroupwareWizard::usrWriteConfig()
{
}
