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
#include "engine.h"
#include <qlist.h>
#include <kplugininfo.h>
#include <kdebug.h>
#include <klocale.h>

#include "engineslist.h"
#include "weaver.h"
#include "enginedata.h"

#include "smslist.h"

using namespace KMobileTools;

class KMobileTools::EnginePrivate {
public:
    EnginePrivate() : p_pluginInfo(NULL), enginedata(NULL)
    {}
        KPluginInfo *p_pluginInfo;
        SMSList *p_diffSMSList;
        KMobileTools::Weaver *weaver;

        QList<KMobileTools::Job*> jobs;

        /**
         * Enqueue a job.
         */
        void enqueueJob(Job *job);

        int i_currentPBMemSlot;

        /**
         * New SMS number.
         */
        int s_newSMS;

        /**
         * Total SMS number.
         */
        int s_totalSMS;

        /**
         * Phone SMS folders.
         */
        QStringList s_folders;

        /**
         * Phone SMS slot used.
         */
        int sms_slot;

        int i_suspendStatusJobs;

        bool b_ownweaver;

        KMobileTools::EngineData *enginedata;

};

Engine::Engine( QObject *parent, const QString &name)
    : QObject(parent), d(new EnginePrivate)
{
    setObjectName(name);
    d->enginedata=new EngineData(this);
//     b_ownweaver=ownWeaver;
    /*if(ownWeaver) */d->weaver=new KMobileTools::Weaver(this/*, name, 2, 2*/);
//     else weaver=ThreadWeaver::Weaver::instance();
    d->p_diffSMSList=new SMSList();
    connect(d->weaver, SIGNAL(jobDone(KMobileTools::Job*) ), SLOT(processSlot(KMobileTools::Job*) ) );
    connect(d->weaver, SIGNAL(suspended() ), this, SLOT(slotWeaverSuspended() ) );

    engineData()->setManufacturerID(Unknown);
    d->s_newSMS = 0;
    d->s_totalSMS = 0;
    d->i_suspendStatusJobs=0;
    EnginesList::instance()->append( this );
}


KMobileTools::EngineData* Engine::engineData() {
    return d->enginedata;
}

const KMobileTools::EngineData* Engine::constEngineData() const {
    return static_cast<const EngineData*>( d->enginedata );
}


Engine::~Engine()
{
    kDebug() << "Engine::~Engine()\n";
//     weaver->dequeue();
//     weaver->finish();
//     delete weaver;
//     weaver=NULL;

//     delete p_addresseeList;
    delete d->p_diffSMSList;

    EnginesList::instance()->remove( this );
    delete d;
}

void Engine::processSlot(KMobileTools::Job* job)
{
    if( ! job->inherits("KMobileTools::Job")) return; ///@TODO look for namespace
KMobileTools::Job *kjob=(Job*) job;
    emit jobFinished(kjob->type() );
}

KMobileTools::Weaver *Engine::ThreadWeaver()
{
    return d->weaver;
}

int Engine::newSMSCount()
{
    return d->s_newSMS;
}

int Engine::totalSMSCount()
{
    return d->s_totalSMS;
}



SMSList *Engine::diffSMSList() const
{
    return d->p_diffSMSList;
}

QStringList Engine::smsFolders()
{
    return d->s_folders;
}

void Engine::setSMSSlot(int slot)
{
    d->sms_slot = slot;
}

int Engine::smsSlot()
{
    return d->sms_slot;
}

void Engine::queryClose()
{
    kDebug() << "Engine::queryClose()\n";
//     if(!ownWeaver) return;
    d->weaver->dequeue();
    d->weaver->suspend();
    d->weaver->finish();
    delete d->weaver;
    d->weaver=NULL;
}

void Engine::enqueueJob(KMobileTools::Job *job)
{
    d->weaver->enqueue(job);
    emit jobEnqueued(job);
}

void Engine::slotSwitchToFSMode()
{
    slotStopDevice();
}


KPluginInfo *Engine::pluginInfo()
{
    return EnginesList::instance()->engineInfo(engineLibName(), true );
}

QString Engine::currentDeviceName() const
{
    return QString(); // ### FIXME
}

QString Engine::shortDesc()
{
    return pluginInfo()->comment();
}

QString Engine::longDesc()
{
    return pluginInfo()->property("X-KMobileTools-LongDesc").toString();
}


int Engine::currentPBMemSlot()
{
    return d->i_currentPBMemSlot;
}

void Engine::setCurrentPBMemSlot(int type)
{
    d->i_currentPBMemSlot=type;
}

void Engine::slotStopDevice()
{
    d->weaver->suspend();
}

void Engine::slotResumeDevice()
{
    d->weaver->resume();
    emit resumed();
}

void Engine::slotWeaverSuspended()
{
    emit suspended();
}

void Engine::suspendStatusJobs(bool suspend)
{
    if(suspend) d->i_suspendStatusJobs++; else d->i_suspendStatusJobs--;
}

int Engine::statusJobsSuspended() const
{
    return d->i_suspendStatusJobs;
}

Engine *Engine::load(const QString &libname, QObject *parent)
{
    kDebug() << "Engine::load(" << libname << ")" << endl;
    KLibFactory *factory=KLibLoader::self()->factory(qPrintable(libname));
    if(!factory) {
        kDebug() << "Error loading library: " << KLibLoader::self()->lastErrorMessage() << endl;
        return NULL;
    }
    Engine *ret=static_cast<KMobileTools::Engine *>(factory->create(parent, "KMobileTools::Engine" ) );
    if(parent) ret->setObjectName(parent->objectName());
    return ret;
}


#include "engine.moc"
