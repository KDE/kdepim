/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include "statusbar.h"
#include <libkmobiletools/overlaywidget.h>
#include <libkmobiletools/engineslist.h>

#include <kpushbutton.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kicon.h>
#include <kdebug.h>
#include <qlabel.h>
#include <qtimer.h>
#include <q3scrollview.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QEvent>
#include <kstatusbar.h>
#include <qapplication.h>
#include <qlayout.h>

#define PIX_UP KIcon( "up" )
#define PIX_DOWN KIcon( "down" )

StatusBarScrollView::StatusBarScrollView(QWidget *parent, const char *name, Qt::WFlags f)
    : Q3ScrollView(parent, name, f)
{
    setFrameStyle( NoFrame );
    setResizePolicy( Q3ScrollView::AutoOneFit );
    vbox=new Q3VBox( viewport() );
    vbox->setSpacing(5);
    vbox->show();
    vbox->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum);
    addChild(vbox);
}

StatusBarScrollView::~ StatusBarScrollView()
{
}

// From KDEPIM libkdepim/progressdialog.cpp
/** -*- c++ -*-
* progressdialog.cpp
*
*  Copyright (c) 2004 Till Adam <adam@kde.org>,
*                     David Faure <faure@kde.org>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; version 2 of the License
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*  In addition, as a special exception, the copyright holders give
*  permission to link the code of this program with any edition of
*  the Qt library by Trolltech AS, Norway (or with modified versions
*  of Qt that use the same license as Qt), and distribute linked
*  combinations including the two.  You must obey the GNU General
*  Public License in all respects for all of the code used other than
*  Qt.  If you modify this file, you may extend this exception to
*  your version of the file, but you are not obligated to do so.  If
*  you do not wish to do so, delete this exception statement from
*  your version.
*/
void StatusBarScrollView::resizeContents( int w, int h )
{
  //kDebug(5300) << k_funcinfo << w <<"," << h;
    Q3ScrollView::resizeContents( w, h );
  // Tell the layout in the parent (progressdialog) that our size changed
    updateGeometry();
  // Resize the parent (progressdialog) - this works but resize horizontally too often
  //parentWidget()->adjustSize();

    QApplication::sendPostedEvents( 0, QEvent::ChildInserted );
    QApplication::sendPostedEvents( 0, QEvent::LayoutHint );
    QSize sz = parentWidget()->sizeHint();
    int currentWidth = parentWidget()->width();
  // Don't resize to sz.width() every time when it only reduces a little bit
    if ( currentWidth < sz.width() || currentWidth > sz.width() + 100 )
        currentWidth = sz.width();
    parentWidget()->resize( currentWidth, sz.height() );
}

QSize StatusBarScrollView::sizeHint() const
{
    return minimumSizeHint();
}

QSize StatusBarScrollView::minimumSizeHint() const
{
    int f = 2 * frameWidth();
  // Make room for a vertical scrollbar in all cases, to avoid a horizontal one
    int vsbExt = verticalScrollBar()->sizeHint().width();
    int minw = topLevelWidget()->width() / 4;
    int maxh = topLevelWidget()->height() / 2;
    QSize sz( vbox->minimumSizeHint() );
    sz.setWidth( qMax( sz.width(), minw ) + f + (vsbExt /4) );
    sz.setHeight( qMin( sz.height(), maxh ) + f );
    return sz;
}

// here ends code from KDEPIM

StatusBarProgressBox::StatusBarProgressBox( KStatusBar *statusbar, QWidget * parent, const char * name)
    : Q3HBox(0, name, 0)
        , b_shown(false), jobsCount(0)
{
    jobs.setAutoDelete(false);
    parentWidget=parent;
    showHideButton=new KPushButton(this);
    generalProgress=new QProgressBar(this);
    generalProgress->setObjectName(QLatin1String("generalProgress"));
    showHideButton->setIcon( PIX_UP );
    generalProgress->setMaximumSize(70,16);
    showHideButton->setMaximumHeight(16);
    overlay=new KMobileTools::OverlayWidget( statusbar, parent );
//     scrollView=new QScrollView(parent, 0, Qt::WStyle_Customize | Qt::WStyle_NoBorder | WType_TopLevel);
    scrollView=new StatusBarScrollView(overlay);
    itemsBox=scrollView->getVBox();
    scrollView->show();
    overlay->hide();
    connect(showHideButton, SIGNAL(clicked()), this, SLOT(slotShowHide()) );
//     hide();
    connect(this, SIGNAL(totalProgressChanged(int ) ), generalProgress, SLOT(setProgress(int ) ) );
// #ifndef NO_DEBUG
// #warning Compiling with debug flags, the statusbar job progressbar will start in verbose mode
//     slotShowHide();
// #endif
}

StatusBarProgressBox::~ StatusBarProgressBox()
{
}

void StatusBarProgressBox::slotJobEnqueued( KMobileTools::Job *job)
{
    showHideButton->show();
    generalProgress->show();
    jobsCount++;
    StatusBarJob *newjob=new StatusBarJob(job->typeString(), job, this);
    newjob->hide();
    QTimer::singleShot(20, newjob, SLOT(show() ) );
    jobs.append(newjob);
}

void StatusBarProgressBox::slotDeletedJob(StatusBarJob* job)
{
    jobsCount--; 
    jobs.remove(job);
    if(! jobsCount)
    {
        showHideButton->hide();
        generalProgress->hide();
    }
}

void StatusBarProgressBox::countTotalProgress()
{
    Q3PtrListIterator<StatusBarJob> it(jobs);
    StatusBarJob *curjob=0;
    int totalprogress=0;
    while ( (curjob=it.current()) )
    {
        totalprogress+=curjob->progress();
        ++it;
    }
    totalprogress/=jobs.count();
    emit totalProgressChanged(totalprogress);
}

void StatusBarProgressBox::slotShowHide()
{
    if(b_shown)
    {
        overlay->hide();
        showHideButton->setIcon(PIX_UP);
        b_shown=false;
        return;
    }
//     itemsBox->adjustSize();
//     itemsBox->move( parentWidget->width() - itemsBox->width(), parentWidget->height() - itemsBox->height() );
    overlay->show();
    showHideButton->setIcon(PIX_DOWN);
    b_shown=true;
}

SingleJobProgressBox::SingleJobProgressBox( int jobType, const QString &description, QWidget *parent, const char *name)
    : Q3HBox(parent, name, 0)
{
    setSpacing(10);
    QLabel *iconLabel=new QLabel(this);
    switch( jobType ){
        case KMobileTools::Job::initPhone:
            s_itemLabelName="connect_creating"; break;
        case KMobileTools::Job::fetchSMS:
        case KMobileTools::Job::selectSMSSlot:
        case KMobileTools::Job::smsFolders:
        case KMobileTools::Job::sendStoredSMS:
            s_itemLabelName="mail_generic"; break;
        case KMobileTools::Job::sendSMS:
        case KMobileTools::Job::storeSMS:
            s_itemLabelName="mail_forward"; break;
        case KMobileTools::Job::delAddressee:
        case KMobileTools::Job::addAddressee:
        case KMobileTools::Job::editAddressee:
        case KMobileTools::Job::fetchAddressBook:
            s_itemLabelName="kontact_contacts"; break;
        case KMobileTools::Job::syncDateTimeJob:
            s_itemLabelName="kalarm"; break;
        case KMobileTools::Job::testPhoneFeatures:
            s_itemLabelName="gear"; break;
        case KMobileTools::Job::fetchKCal:
            s_itemLabelName="date"; break;
        default:
            s_itemLabelName="kmobiletools"; break;
    }
    iconLabel->setPixmap( KIconLoader::global()->loadIcon(s_itemLabelName, K3Icon::NoGroup, K3Icon::SizeMedium) );
    Q3VBox *vlayout=new Q3VBox(this);
    itemNameLabel=new QLabel(description, vlayout);
    itemProgress=new QProgressBar(vlayout);
    itemProgress->setMaximumSize(150,16);
    setFrameShape(Q3Frame::PopupPanel);
    setFrameShadow(Q3Frame::Sunken);
    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum);
}

SingleJobProgressBox::~ SingleJobProgressBox()
{
}

void SingleJobProgressBox::setPercent(int p)
{
    itemProgress->setValue(p);
}


StatusBarJob::StatusBarJob(const QString &description, KMobileTools::Job *job,StatusBarProgressBox * parent, const char *name)
    : QObject(parent)
{
    setObjectName(QLatin1String(name));
    box=new SingleJobProgressBox(job->type(), description, parent->statusItemsBox(), name);
    q_iconLabel=new QLabel(parent);
    q_iconLabel->setObjectName(QLatin1String("icon"));
    q_iconLabel->setPixmap( KIconLoader::global()->loadIcon(box->iconLabelName(), K3Icon::NoGroup, K3Icon::SizeSmall) );
    q_iconLabel->setToolTip(job->typeString());
    q_iconLabel->show();
    parentBox=parent;
    connect(job, SIGNAL(percentDone(int ) ), box, SLOT(setPercent(int ) ) );
    connect(job, SIGNAL(percentDone(int ) ), parentBox, SLOT(countTotalProgress() ) );
    connect(job, SIGNAL(done(Job* ) ), this, SLOT(jobDone() ) );
}

StatusBarJob::~ StatusBarJob()
{

}

void StatusBarJob::jobDone()
{
    box->setPercent(100);
    parentBox->countTotalProgress();
    QTimer::singleShot( 1000, this, SLOT(deleteThis() ) );
}

void StatusBarJob::deleteThis()
{
    if( KMobileTools::EnginesList::instance()->closing() ) return;
    delete box;
    q_iconLabel->setParent(0);
    delete q_iconLabel;
    q_iconLabel=0L;
    parentBox->slotDeletedJob(this);
    delete this;
}

#include "statusbar.moc"
