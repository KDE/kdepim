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

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqbuttongroup.h>
#include <tqvbox.h>
#include <tqhbox.h>
#include <tqradiobutton.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqscrollview.h>
#include <tqtextbrowser.h>
#include <tqapplication.h>


#include <klocale.h>
#include <kglobal.h>

#include "kincidencechooser.h"
#include "libkcal/incidence.h"
#include "libkcal/incidenceformatter.h"

int KIncidenceChooser::chooseMode = KIncidenceChooser::ask ;

KIncidenceChooser::KIncidenceChooser(TQWidget *parent, char *name) :
    KDialog(parent,name,true)
{
    KDialog *topFrame = this;
    TQGridLayout *topLayout = new TQGridLayout(topFrame,5,3);
    int iii = 0;
    setCaption( i18n("Conflict Detected"));
    TQLabel * lab;
    lab = new TQLabel( i18n(
                        "<qt>A conflict was detected. This probably means someone edited the same entry on the server while you changed it locally."
                        "<br/>NOTE: You have to check mail again to apply your changes to the server.</qt>"), topFrame);
    topLayout->addMultiCellWidget(lab, iii,iii,0,2);
    ++iii;
    TQHBox * b_box = new TQHBox( topFrame );
    topLayout->addMultiCellWidget(b_box, iii,iii,0,2);
    ++iii;
    TQPushButton* button = new TQPushButton( i18n("Take Local"), b_box );
    connect ( button, TQT_SIGNAL( clicked()), this, TQT_SLOT (takeIncidence1() ) );
    button = new TQPushButton( i18n("Take New"), b_box );
    connect ( button, TQT_SIGNAL( clicked()), this, TQT_SLOT (takeIncidence2() ) );
    button = new TQPushButton( i18n("Take Both"), b_box );
    connect ( button, TQT_SIGNAL( clicked()), this, TQT_SLOT (takeBoth() ) );
    topLayout->setSpacing(spacingHint());
    topLayout->setMargin(marginHint());
    // text is not translated, because text has to be set later
    mInc1lab = new TQLabel ( i18n("Local incidence"), topFrame);
    topLayout->addWidget(mInc1lab ,iii,0);
    mInc1Sumlab = new TQLabel ( i18n("Local incidence summary"), topFrame);
    topLayout->addMultiCellWidget(mInc1Sumlab, iii,iii,1,2);
    ++iii;
    topLayout->addWidget( new TQLabel ( i18n("Last modified:"), topFrame) ,iii,0);
    mMod1lab = new TQLabel ( "Set Last modified", topFrame);
    topLayout->addWidget(mMod1lab,iii,1);
    mShowDetails1 = new TQPushButton( i18n("Show Details"),topFrame );
    connect ( mShowDetails1, TQT_SIGNAL( clicked()), this, TQT_SLOT (showIncidence1() ) );
    topLayout->addWidget(mShowDetails1,iii,2);
    ++iii;

    mInc2lab = new TQLabel ( "Local incidence", topFrame);
    topLayout->addWidget(mInc2lab,iii,0);
    mInc2Sumlab = new TQLabel ( "Local incidence summary", topFrame);
    topLayout->addMultiCellWidget(mInc2Sumlab, iii,iii,1,2);
    ++iii;
    topLayout->addWidget( new TQLabel ( i18n("Last modified:"), topFrame) ,iii,0);
    mMod2lab = new TQLabel ( "Set Last modified", topFrame);
    topLayout->addWidget(mMod2lab,iii,1);
    mShowDetails2 = new TQPushButton( i18n("Show Details"), topFrame);
    connect ( mShowDetails2, TQT_SIGNAL( clicked()), this, TQT_SLOT (showIncidence2() ) );
    topLayout->addWidget(mShowDetails2,iii,2);
    ++iii;
    //
#if 0
    // commented out for now, because the diff code has too many bugs
    mDiffBut = new TQPushButton( i18n("Show Differences"), topFrame );
    connect ( mDiffBut, TQT_SIGNAL( clicked()), this, TQT_SLOT ( showDiff() ) );
    topLayout->addMultiCellWidget(mDiffBut, iii,iii,0,2);
    ++iii;
#else
    mDiffBut = 0;
#endif
    mBg = new TQButtonGroup ( 1,  Qt::Horizontal, i18n("Sync Preferences"), topFrame);
    topLayout->addMultiCellWidget(mBg, iii,iii,0,2);
    ++iii;
    mBg->insert( new TQRadioButton ( i18n("Take local entry on conflict"), mBg ), KIncidenceChooser::local);
    mBg->insert( new TQRadioButton ( i18n("Take new (remote) entry on conflict"), mBg ),  KIncidenceChooser::remote);
    mBg->insert( new TQRadioButton ( i18n("Take newest entry on conflict"), mBg ), KIncidenceChooser::newest );
    mBg->insert( new TQRadioButton ( i18n("Ask for every entry on conflict"), mBg ),KIncidenceChooser::ask );
    mBg->insert( new TQRadioButton ( i18n("Take both on conflict"), mBg ), KIncidenceChooser::both );
    mBg->setButton ( chooseMode );
    mTbL = 0;
    mTbN =  0;
    mDisplayDiff = 0;
    mSelIncidence = 0;
    button = new TQPushButton( i18n("Apply This to All Conflicts of This Sync"), topFrame );
    connect ( button, TQT_SIGNAL( clicked()), this, TQT_SLOT ( setSyncMode() ) );
    topLayout->addMultiCellWidget(button, iii,iii,0,2);
}

KIncidenceChooser::~KIncidenceChooser()
{
    if ( mTbL ) delete mTbL;
    if ( mTbN ) delete mTbN;
    if ( mDisplayDiff ) {
        delete mDisplayDiff;
        delete diff;
    }
}

void KIncidenceChooser::setIncidence( KCal::Incidence* local ,KCal::Incidence* remote )
{
    mInc1 = local;
    mInc2 = remote;
    setLabels();

}
KCal::Incidence* KIncidenceChooser::getIncidence( )
{

    KCal::Incidence* retval = mSelIncidence;
    if ( chooseMode == KIncidenceChooser::local )
        retval = mInc1;
    else  if ( chooseMode == KIncidenceChooser::remote )
        retval = mInc2;
    else  if ( chooseMode == KIncidenceChooser::both ) {
        retval = 0;
    }
    else  if ( chooseMode == KIncidenceChooser::newest ) {
        if ( mInc1->lastModified() == mInc2->lastModified())
            retval = 0;
        if ( mInc1->lastModified() >  mInc2->lastModified() )
            retval =  mInc1;
        else
            retval = mInc2;
    }
    return retval;
}

void KIncidenceChooser::setSyncMode()
{
    chooseMode = mBg->selectedId ();
    if ( chooseMode != KIncidenceChooser::ask )
        TQDialog::accept();

}

void KIncidenceChooser::useGlobalMode()
{
    if ( chooseMode != KIncidenceChooser::ask )
        TQDialog::reject();
}

void KIncidenceChooser::setLabels()
{
  KCal::Incidence* inc = mInc1;
    TQLabel* des = mInc1lab;
    TQLabel * sum = mInc1Sumlab;


    if ( inc->type() == "Event" ) {
        des->setText( i18n( "Local Event") );
        sum->setText( inc->summary().left( 30 ));
        if ( mDiffBut )
            mDiffBut->setEnabled( true );
    }
    else if ( inc->type() == "Todo" ) {
        des->setText( i18n( "Local Todo") );
        sum->setText( inc->summary().left( 30 ));
        if ( mDiffBut )
            mDiffBut->setEnabled( true );

    }
    else if ( inc->type() == "Journal" ) {
        des->setText( i18n( "Local Journal") );
        sum->setText( inc->description().left( 30 ));
        if ( mDiffBut )
            mDiffBut->setEnabled( false );
    }
    mMod1lab->setText( KGlobal::locale()->formatDateTime(inc->lastModified() ));
    inc = mInc2;
    des = mInc2lab;
    sum = mInc2Sumlab;
    if ( inc->type() == "Event" ) {
        des->setText( i18n( "New Event") );
        sum->setText( inc->summary().left( 30 ));
    }
    else if ( inc->type() == "Todo" ) {
        des->setText( i18n( "New Todo") );
        sum->setText( inc->summary().left( 30 ));

    }
    else if ( inc->type() == "Journal" ) {
        des->setText( i18n( "New Journal") );
        sum->setText( inc->description().left( 30 ));

    }
    mMod2lab->setText( KGlobal::locale()->formatDateTime(inc->lastModified() ));
}

void KIncidenceChooser::showIncidence1()
{
    if ( mTbL ) {
        if ( mTbL->isVisible() ) {
            mShowDetails1->setText( i18n("Show Details"));
            mTbL->hide();
        } else {
            mShowDetails1->setText( i18n("Hide Details"));
            mTbL->show();
            mTbL->raise();
        }
        return;
    }
    mTbL = new KDialogBase( this, "", false /*not modal*/, mInc1lab->text(), KDialogBase::Ok );
    mTbL->setEscapeButton( KDialogBase::Ok );
    connect( mTbL, TQT_SIGNAL( okClicked() ), this, TQT_SLOT( detailsDialogClosed() ) );
    TQTextBrowser* textBrowser = new TQTextBrowser( mTbL );
    mTbL->setMainWidget( textBrowser );
    textBrowser->setText( KCal::IncidenceFormatter::extensiveDisplayString( mInc1 )  );
    mTbL->setMinimumSize( 400, 400 );
    mShowDetails1->setText( i18n("Hide Details"));
    mTbL->show();
    mTbL->raise();
}

void KIncidenceChooser::detailsDialogClosed()
{
    KDialogBase* dialog = static_cast<KDialogBase *>( const_cast<TQObject *>( sender() ) );
    if ( dialog == mTbL )
        mShowDetails1->setText( i18n( "Show details..." ) );
    else
        mShowDetails2->setText( i18n( "Show details..." ) );
}

void KIncidenceChooser::showDiff()
{
    if ( mDisplayDiff ) {
        mDisplayDiff->show();
        mDisplayDiff->raise();
        return;
    }
    mDisplayDiff = new KPIM::HTMLDiffAlgoDisplay (this);
    if ( mInc1->summary().left( 20 ) != mInc2->summary().left( 20 ) )
        mDisplayDiff->setCaption( i18n( "Differences of %1 and %2").arg( mInc1->summary().left( 20 ) ).arg( mInc2->summary().left( 20 ) ) );
    else
        mDisplayDiff->setCaption( i18n( "Differences of %1").arg( mInc1->summary().left( 20 ) ) );

    diff = new KPIM::CalendarDiffAlgo( mInc1, mInc2);
    diff->setLeftSourceTitle(  i18n( "Local entry"));
    diff->setRightSourceTitle(i18n( "New (remote) entry") );
    diff->addDisplay( mDisplayDiff );
    diff->run();
    mDisplayDiff->show();
    mDisplayDiff->raise();
}

void KIncidenceChooser::showIncidence2()
{
   if ( mTbN ) {
        if ( mTbN->isVisible() ) {
            mShowDetails2->setText( i18n("Show Details"));
            mTbN->hide();
        } else {
            mShowDetails2->setText( i18n("Hide Details"));
            mTbN->show();
            mTbN->raise();
        }
        return;
    }
    mTbN = new KDialogBase( this, "", false /*not modal*/, mInc2lab->text(), KDialogBase::Ok );
    mTbN->setEscapeButton( KDialogBase::Ok );
    connect( mTbN, TQT_SIGNAL( okClicked() ), this, TQT_SLOT( detailsDialogClosed() ) );
    TQTextBrowser* textBrowser = new TQTextBrowser( mTbN );
    mTbN->setMainWidget( textBrowser );
    textBrowser->setText( KCal::IncidenceFormatter::extensiveDisplayString( mInc2 ) );
    mTbN->setMinimumSize( 400, 400 );
    mShowDetails2->setText( i18n("Hide Details"));
    mTbN->show();
    mTbN->raise();
}

void KIncidenceChooser::takeIncidence1()
{
    mSelIncidence = mInc1;
    TQDialog::accept();
}

void KIncidenceChooser::takeIncidence2()
{
    mSelIncidence = mInc2;
    TQDialog::accept();
}

void KIncidenceChooser::takeBoth()
{

    mSelIncidence = 0;
    TQDialog::accept();
}


#include "kincidencechooser.moc"
