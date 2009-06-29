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

#include "overviewpage.h"
#include "egroupwarewizard.h"
#include "kolabwizard.h"
#include "sloxwizard.h"
#include "groupwisewizard.h"

#include <kacceleratormanager.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <KStandardGuiItem>

#include <QButtonGroup>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>

OverViewPage::OverViewPage( QWidget *parent )
  : QWidget( parent )
{

  QGridLayout *layout = new QGridLayout;
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );
  setLayout(layout);

  const QString msg = i18n( "KDE Groupware Wizard" );
  QLabel *label = new QLabel( "<qt><b><u><h2>" + msg + "</h2></u></b></qt>" , this );
  layout->addWidget( label, 0, 0, 1, 3 );

  label = new QLabel( this );
  label->setPixmap( KIconLoader::global()->loadIcon( "network-wired", KIconLoader::Desktop ) );
  layout->addWidget( label, 0, 3 );

  label = new QLabel( "", this );
  layout->addWidget( label, 1, 0 );
  layout->setRowMinimumHeight( 1, 20 );

  label = new QLabel( i18n( "Select the type of server you would like Kontact to connect to:" ), this );
  layout->addWidget( label, 2, 0, 1, 4 );

  QPushButton *button = new QPushButton( i18n("eGroupware"), this );
  layout->addWidget( button, 3, 0, 1, 4 );
  connect( button, SIGNAL( clicked() ), SLOT( showWizardEGroupware() ) );

  // FIXME: Maybe hyperlinks would be better than buttons.

  button = new QPushButton( i18n("Kolab"), this );
  layout->addWidget( button, 4, 0, 1, 4 );
  connect( button, SIGNAL( clicked() ), SLOT( showWizardKolab() ) );

  button = new QPushButton( i18n("SUSE Linux Openexchange (SLOX)"), this );
  layout->addWidget( button, 5, 0, 1, 4 );
  connect( button, SIGNAL( clicked() ), SLOT( showWizardSlox() ) );

  button = new QPushButton( i18n("Novell GroupWise"), this );
  layout->addWidget( button, 6, 0, 1, 4 );
  connect( button, SIGNAL( clicked() ), SLOT( showWizardGroupwise() ) );


  QFrame *frame = new QFrame( this );
  frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  layout->addWidget( frame, 8, 0, 1, 4 );

  QPushButton *cancelButton = new KPushButton( KStandardGuiItem::close(), this );
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
// FIXME: disabled as egroupware is disabled due to a kaddressbook dependency
/*  EGroupwareWizard wizard;
  wizard.exec();*/
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

#include "overviewpage.moc"
