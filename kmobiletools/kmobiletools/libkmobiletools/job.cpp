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
#include "job.h"
#include <threadweaver/DependencyPolicy.h>
#include "klocalizedstring.h"
#include <QVariant>

class KMobileTools::JobPrivate {
public:
    JobPrivate() : ji_percent(0)
    {}
    quint8 ji_percent;
};

KMobileTools::Job::Job (const QString &owner, QObject* parent)
    : ThreadWeaver::Job (parent), d(new JobPrivate)
{
    setProperty("owner", owner);
    connect(this, SIGNAL(SPR()), this, SLOT(slotPercentDone()));
}

KMobileTools::Job::Job (Job *pjob, QObject* parent)
    : ThreadWeaver::Job(parent), d(new JobPrivate)
{
    if(pjob) ThreadWeaver::DependencyPolicy::instance().addDependency(this, pjob);
    connect(this, SIGNAL(SPR()), this, SLOT(slotPercentDone()));
}

KMobileTools::Job::~Job()
{
    delete d;
}

void KMobileTools::Job::setPercentDone(quint8 percentDone) { d->ji_percent=percentDone;}

quint8 KMobileTools::Job::percentDone() { return d->ji_percent; }


void KMobileTools::Job::slotPercentDone()
{
//     kDebug() << "*****slotPercentDone with percentDone==" << ji_percent << endl;
    if(!percentDone() ) return;
    emit percentDone( percentDone() );
    setPercentDone(0);
}


void KMobileTools::Job::slotPercentDone(quint8 p)
{
//     kDebug() << "|||||Calling triggerSPR with percentDone==" << p << endl;
    setPercentDone(p);
//     triggerSPR(); @TODO look for new api
}


const QString KMobileTools::Job::typeString()
{
    switch( type() ){
        case KMobileTools::Job::initPhone:
            return i18n("Connecting to the Phone");
        case KMobileTools::Job::pollStatus:
            return i18n("Fetching Phone Status");
        case KMobileTools::Job::fetchSMS:
            return i18n("Fetching SMS");
        case KMobileTools::Job::fetchAddressBook:
            return i18n("Fetching PhoneBook");
        case KMobileTools::Job::fetchPhoneInfos:
            return i18n("Fetching Phone Informations");
        case KMobileTools::Job::testPhoneFeatures:
            return i18n("Testing Phone Capabilities");
        case KMobileTools::Job::syncDateTimeJob:
            return i18n("Syncing Date and Time");
        case KMobileTools::Job::selectSMSSlot:
            return i18n("Changing SMS Memory Slot");
        case KMobileTools::Job::selectCharacterSet:
            return i18n("Changing Character Set");
        case KMobileTools::Job::sendSMS:
            return i18n("Sending SMS");
        case KMobileTools::Job::storeSMS:
            return i18n("Storing SMS");
        case KMobileTools::Job::addAddressee:
            return i18n("Adding Contacts");
        case KMobileTools::Job::delAddressee:
            return i18n("Deleting Contacts");
        case KMobileTools::Job::editAddressee:
            return i18n("Modifying Contacts");
        case KMobileTools::Job::smsFolders:
            return i18n("Getting SMS Folders");
        case KMobileTools::Job::delSMS:
            return i18n("Deleting SMS");
        case KMobileTools::Job::fetchKCal:
            return i18n("Fetching Calendar Events");
        case KMobileTools::Job::sendStoredSMS:
            return i18n("Sending SMS from Storage");
        default:
            return i18n("Unknown Job");
    }
}

#include "job.moc"
