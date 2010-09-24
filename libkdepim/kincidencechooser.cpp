/*
  This file is part of libkdepim.

  Copyright (c) 2004 Lutz Rogowski <rogowski@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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
using namespace KCal;

#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qapplication.h>
#include <qbuttongroup.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtextbrowser.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

using namespace KPIM;

KIncidenceChooser::KIncidenceChooser( const QString &folder, ConflictAskPolicy askPolicy,
                                      bool folderOnly, QWidget *parent, char *name )
  : KDialog( parent, name, true ),
    mFolder( folder ), mAskPolicy( askPolicy ), mFolderOnly( folderOnly )
{
  mTakeMode = Newer;
  QColor bc = KGlobalSettings::alternateBackgroundColor();

  KDialog *topFrame = this;
  QGridLayout *topLayout = new QGridLayout( topFrame, 5, 1 );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( marginHint() );

  // row 1: dialog description
  int iii = 0;
  setCaption( i18n( "Synchronization Conflict Detected" ) );
  QLabel *lab = new QLabel(
    i18n( "<qt>A synchronization conflict occurred: Probably someone "
          "modified an entry remotely (i.e. on the server), which you "
          "have also modified locally (i.e. on your client).</qt>" ), topFrame );

  lab->setFrameShape( QFrame::StyledPanel );
  lab->setPaletteBackgroundColor( bc );
  topLayout->addMultiCellWidget( lab, iii, iii, 0, 1 );
  ++iii;

  // row 2: info box
  QGridLayout *infoGrid = new QGridLayout( topLayout, 4, 3 );
  ++iii;

  QLabel *conflictLab = new QLabel( i18n( "Conflict in folder:" ), topFrame );
  infoGrid->addWidget( conflictLab, 0, 0 );

  QString folderStr = mFolder;
  if ( mFolder.isEmpty() ) {
    folderStr = i18n( "folder name was not specified", "not specified" );
  }
  QLabel *folderLab = new QLabel( QString( "%1" ).arg( folderStr ), topFrame );
  infoGrid->addWidget( folderLab, 0, 1 );

  QLabel *locEntryLab = new QLabel( i18n( "Local entry:" ), topFrame );
  infoGrid->addWidget( locEntryLab, 1, 0 );

  mLocEntryVal = new QLabel( "summary", topFrame );
  mLocEntryVal->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  infoGrid->addWidget( mLocEntryVal, 1, 1 );

  QLabel *locModLab = new QLabel( i18n( "Last modified:" ), topFrame );
  infoGrid->addWidget( locModLab, 2, 0 );

  mLocModVal = new QLabel( "modified date/time", topFrame );
  mLocModVal->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  infoGrid->addWidget( mLocModVal, 2, 1 );

  mLocShowDetails = new QPushButton( i18n( "&Show Details..." ), topFrame );
  mLocShowDetails->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  infoGrid->addWidget( mLocShowDetails, 2, 2 );
  QToolTip::add(
    mLocShowDetails,
    i18n( "Hide/Show entry details" ) );
  QWhatsThis::add(
    mLocShowDetails,
    i18n( "Press this button to toggle the entry details display." ) );
  connect( mLocShowDetails, SIGNAL(clicked()), this, SLOT (showLocalIncidence()) );

  QLabel *remEntryLab = new QLabel( i18n( "Remote entry:" ), topFrame );
  infoGrid->addWidget( remEntryLab, 3, 0 );

  mRemEntryVal = new QLabel( "summary", topFrame );
  mRemEntryVal->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  infoGrid->addWidget( mRemEntryVal, 3, 1 );

  QLabel *remModLab = new QLabel( i18n( "Last modified:" ), topFrame );
  infoGrid->addWidget( remModLab, 4, 0 );

  mRemModVal = new QLabel( "modified date/time", topFrame );
  mRemModVal->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  infoGrid->addWidget( mRemModVal, 4, 1 );

  mRemShowDetails = new QPushButton( i18n( "Show &Details..." ), topFrame );
  mRemShowDetails->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  infoGrid->addWidget( mRemShowDetails, 4, 2 );
  QToolTip::add(
    mRemShowDetails,
    i18n( "Hide/Show entry details" ) );
  QWhatsThis::add(
    mRemShowDetails,
    i18n( "Press this button to toggle the entry details display." ) );
  connect( mRemShowDetails, SIGNAL(clicked()), this, SLOT (showRemoteIncidence()) );

  // row 3: take duration prefs
  mBg = new QButtonGroup( 4, Qt::Horizontal, i18n( "Take Option Duration" ), topFrame );
  mBg->setRadioButtonExclusive( true );
  lab = new QLabel(
    i18n( "<qt>Please choose how long the \"Take Option\" selected below "
          "should remain in effect:</qt>" ), mBg );
  mBg->setColumns( 1 );
  topLayout->addMultiCellWidget( mBg, iii, iii, 0, 1 );
  ++iii;

  QRadioButton *syncOnceBut = new QRadioButton(
    i18n( "Apply to all conflicts occurring during this synchronization" ), mBg );
  mBg->insert( syncOnceBut, KIncidenceChooser::Sync );
  QToolTip::add(
    syncOnceBut,
    i18n( "Ask only for the first conflict during a synchronization" ) );
  QWhatsThis::add(
    syncOnceBut,
    i18n( "This option says to show the conflict resolution dialog only for the "
          "first conflict that may occur during a synchronization. That is, do not "
          "show this dialog again until a conflict occurs during another synchronization." ) );

  const QString app = qApp->name();
  QRadioButton *syncSessionBut = new QRadioButton(
    i18n( "Apply to all conflicts during this %1 session" ).arg( app ), mBg );
  mBg->insert( syncSessionBut, KIncidenceChooser::Session );
  QToolTip::add(
    syncSessionBut,
    i18n( "Ask only for the first conflict after a %1 restart" ).arg( app ) );
  QWhatsThis::add(
    syncSessionBut,
    i18n( "This option says to show the conflict resolution dialog only for the "
          "first conflict that occurs since %1 was started. More specifically, "
          "do not show this dialog again during until %2 is restarted." ).arg( app, app ) );

  QRadioButton *syncPermanentBut = new QRadioButton(
    i18n( "Apply always and permanently" ), mBg );
  mBg->insert( syncPermanentBut, KIncidenceChooser::Never );
  QToolTip::add(
    syncPermanentBut,
    i18n( "Never ask again (may require system administrative help to undo!)" ) );
  QWhatsThis::add(
    syncPermanentBut,
    i18n( "This option says to never show th conflict resolution dialog again, always "
          "using the take option chosen specified now. Be aware that this option cannot "
          "be undone without editing a file by-hand and may require system administrative "
          "help to undo." ) );

  QRadioButton *syncAskBut = new QRadioButton(
    i18n( "Apply only to this conflict (ask each time)" ), mBg );
  mBg->insert( syncAskBut, KIncidenceChooser::Always );
  QToolTip::add(
    syncAskBut,
    i18n( "Ask for every conflict" ) );
  QWhatsThis::add(
    syncAskBut,
    i18n( "This option says to show the conflict resolution dialog "
          "for each and every conflict." ) );

  mBg->setButton( mAskPolicy );

  // duration options box (in row 3)
  QHBox *optGrid = new QHBox( mBg );
  QLabel *l = new QLabel( i18n( "The duration selection applies to:" ), optGrid );
  optGrid->setStretchFactor( l, 40 );

  mFolderAllBut = new QRadioButton( i18n( "all folders" ), optGrid );
  QToolTip::add(
    mFolderAllBut,
    i18n( "Applies this setting as the default duration for all folders" ) );
  QWhatsThis::add(
    mFolderAllBut,
    i18n( "This option says to apply the duration setting to all shared folders that "
          "do not have their own specific setting. That is, this is the default setting "
          "for the take duration." ) );
  optGrid->setStretchFactor( mFolderAllBut, 30 );

  mFolderOnlyBut = new QRadioButton( i18n( "this folder only" ), optGrid );
  QToolTip::add(
    mFolderOnlyBut,
    i18n( "Applies this setting as the duration for the folder \"%1\" only" ).arg( mFolder ) );
  QWhatsThis::add(
    mFolderOnlyBut,
    i18n( "This option says to apply the duration setting to the folder \"%1\" only. "
          "All other folders will use the default take duration setting." ).arg( mFolder ) );
  optGrid->setStretchFactor( mFolderOnlyBut, 30 );

  // init the folder buttons
  mFolderAllBut->setChecked( !mFolderOnly );
  mFolderOnlyBut->setChecked( mFolderOnly );
  connect( mFolderOnlyBut, SIGNAL(clicked()), this, SLOT(slotFolderOnly()) );
  connect( mFolderAllBut, SIGNAL(clicked()), this, SLOT(slotFolderAll()) );

  // row 4: "take" button box
  QButtonGroup *b_box = new QButtonGroup( 5, Qt::Horizontal, i18n( "Take Option" ), topFrame );
  b_box->setColumns( 1 );
  topLayout->addMultiCellWidget( b_box, iii, iii, 0, 1 );
  ++iii;

  lab = new QLabel(
    i18n( "<qt>Please choose which of the two entries shall be retained:</qt>" ), b_box );

  QPushButton *newBut = new QPushButton( i18n( "Take &Newer (last modified)" ), b_box );
  newBut->setFocus();
  connect( newBut, SIGNAL(clicked()), this, SLOT (takeNewerIncidence()) );
  QToolTip::add(
    newBut,
    i18n( "Take the newer version of the entry" ) );
  QWhatsThis::add(
    newBut,
    i18n( "A conflict was detected between your local copy of the entry "
          "and the remote entry on the server. Press the \"Take Newer\" button "
          "to use the version most recently modified, possibly overwriting your local copy" ) );

  QPushButton *remBut = new QPushButton( i18n( "Take &Remote (server-side)" ), b_box );
  connect( remBut, SIGNAL(clicked()), this, SLOT (takeRemoteIncidence()) );
  QToolTip::add(
    remBut,
    i18n( "Take the server copy of the entry" ) );
  QWhatsThis::add(
    remBut,
    i18n( "A conflict was detected between your local copy of the entry "
          "and the remote entry on the server. Press the \"Take Remote\" button "
          "to use the server copy, thereby overwriting your local copy" ) );

  QPushButton *locBut = new QPushButton( i18n( "Take &Local (client-side)" ), b_box );
  connect( locBut, SIGNAL(clicked()), this, SLOT (takeLocalIncidence()) );
  QToolTip::add(
    locBut,
    i18n( "Take your local copy of the entry" ) );
  QWhatsThis::add(
    locBut,
    i18n( "A conflict was detected between your local copy of the entry "
          "and the remote entry on the server. Press the \"Take Local\" button "
          "to make sure your local copy is used." ) );

  QPushButton *bothBut =
    new QPushButton( i18n( "Take &Both (resulting in two different, parallel entries)" ), b_box );
  connect( bothBut, SIGNAL(clicked()), this, SLOT (takeBothIncidence()) );
  QToolTip::add(
    bothBut,
    i18n( "Take both copies of the entry" ) );
  QWhatsThis::add(
    bothBut,
    i18n( "A conflict was detected between your local copy of the entry "
          "and the remote entry on the server. Press the \"Take Both\" button "
          "to keep both the local and the server copies, resulting in "
          "two differing entries in parallel." ) );

  // final settings

  mTbL = 0;
  mTbN =  0;
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
}

void KIncidenceChooser::setIncidences( Incidence *local, Incidence *remote )
{
  mLocInc = local;
  mRemInc = remote;
  setLabels();
}

Incidence *KIncidenceChooser::takeIncidence()
{
  return mSelIncidence;
}

KIncidenceChooser::TakeMode KIncidenceChooser::takeMode()
{
  return mTakeMode;
}

void KIncidenceChooser::setConflictAskPolicy( ConflictAskPolicy policy )
{
  mAskPolicy = policy;
}

KIncidenceChooser::ConflictAskPolicy KIncidenceChooser::conflictAskPolicy()
{
  return mAskPolicy;
}

void KIncidenceChooser::setFolderOnly( bool folderOnly )
{
  mFolderOnly = folderOnly;
}

bool KIncidenceChooser::folderOnly()
{
  return mFolderOnly;
}

void KIncidenceChooser::useGlobalMode()
{
  if ( mAskPolicy != KIncidenceChooser::Always ) {
    QDialog::reject();
  }
}

QString KIncidenceChooser::summaryStr( Incidence *incidence ) const
{
  static QString etc = i18n( "elipsis", "..." );

  uint maxLen = 30;
  QString s = incidence->summary();
  if ( s.isEmpty() ) {
    return i18n( "unspecified" );
  } else {
    if ( s.length() > maxLen ) {
      maxLen -= etc.length();
      s = s.left( maxLen );
      s += etc;
    }
    return s;
  }
}

QString KIncidenceChooser::modifiedStr( Incidence *incidence ) const
{
  return KGlobal::locale()->formatDateTime( incidence->lastModified() );
}

void KIncidenceChooser::setLabels()
{
  mLocEntryVal->setText( summaryStr( mLocInc ) );
  mLocModVal->setText( modifiedStr( mLocInc ) );

  mRemEntryVal->setText( summaryStr( mRemInc ) );
  mRemModVal->setText( modifiedStr( mRemInc ) );
}

void KIncidenceChooser::detailsDialogClosed()
{
  KDialogBase* dialog = static_cast<KDialogBase *>( const_cast<QObject *>( sender() ) );
  if ( dialog == mTbL ) {
    mLocShowDetails->setText( i18n( "Show details..." ) );
  } else {
    mRemShowDetails->setText( i18n( "Show details..." ) );
  }
}

void KIncidenceChooser::slotFolderAll()
{
  mFolderOnlyBut->setChecked( false );
}

void KIncidenceChooser::slotFolderOnly()
{
  mFolderAllBut->setChecked( false );
}

void KIncidenceChooser::showLocalIncidence()
{
  if ( mTbL ) {
    if ( mTbL->isVisible() ) {
      mLocShowDetails->setText( i18n( "Show Details..." ) );
      mTbL->hide();
    } else {
      mLocShowDetails->setText( i18n( "Hide Details" ) );
      mTbL->show();
      mTbL->raise();
    }
    return;
  }
  mTbL = new KDialogBase( this, "", false/*not modal*/, mLocEntryVal->text(), KDialogBase::Ok );
  mTbL->setEscapeButton( KDialogBase::Ok );
  connect( mTbL, SIGNAL(okClicked()), this, SLOT(detailsDialogClosed()) );
  QTextBrowser *textBrowser = new QTextBrowser( mTbL );
  mTbL->setMainWidget( textBrowser );
  textBrowser->setText( IncidenceFormatter::extensiveDisplayStr( 0, mLocInc ) );
  QToolTip::add( textBrowser, i18n( "Incidence details" ) );
  QWhatsThis::add( textBrowser, i18n( "This area shows the entry details" ) );
  mTbL->setMinimumSize( 400, 400 );
  mLocShowDetails->setText( i18n( "Hide Details" ) );
  mTbL->show();
  mTbL->raise();
}

void KIncidenceChooser::showRemoteIncidence()
{
  if ( mTbN ) {
    if ( mTbN->isVisible() ) {
      mRemShowDetails->setText( i18n( "Show Details" ) );
      mTbN->hide();
    } else {
      mRemShowDetails->setText( i18n( "Hide Details" ) );
      mTbN->show();
      mTbN->raise();
    }
    return;
  }
  mTbN = new KDialogBase( this, "", false/*not modal*/, mRemEntryVal->text(), KDialogBase::Ok );
  mTbN->setEscapeButton( KDialogBase::Ok );
  connect( mTbN, SIGNAL(okClicked()), this, SLOT(detailsDialogClosed()) );
  QTextBrowser *textBrowser = new QTextBrowser( mTbN );
  mTbN->setMainWidget( textBrowser );
  textBrowser->setText( IncidenceFormatter::extensiveDisplayStr( 0, mRemInc ) );
  QToolTip::add( textBrowser, i18n( "Incidence details" ) );
  QWhatsThis::add( textBrowser, i18n( "This area shows the entry details" ) );
  mTbN->setMinimumSize( 400, 400 );
  mRemShowDetails->setText( i18n( "Hide Details" ) );
  mTbN->show();
  mTbN->raise();
}

void KIncidenceChooser::takeNewerIncidence()
{
  if ( mLocInc->lastModified() == mRemInc->lastModified() ) {
    mSelIncidence = 0;
  } else if ( mLocInc->lastModified() >  mRemInc->lastModified() ) {
    mSelIncidence =  mLocInc;
  } else {
    mSelIncidence = mRemInc;
  }
  mAskPolicy = ( ConflictAskPolicy )mBg->selectedId ();
  mTakeMode = Newer;
  mFolderOnly = mFolderOnlyBut->isChecked();
  QDialog::accept();
}

void KIncidenceChooser::takeLocalIncidence()
{
  mSelIncidence = mLocInc;
  mAskPolicy = ( ConflictAskPolicy )mBg->selectedId ();
  mTakeMode = Local;
  mFolderOnly = mFolderOnlyBut->isChecked();
  QDialog::accept();
}

void KIncidenceChooser::takeRemoteIncidence()
{
  mSelIncidence = mRemInc;
  mAskPolicy = ( ConflictAskPolicy )mBg->selectedId ();
  mTakeMode = Remote;
  mFolderOnly = mFolderOnlyBut->isChecked();
  QDialog::accept();
}

void KIncidenceChooser::takeBothIncidence()
{
  mSelIncidence = 0;
  mAskPolicy = ( ConflictAskPolicy )mBg->selectedId ();
  mTakeMode = Both;
  mFolderOnly = mFolderOnlyBut->isChecked();
  QDialog::accept();
}

void KIncidenceChooser::closeEvent( QCloseEvent *e )
{
  Q_UNUSED( e );
  KMessageBox::sorry( parentWidget(),
                      i18n( "Sorry, you must select an entry from the conflict." ) );
}

void KIncidenceChooser::keyPressEvent( QKeyEvent *e )
{
  if ( e->state() == 0 && e->key() == Key_Escape ) {
    KMessageBox::sorry( parentWidget(),
                        i18n( "Sorry, you must select an entry from the conflict." ) );
    e->ignore();
    return;
  }
  KDialog::keyPressEvent( e );
}

#include "kincidencechooser.moc"
