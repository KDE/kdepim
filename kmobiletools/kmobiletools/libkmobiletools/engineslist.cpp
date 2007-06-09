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
#include "engineslist.h"
#include "devicesconfig.h"

#include "weaver.h"
#include "engine.h"
#include <kservicetypetrader.h>
#include <kplugininfo.h>
#include <kconfigskeleton.h>

using namespace KMobileTools;

class EnginesListPrivate {
    public:
        EnginesListPrivate(): b_closing(false), wizardEngine(NULL), wizardcfg(NULL)
        {}
        QStringList s_locklist;
        bool b_closing;
        KMobileTools::Engine *wizardEngine;
        KConfigSkeleton *wizardcfg;
};

K_GLOBAL_STATIC(KMobileTools::EnginesList, engineslist_instance)


KMobileTools::EnginesList* KMobileTools::EnginesList::instance()
{
    return engineslist_instance;
}

KMobileTools::EnginesList::EnginesList()
    : QObject(), QList<KMobileTools::Engine*>()
{
    d=new EnginesListPrivate;
}


KMobileTools::EnginesList::~EnginesList()
{
    kDebug() << "KMobileTools::EnginesList::~EnginesList()\n";
    delete d;
}

#include "engineslist.moc"

void KMobileTools::EnginesList::queryClose()
{
    d->b_closing=true;
    KMobileTools::Engine *tempDevice;
    QStringList out;
    QList<KMobileTools::Engine*>::ConstIterator it=begin(), itEnd=end();
    for( ; it!=itEnd; ++it)
    {
        tempDevice = *it;
        if(tempDevice)
        {
//             tempDevice->disconnect(tempDevice, 0, 0, 0);
            tempDevice->queryClose();
        }
    }
    KMobileTools::Weaver::instance()->requestAbort();
}

void KMobileTools::EnginesList::append( KMobileTools::Engine* item )
{
    emit engineAdded( item );
    connect(item, SIGNAL(phoneBookChanged()), this, SIGNAL(phonebookUpdated() ));
    QList<KMobileTools::Engine*>::append(item);
}

void KMobileTools::EnginesList::remove( KMobileTools::Engine* item )
{
    emit engineRemoved( item );
    disconnect(item, SIGNAL(phoneBookChanged()), this, SIGNAL(phonebookUpdated() ));
    QList<KMobileTools::Engine*>::removeAll(item);
}
/*
void KMobileTools::EnginesList::emitPhonebookUpdated()
{
    kDebug() << "KMobileTools::EnginesList::emitPhonebookUpdated()\n";
    emit phonebookUpdated();
}*/

void KMobileTools::EnginesList::dump()
{
    kDebug() << "KMobileTools::EnginesList::dump()\n";
}

const QStringList KMobileTools::EnginesList::namesList(bool friendlyNames)
{
    KMobileTools::Engine *tempDevice;
    QStringList out;
    QList<KMobileTools::Engine*>::ConstIterator it=begin(), itEnd=end();
    for( ; it!=itEnd; ++it)
    {
        tempDevice = *it;
        if(friendlyNames) out+= DEVCFG(tempDevice->objectName() )->devicename();
        else out+=tempDevice->objectName();
    }
    return out;
}

KMobileTools::Engine *KMobileTools::EnginesList::find( const QString &name, bool friendlyName)
{
    KMobileTools::Engine *tempDevice;
    QList<KMobileTools::Engine*>::ConstIterator it=begin(), itEnd=end();
    for( ; it!=itEnd; ++it)
    {
        tempDevice = *it;
        if(friendlyName)
        {
            if ( DEVCFG(tempDevice->objectName() )->devicename() == name ) return tempDevice;
        }
        else
            if( tempDevice->objectName() == name ) return tempDevice;
    }
    return 0;
}

KPluginInfo::List KMobileTools::EnginesList::availEngines()
{
    KServiceTypeTrader* trader=KServiceTypeTrader::self();
    return KPluginInfo::fromServices(trader->query("KMobileTools/Engine") );
}

KPluginInfo *KMobileTools::EnginesList::engineInfo(const QString &s, bool searchByLibrary)
{
    QList<KPluginInfo *> l_engines=availEngines();
    for(QList<KPluginInfo *>::const_iterator it=l_engines.begin(); it!=l_engines.end(); it++)
        if( ( (!searchByLibrary) && s==(*it)->name() ) ||
                (searchByLibrary && s==(*it)->service()->library() ) ) return (*it);
    return NULL;
}

bool KMobileTools::EnginesList::closing() const
{
    return d->b_closing;
}


bool KMobileTools::EnginesList::locked(const QString &device) const
{
    return (bool) d->s_locklist.contains(device);
}

const QStringList KMobileTools::EnginesList::locklist()
{
    return d->s_locklist;
}
bool KMobileTools::EnginesList::lock(const QString &device)
{
    if(locked(device)) return false;
    d->s_locklist+=device;
    return true;
}

void KMobileTools::EnginesList::unlock(const QString &device)
{
    d->s_locklist.removeAll(device);
}

KMobileTools::Engine* KMobileTools::EnginesList::wizardEngine()
{
    return d->wizardEngine;
}

void KMobileTools::EnginesList::setWizardEngine(KMobileTools::Engine* newEngine)
{
    if(d->wizardEngine)
    {
        d->wizardEngine->queryClose();
        delete d->wizardEngine;
    }
    d->wizardEngine=newEngine;
}

KConfigSkeleton *KMobileTools::EnginesList::wizardConfig()
{
    return d->wizardcfg;
}

void KMobileTools::EnginesList::setWizardConfig(KConfigSkeleton *wizardcfg)
{
    d->wizardcfg=wizardcfg;
}

