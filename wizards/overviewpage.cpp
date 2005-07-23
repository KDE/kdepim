/*
    This file is part of kdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>

#include <kaccelmanager.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

#include "egroupwarewizard.h"
#include "kolabwizard.h"
#include "sloxwizard.h"
#include "groupwisewizard.h"
#include "exchangewizard.h"

#include "overviewpage.h"

OverViewPage::OverViewPage( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QGridLayout *layout = new QGridLayout( this, 7, 4, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  const QString msg = i18n( "KDE Groupware Wizard" );
  QLabel *label = new QLabel( "<qt><b><u><h2>" + msg + "</h2></u></b></qt>" , this );
  layout->addMultiCellWidget( label, 0, 0, 0, 2 );

  label = new QLabel( this );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "network", KIcon::Desktop ) );
  layout->addWidget( label, 0, 3 );

  label = new QLabel( "", this );
  layout->addWidget( label, 1, 0 );
  layout->setRowSpacing( 1, 20 );

  label = new QLabel( i18n( "Select the type of server you want connect your KDE to:" ), this );
  layout->addMultiCellWidget( label, 2, 2, 0, 3 );

  QPushButton *button = new QPushButton( i18n("eGroupware"), this );
  layout->addMultiCellWidget( button, 3, 3, 0, 3 );
  connect( button, SIGNAL( clicked() ), SLOT( showWizardEGroupware() ) );

  // FIXME: Maybe hyperlinks would be better than buttons.

  button = new QPushButton( i18n("Kolab"), this );
  layout->addMultiCellWidget( button, 4, 4, 0, 3 );
  connect( button, SIGNAL( clicked() ), SLOT( showWizardKolab() ) );

  button = new QPushButton( i18n("SUSE Linux Openexchange (SLOX)"), this );
  layout->addMultiCellWidget( button, 5, 5, 0, 3 );
  connect( button, SIGNAL( clicked() ), SLOT( showWizardSlox() ) );

  button = new QPushButton( i18n("Novell GroupWise"), this );
  layout->addMultiCellWidget( button, 6, 6, 0, 3 );
  connect( button, SIGNAL( clicked() ), SLOT( showWizardGroupwise() ) );

  button = new QPushButton( i18n("Microsoft Exchange"), this );
  button->hide(); // not quite ready yet
  layout->addMultiCellWidget( button, 7, 7, 0, 3 );
  connect( button, SIGNAL( clicked() ), SLOT( showWizardExchange() ) );


  QFrame *frame = new QFrame( this );
  frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  layout->addMultiCellWidget( frame, 8, 8, 0, 3 );

  QPushButton *cancelButton = new KPushButton( KStdGuiItem::close(), this );
  layout->addWidget( cancelButton, 9, 3 );

  connect( cancelButton, SIGNAL( clicked() ), this, SIGNAL( cancel() ) );

  layout->setRowStretch( 8, 1 );

  KAcceleratorManager::manage( this );
}

OverViewPage::~OverViewPage()
{
}

void OverViewPage::showWizardEGroupware()
{
  EGroupwareWizard wizard;
  wizard.exec();
}

void OverViewPage::showWizardKolab()
{
  KolabWizard wizard;
  wizard.exec();
}

void OverViewPage::showWizardSlox()
{
  SloxWizard wizard;
  wizard.exec();
}

void OverViewPage::showWizardGroupwise()
{
  GroupwiseWizard wizard;
  wizard.exec();
}

void OverViewPage::showWizardExchange()
{
  ExchangeWizard wizard;
  wizard.exec();
}

#include "overviewpage.moc"
