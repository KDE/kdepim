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
#include "enginedata.h"

#include "engine.h"
#include "smslist.h"
#include "contactslist.h"
#include <kabc/addressee.h>

using namespace KMobileTools;

class EngineDataPrivate {
    public:
        EngineDataPrivate() : engine(NULL), i_manufacturer(KMobileTools::Engine::Unknown) {}
        KMobileTools::Engine *engine;
        QString s_manufacturer;         // manufacturer raw string
        int i_manufacturer;             // enum value?
        QString s_model;                // phone model
        QString s_imei;                 // phone imei code
        QString s_smscenter;            // SMS Center number
        QString s_revision;             // Firmware revision
        KCal::Event::List *p_calendar;  // Internal Calendar Events List
        ContactsList* p_addresseeList;  // Phonebook Contacts List
        SMSList *p_smsList;             // List of SMS fetched from the phone
};

EngineData::EngineData(KMobileTools::Engine *parentEngine)
    : QObject(parentEngine), d(new EngineDataPrivate)
{
    d->engine=parentEngine;
    if(d->engine) d->p_smsList=new SMSList(d->engine->objectName() );
    d->p_addresseeList = new ContactsList();
    d->p_calendar=new KCal::Event::List();
}

EngineData::~EngineData()
{
    delete d->p_smsList;
    delete d->p_calendar;
    delete d->p_addresseeList;
    delete d;
}

//KMobileTools::Engine *EngineData::engine() { return d->engine; }

#include "enginedata.moc"

QString EngineData::manufacturer() const { return d->s_manufacturer;}
void EngineData::setManufacturer(const QString &s) { d->s_manufacturer=s;}

void EngineData::setManufacturerID(int i) { d->i_manufacturer=i; }
int EngineData::manufacturerID() const { return d->i_manufacturer; }

void EngineData::setModel(const QString &s) { d->s_model=s;}
QString EngineData::model() const { return d->s_model; }

void EngineData::setIMEI(const QString &s) { d->s_imei=s;}
QString EngineData::imei() const { return d->s_imei; }

void EngineData::setSMSCenter(const QString &s) { d->s_smscenter=s;}
QString EngineData::smsCenter() const { return d->s_smscenter;}

void EngineData::setRevision(const QString &s) { d->s_revision=s;}
QString EngineData::revision() const { return d->s_revision; }

KCal::Event::List *EngineData::calendar() { return d->p_calendar; }

SMSList *EngineData::smsList() const { return d->p_smsList; }

ContactsList *EngineData::contactsList() const { return d->p_addresseeList; }
void EngineData::setContactsList(ContactsList* cl) { d->p_addresseeList=cl; }


