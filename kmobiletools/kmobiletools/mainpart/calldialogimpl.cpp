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
#include "calldialogimpl.h"
#include <k3activelabel.h>
#include <qtimer.h>
#include <qlabel.h>
#include <kdebug.h>

#include <libkmobiletools/engine.h>
#include <libkmobiletools/weaver.h>

callDialogImpl::callDialogImpl(KMobileTools::Engine *engine, QWidget *parent, const char *name)
 : KDialog(parent), b_dialing(false), b_labelSet(false)
{
    ui.setupUi(this);
    setObjectName(QLatin1String(name));
    setWindowFlags(Qt::WindowStaysOnTopHint);
    this->engine=engine;
    kDebug() << "callDialogImpl::callDialogImpl()\n";
    disconnect(ui.timerStart, SIGNAL(linkClicked(const QString &)), ui.timerStart, SLOT(openLink(const QString &)));
    connect(ui.timerStart, SIGNAL(linkClicked(const QString &)), this, SLOT(slotTimerStart()));
    ui.timerStack->setCurrentIndex(0);
    resize(minimumSize());
}


callDialogImpl::~callDialogImpl()
{
}

void callDialogImpl::slotTimerStart()
{
    ui.timerStack->setCurrentIndex(1);
    time.start();
    slotTimerPoll();
    QTimer *p_timer=new QTimer(this);
    p_timer->setSingleShot(false);
    connect(p_timer, SIGNAL(timeout()), this, SLOT(slotTimerPoll()));
    p_timer->start(1000);
}

void callDialogImpl::slotTimerPoll()
{
    ui.timer->setText( QTime().addMSecs(time.elapsed()).toString() );
}

#include "calldialogimpl.moc"


/*!
    \fn callDialogImpl::done(int)
 */
void callDialogImpl::done(int r)
{
    /// @todo implement me
    if(b_dialing) {
        QTimer::singleShot(500, this, SLOT(accept()));
        endCall();
        return;
    }
    KDialog::done(r);
}


/*!
    \fn callDialogImpl::call(const QString &number, const QString &showName=QString::null)
 */
int callDialogImpl::call(const QString &number, const QString &showName)
{
    this->number=number;
    if(showName.isNull())
    {
        setWindowTitle(windowTitle().arg(number) );
        ui.lInfo->setText(i18n("<qt>Calling phone number: <b>%1</b><br/><br/></qt>",
                number));
    }
    else
    {
        ui.lInfo->setText(i18n("<qt>Calling <b>%1</b><br/>Phone number: <b>%2</b><br/><br/></qt>",
                showName, number));
        setWindowTitle(windowTitle().arg(showName));
    }
    connect(engine->ThreadWeaver(), SIGNAL(suspended()), this, SLOT(triggerCall()));
    engine->suspendStatusJobs( true );
    engine->ThreadWeaver()->suspend();
    kDebug() << "callDialogImpl: suspending jobs in engine\n";
    ui.lStatus->setText(i18n("Suspending current tasks before calling."));
    return exec();
}


/*!
    \fn callDialogImpl::triggerCall()
 */
void callDialogImpl::triggerCall()
{
    disconnect(engine->ThreadWeaver(), SIGNAL(suspended()), this, SLOT(triggerCall()));
    b_dialing=true;
    ui.lStatus->setText(i18n("Dialing number."));
    kDebug() << "callDialogImpl: jobs suspended, now dialing number\n";
    ui.lStatus->setText(i18n("Phone status: calling."));
    engine->slotDial( KMobileTools::Engine::DIAL_DIAL, number );

}


/*!
    \fn callDialogImpl::endCall()
 */
void callDialogImpl::endCall()
{
    if( ! b_labelSet )
    {
        b_labelSet=true;
        ui.lStatus->setText(i18n("Phone status: closing call."));
        QTimer::singleShot(100, this, SLOT(endCall()) );
        return;
    }
    engine->slotDial( KMobileTools::Engine::DIAL_HANGUP );
    engine->suspendStatusJobs( false );
    kDebug() << "DevicePart: call finished, now resuming job queue\n";
    engine->ThreadWeaver()->resume();
    b_dialing=false;
}
