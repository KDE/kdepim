/*
    This file is part of libkdepim.

    Copyright (c) 2004 Lutz Rogowski <rogowski@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kincidencechooser.h"
#include "libkcal/incidence.h"
#include "libkcal/incidenceformatter.h"

#include <klocale.h>

#include <qbuttongroup.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

int KIncidenceChooser::chooseMode = KIncidenceChooser::ask;

KIncidenceChooser::KIncidenceChooser( QWidget *parent, char *name )
  : KDialog( parent, name, true )
{
  KDialog *topFrame = this;
  QGridLayout *topLayout = new QGridLayout( topFrame, 5, 3 );

  int iii = 0;
  setCaption( i18n( "Conflict Detected" ) );
  QLabel *lab;
  lab = new QLabel(
    i18n( "<qt>A conflict was detected. This probably means someone edited "
          "the same incidence on the server while you changed it locally."
          "<p>"
          "<b>NOTE</b>: You have to check mail again to apply your changes "
          "to the server.</qt>" ), topFrame );
  topLayout->addMultiCellWidget( lab, iii, iii, 0, 2 );
  ++iii;

  QHBox *b_box = new QHBox( topFrame );
  topLayout->addMultiCellWidget( b_box, iii, iii, 0, 2 );
  ++iii;

  QPushButton *locBut = new QPushButton( i18n( "Take Local" ), b_box );
  connect( locBut, SIGNAL(clicked()), this, SLOT (takeIncidence1()) );
  QToolTip::add(
    locBut,
    i18n( "Take your local copy of the incidence" ) );
  QWhatsThis::add(
    locBut,
    i18n( "A conflict was detected between your local copy of the incidence "
          "and the remote incidence on the server. Press the \"Take Local\" "
          "button to make sure your local copy is used." ) );

  QPushButton *remBut = new QPushButton( i18n( "Take New" ), b_box );
  connect( remBut, SIGNAL(clicked()), this, SLOT (takeIncidence2()) );
  QToolTip::add(
    remBut,
    i18n( "Take the server copy of the incidence" ) );
  QWhatsThis::add(
    remBut,
    i18n( "A conflict was detected between your local copy of the incidence "
          "and the remote incidence on the server. Press the \"Take New\" "
          "button to use the server copy, thereby overwriting your local copy" ) );

  QPushButton *bothBut = new QPushButton( i18n( "Take Both" ), b_box );
  bothBut->setFocus(); //kolab/issue4147:  "Take Both" should be default
  connect( bothBut, SIGNAL(clicked()), this, SLOT (takeBoth()) );
  QToolTip::add(
    bothBut,
    i18n( "Take both copies of the incidence" ) );
  QWhatsThis::add(
    bothBut,
    i18n( "A conflict was detected between your local copy of the incidence "
          "and the remote incidence on the server. Press the \"Take Both\" "
          "button to keep both the local and the server copies." ) );

  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( marginHint() );

  mInc1lab = new QLabel( i18n( "Local incidence" ), topFrame );
  topLayout->addWidget( mInc1lab, iii, 0 );

  mInc1Sumlab = new QLabel( i18n( "Local incidence summary" ), topFrame );
  topLayout->addMultiCellWidget( mInc1Sumlab, iii, iii, 1, 2 );
  ++iii;

  topLayout->addWidget( new QLabel( i18n( "Last modified:" ), topFrame ), iii, 0 );

  mMod1lab = new QLabel( "Set Last modified", topFrame );
  topLayout->addWidget( mMod1lab, iii, 1 );

  mShowDetails1 = new QPushButton( i18n( "Show Details" ), topFrame );
  QToolTip::add(
    mShowDetails1,
    i18n( "Hide/Show incidence details" ) );
  QWhatsThis::add(
    mShowDetails1,
    i18n( "Press this button to toggle the incidence details display." ) );
  connect( mShowDetails1, SIGNAL(clicked()), this, SLOT (showIncidence1()) );
  topLayout->addWidget( mShowDetails1, iii, 2 );
  ++iii;

  mInc2lab = new QLabel( "Local incidence", topFrame );
  topLayout->addWidget( mInc2lab, iii, 0 );

  mInc2Sumlab = new QLabel( "Local incidence summary", topFrame );
  topLayout->addMultiCellWidget( mInc2Sumlab, iii, iii, 1, 2 );
  ++iii;

  topLayout->addWidget( new QLabel( i18n( "Last modified:" ), topFrame ), iii, 0 );

  mMod2lab = new QLabel( "Set Last modified", topFrame );
  topLayout->addWidget( mMod2lab, iii, 1 );

  mShowDetails2 = new QPushButton( i18n( "Show Details" ), topFrame );
  QToolTip::add(
    mShowDetails2,
    i18n( "Hide/Show incidence details" ) );
  QWhatsThis::add(
    mShowDetails2,
    i18n( "Press this button to toggle the incidence details display." ) );
  connect( mShowDetails2, SIGNAL(clicked()), this, SLOT(showIncidence2()) );
  topLayout->addWidget( mShowDetails2, iii, 2 );
  ++iii;

#if 0
  // commented out for now, because the diff code has too many bugs
  mDiffBut = new QPushButton( i18n( "Show Differences" ), topFrame );
  QToolTip::add(
    mDiffBut,
    i18n( "Show the differences between the two incidences" ) );
  QWhatsThis::add(
    mDiffBut,
    i18n( "Press the \"Show Differences\" button to see the specific "
          "differences between the incidences which are in conflict." ) );
  connect ( mDiffBut, SIGNAL(clicked()), this, SLOT (showDiff()) );
  topLayout->addMultiCellWidget( diffBut, iii, iii, 0, 2 );
  ++iii;
#else
  mDiffBut = 0;
#endif
  mBg = new QButtonGroup( 1, Qt::Horizontal, i18n( "Sync Preferences" ), topFrame );
  topLayout->addMultiCellWidget( mBg, iii, iii, 0, 2 );
  ++iii;

  QRadioButton *locRad = new QRadioButton(
    i18n( "Take local copy on conflict" ), mBg );
  mBg->insert( locRad, KIncidenceChooser::local );
  QToolTip::add(
    locRad,
    i18n( "Take local copy of the incidence on conflicts" ) );
  QWhatsThis::add(
    locRad,
    i18n( "When a conflict is detected between a local copy of an incidence "
          "and a remote incidence on the server, this option enforces using "
          "the local copy." ) );

  QRadioButton *remRad = new QRadioButton(
    i18n( "Take remote copy on conflict" ), mBg );
  mBg->insert( remRad, KIncidenceChooser::remote );
  QToolTip::add(
    remRad,
    i18n( "Take remote copy of the incidence on conflicts" ) );
  QWhatsThis::add(
    remRad,
    i18n( "When a conflict is detected between a local copy of an incidence "
          "and a remote incidence on the server, this option enforces using "
          "the remote copy." ) );

  QRadioButton *newRad = new QRadioButton(
    i18n( "Take newest incidence on conflict" ), mBg );
  mBg->insert( newRad, KIncidenceChooser::newest );
  QToolTip::add(
    newRad,
    i18n( "Take newest version of the incidence on conflicts" ) );
  QWhatsThis::add(
    newRad,
    i18n( "When a conflict is detected between a local copy of an incidence "
          "and a remote incidence on the server, this option enforces using "
          "the newest version available." ) );

  QRadioButton *askRad = new QRadioButton(
    i18n( "Ask for every conflict" ), mBg );
  mBg->insert( askRad, KIncidenceChooser::ask );
  QToolTip::add(
    askRad,
    i18n( "Ask for every incidence conflict" ) );
  QWhatsThis::add(
    askRad,
    i18n( "When a conflict is detected between a local copy of an incidence "
          "and a remote incidence on the server, this option says to ask "
          "the user which version they want to keep." ) );

  QRadioButton *bothRad = new QRadioButton(
    i18n( "Take both on conflict" ), mBg );
  mBg->insert( bothRad, KIncidenceChooser::both );
  QToolTip::add(
    bothRad,
    i18n( "Take both incidences on conflict" ) );
  QWhatsThis::add(
    bothRad,
    i18n( "When a conflict is detected between a local copy of an incidence "
          "and a remote incidence on the server, this option says to keep "
          "both versions of the incidence." ) );

  mBg->setButton ( chooseMode );

  QPushButton *applyBut = new QPushButton(
    i18n( "Apply preference to all conflicts of this sync" ), topFrame );
  connect( applyBut, SIGNAL(clicked()), this, SLOT(setSyncMode()) );
  QToolTip::add(
    applyBut,
    i18n( "Apply the preference to all conflicts that may occur during the sync" ) );
  QWhatsThis::add(
    applyBut,
    i18n( "Press this button to apply the selected preference to all "
          "future conflicts that might occur during this sync." ) );
  topLayout->addMultiCellWidget( applyBut, iii, iii, 0, 2 );

  mTbL = 0;
  mTbN =  0;
  mDisplayDiff = 0;
  mSelIncidence = 0;
}

KIncidenceChooser::~KIncidenceChooser()
{
  if ( mTbL ) {
    delete mTbL;
  }
  if ( mTbN ) {
    delete mTbN;
  }
  if ( mDisplayDiff ) {
    delete mDisplayDiff;
    delete diff;
  }
}

void KIncidenceChooser::setIncidence( Incidence *local, Incidence *remote )
{
  mInc1 = local;
  mInc2 = remote;
  setLabels();

}

Incidence *KIncidenceChooser::getIncidence( )
{
  Incidence *retval = mSelIncidence;
  if ( chooseMode == KIncidenceChooser::local ) {
    retval = mInc1;
  } else if ( chooseMode == KIncidenceChooser::remote ) {
    retval = mInc2;
  } else if ( chooseMode == KIncidenceChooser::both ) {
    retval = 0;
  } else if ( chooseMode == KIncidenceChooser::newest ) {
    if ( mInc1->lastModified() == mInc2->lastModified() ) {
      retval = 0;
    }
    if ( mInc1->lastModified() >  mInc2->lastModified() ) {
      retval =  mInc1;
    } else {
      retval = mInc2;
    }
  }
  return retval;
}

void KIncidenceChooser::setSyncMode()
{
  chooseMode = mBg->selectedId ();
  if ( chooseMode != KIncidenceChooser::ask ) {
    QDialog::accept();
  }
}

void KIncidenceChooser::useGlobalMode()
{
  if ( chooseMode != KIncidenceChooser::ask ) {
    QDialog::reject();
  }
}

void KIncidenceChooser::setLabels()
{
  Incidence *inc = mInc1;
  QLabel *des = mInc1lab;
  QLabel *sum = mInc1Sumlab;

  if ( inc->type() == "Event" ) {
    des->setText( i18n( "Local Event" ) );
    sum->setText( inc->summary().left( 30 ) );
    if ( mDiffBut ) {
      mDiffBut->setEnabled( true );
    }
  } else if ( inc->type() == "Todo" ) {
    des->setText( i18n( "Local Todo" ) );
    sum->setText( inc->summary().left( 30 ) );
    if ( mDiffBut ) {
      mDiffBut->setEnabled( true );
    }
  } else if ( inc->type() == "Journal" ) {
    des->setText( i18n( "Local Journal" ) );
    sum->setText( inc->description().left( 30 ) );
    if ( mDiffBut ) {
      mDiffBut->setEnabled( false );
    }
  }
  mMod1lab->setText( KGlobal::locale()->formatDateTime( inc->lastModified() ) );
  inc = mInc2;
  des = mInc2lab;
  sum = mInc2Sumlab;
  if ( inc->type() == "Event" ) {
    des->setText( i18n( "New Event" ) );
    sum->setText( inc->summary().left( 30 ) );
  } else if ( inc->type() == "Todo" ) {
    des->setText( i18n( "New Todo" ) );
    sum->setText( inc->summary().left( 30 ) );
  } else if ( inc->type() == "Journal" ) {
    des->setText( i18n( "New Journal" ) );
    sum->setText( inc->description().left( 30 ) );
  }
  mMod2lab->setText( KGlobal::locale()->formatDateTime( inc->lastModified() ) );
}

void KIncidenceChooser::showIncidence1()
{
  if ( mTbL ) {
    if ( mTbL->isVisible() ) {
      mShowDetails1->setText( i18n( "Show Details" ) );
      mTbL->hide();
    } else {
      mShowDetails1->setText( i18n( "Hide Details" ) );
      mTbL->show();
      mTbL->raise();
    }
    return;
  }
  mTbL = new KDialogBase( this, "", false/*not modal*/, mInc1lab->text(), KDialogBase::Ok );
  mTbL->setEscapeButton( KDialogBase::Ok );
  connect( mTbL, SIGNAL(okClicked()), this, SLOT(detailsDialogClosed()) );
  QTextBrowser *textBrowser = new QTextBrowser( mTbL );
  mTbL->setMainWidget( textBrowser );
  textBrowser->setText( IncidenceFormatter::extensiveDisplayStr( 0, mInc1 ) );
  QToolTip::add( textBrowser, i18n( "Incidence details" ) );
  QWhatsThis::add( textBrowser, i18n( "This area shows the incidence details" ) );
  mTbL->setMinimumSize( 400, 400 );
  mShowDetails1->setText( i18n( "Hide Details" ) );
  mTbL->show();
  mTbL->raise();
}

void KIncidenceChooser::detailsDialogClosed()
{
  KDialogBase* dialog = static_cast<KDialogBase *>( const_cast<QObject *>( sender() ) );
  if ( dialog == mTbL ) {
    mShowDetails1->setText( i18n( "Show details..." ) );
  } else {
    mShowDetails2->setText( i18n( "Show details..." ) );
  }
}

void KIncidenceChooser::showDiff()
{
  if ( mDisplayDiff ) {
    mDisplayDiff->show();
    mDisplayDiff->raise();
    return;
  }
  mDisplayDiff = new KPIM::HTMLDiffAlgoDisplay ( this );
  if ( mInc1->summary().left( 20 ) != mInc2->summary().left( 20 ) ) {
    mDisplayDiff->setCaption(
      i18n( "Differences of %1 and %2" ).
      arg( mInc1->summary().left( 20 ) ).arg( mInc2->summary().left( 20 ) ) );
  } else {
    mDisplayDiff->setCaption(
      i18n( "Differences of %1" ).arg( mInc1->summary().left( 20 ) ) );
  }
  diff = new KPIM::CalendarDiffAlgo( mInc1, mInc2 );
  diff->setLeftSourceTitle( i18n( "Local incidence" ) );
  diff->setRightSourceTitle( i18n( "Remote incidence" ) );
  diff->addDisplay( mDisplayDiff );
  diff->run();
  mDisplayDiff->show();
  mDisplayDiff->raise();
}

void KIncidenceChooser::showIncidence2()
{
  if ( mTbN ) {
    if ( mTbN->isVisible() ) {
      mShowDetails2->setText( i18n( "Show Details" ) );
      mTbN->hide();
    } else {
      mShowDetails2->setText( i18n( "Hide Details" ) );
      mTbN->show();
      mTbN->raise();
    }
    return;
  }
  mTbN = new KDialogBase( this, "", false/*not modal*/, mInc2lab->text(), KDialogBase::Ok );
  mTbN->setEscapeButton( KDialogBase::Ok );
  connect( mTbN, SIGNAL(okClicked()), this, SLOT(detailsDialogClosed()) );
  QTextBrowser *textBrowser = new QTextBrowser( mTbN );
  mTbN->setMainWidget( textBrowser );
  textBrowser->setText( IncidenceFormatter::extensiveDisplayStr( 0, mInc2 ) );
  QToolTip::add( textBrowser, i18n( "Incidence details" ) );
  QWhatsThis::add( textBrowser, i18n( "This area shows the incidence details" ) );
  mTbN->setMinimumSize( 400, 400 );
  mShowDetails2->setText( i18n( "Hide Details" ) );
  mTbN->show();
  mTbN->raise();
}

void KIncidenceChooser::takeIncidence1()
{
  mSelIncidence = mInc1;
  QDialog::accept();
}

void KIncidenceChooser::takeIncidence2()
{
  mSelIncidence = mInc2;
  QDialog::accept();
}

void KIncidenceChooser::takeBoth()
{
  mSelIncidence = 0;
  QDialog::accept();
}

#include "kincidencechooser.moc"
