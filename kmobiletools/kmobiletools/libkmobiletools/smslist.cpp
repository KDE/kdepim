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
#include "smslist.h"
#include <kdebug.h>
#include <qdir.h>
#include <Q3PtrList>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include "kmobiletools_cfg.h"
#include "devicesconfig.h"

#include <QListIterator>

using namespace KMobileTools;

class SMSListPrivate {
public:
    SMSListPrivate() : i_unread_phone(0), i_unread_sim(0), i_read_phone(0), i_read_sim(0),
                   i_sent_phone(0), i_sent_sim(0), i_unsent_phone(0), i_unsent_sim(0) {}
    int i_unread_phone, i_unread_sim;
    int i_read_phone, i_read_sim;
    int i_sent_phone, i_sent_sim;
    int i_unsent_phone, i_unsent_sim;
    QString s_enginename;
};


SMSList::SMSList(const QString &enginename) : QObject(), QList<SMS*>(), d(new SMSListPrivate)
{
    resetCount();
    if(! enginename.isNull() ) setEngineName(enginename);
}


SMSList::~SMSList()
{
}

void SMSList::append( SMSList *sublist, bool sync)
{
    if(!sublist || !sublist->size()) return;
    QList<SMS*>::iterator it;
    for(it=sublist->begin(); it!=sublist->end(); ++it)
    {
        if(sync) { if( indexOf(*it)==-1) append(*it); }
        else append(*it);
        ++it;
    }
}


/*!
    \fn SMSList::find(int uid)
 */
int SMSList::find(const QString &uid) const
{
    int found = 0;

    QListIterator<SMS*> it( *this );
    while( it.hasNext() ) {
        if( it.next()->uid() == uid )
            return found;
        found++;
    }

    return -1;
}


/*!
    \fn SMSList::sync (SMSList *compList)
 */
void SMSList::sync (SMSList *compList)
{
    SMS *tempSMS;
    // Parsing the current SMSList. If some items are not found in the new one, they'll be removed
    QListIterator<SMS*> it (*this);
    while( (!this->isEmpty()) && it.hasNext() )
    {
        tempSMS=it.next();
        if( compList->find( tempSMS->uid() ) == -1 )
        {
            emit removed(tempSMS->uid() );
            removeAll(tempSMS);
//             delete tempSMS;
        }
    }
        // Now parsing the new SMSList to find new items to add
    QListIterator<SMS*> it2 (*compList);
    while( it2.hasNext() )
    {
        tempSMS=it2.next();
        if( this->find( tempSMS->uid() ) == -1 )
        {
            append(tempSMS);
            emit added(tempSMS->uid() );
        }
    }
}

/// @TODO look if this is to be ported
// int SMSList::compareItems( Q3PtrCollection::Item item1, Q3PtrCollection::Item item2 )
// {
//     return ( ((SMS*) item1)->uid()== ((SMS*) item2)->uid() );
// }

void SMSList::dump() const
{
    SMS *tempSMS;
    int i=0;
    // Parsing the current SMSList. If some items are not found in the new one, they'll be removed
    QListIterator<SMS*> it (*this);
    while( it.hasNext() && !this->isEmpty()  )
    {
        tempSMS=it.next();
        i++;
        kDebug() << "SMSList::dump(): " << QString("%1").arg(i,2) << "|" << tempSMS->uid() << "|" << tempSMS->type() << "|" << tempSMS->getText() << endl;
    }
//     kDebug() << "SMSList::dump(): Unread=" << i_unread << "; Read=" << i_read << "; Sent=" << i_sent << "; Unsent=" << i_unsent << endl;
}

void SMSList::calcSMSNumber() const
{
    resetCount();
    SMS *tempSMS;
    QListIterator<SMS*> it2 (*this);
    while( it2.hasNext() )
    {
        tempSMS=it2.next();
//         kDebug() << QString("SMS Type: %1").arg(tempSMS->type() ,8,2) << endl;
        switch (tempSMS->type())
        {
            case SMS::Unread:
                if(tempSMS->slot() & SMS::SIM) d->i_unread_sim++;
                if(tempSMS->slot() & SMS::Phone) d->i_unread_phone++;
                break;
            case SMS::Read:
                if(tempSMS->slot() & SMS::SIM) d->i_read_sim++;
                if(tempSMS->slot() & SMS::Phone) d->i_read_phone++;
                break;
            case SMS::Unsent:
                if(tempSMS->slot() & SMS::SIM) d->i_unsent_sim++;
                if(tempSMS->slot() & SMS::Phone) d->i_unsent_phone++;
                break;
            case SMS::Sent:
                if(tempSMS->slot() & SMS::SIM) d->i_sent_sim++;
                if(tempSMS->slot() & SMS::Phone) d->i_sent_phone++;
                break;
            case SMS::All:
                //we shouldn't be here...
                break;
        }
    }
}

int SMSList::count(int smsType, int memSlot) const
{
    int result=0;
    if( (smsType & SMS::Unread) )
    {
        if( memSlot & SMS::SIM ) result+= d->i_unread_sim;
        if( memSlot & SMS::Phone) result+=d->i_unread_phone;
    }
    if( (smsType & SMS::Read) )
    {
        if( memSlot & SMS::SIM ) result+= d->i_read_sim;
        if( memSlot & SMS::Phone) result+=d->i_read_phone;
    }
    if( (smsType & SMS::Unsent) )
    {
        if( memSlot & SMS::SIM ) result+= d->i_unsent_sim;
        if( memSlot & SMS::Phone) result+=d->i_unsent_phone;
    }
    if( (smsType & SMS::Sent) )
    {
        if( memSlot & SMS::SIM ) result+= d->i_sent_sim;
        if( memSlot & SMS::Phone) result+=d->i_sent_phone;
    }
    return result;
}

#include "smslist.moc"


/*!
    \fn SMSList::saveToMailBox(const QString &engineName)
 */
void SMSList::saveToMailBox(const QString &engineName)
{
    setEngineName(engineName);
    saveToMailBox();
}

/*!
    \fn SMSList::saveToMailBox()
 */
void SMSList::saveToMailBox() const
{
    QDir savedir=(KMobileTools::DevicesConfig::prefs(engineName() ))->maildir_path();
    QString dir=savedir.dirName();
    savedir.cdUp();
    dir=savedir.absolutePath() + QDir::separator() + '.' + dir + ".directory"
            + QDir::separator() + '.' + KMobileTools::DevicesConfig::prefs(d->s_enginename)->devicename() + ".directory";
    QListIterator<SMS*> it(*this);
    while( it.hasNext() )
    {
        it.next()->exportMD(dir);
    }
}

/*!
    \fn SMSList::saveToCSV(const QString &engineName)
 */
int SMSList::saveToCSV(const QString &filename) const
{
    kDebug() << k_funcinfo << endl;
    SMS *sms;
    kDebug() << "SMSList::saveToCSV(): saving CSV file to: " << filename << endl;
    bool ok=true;
/*    QListIterator<SMS*> it(*this);
    while( (it.hasNext()) )
    {
        sms=it.next();
        ok&=sms->writeToSlotCSV(filename);
    }*/
    for(int i=0; i<size(); i++) {
        sms=at(i);
        ok&=sms->writeToSlotCSV(filename);
    }
    return ok;
}

/*!
    \fn SMSList::saveToCSV()
 */

/// @TODO Check if we can remove dialog windows out of this class, emitting insteada signal.
int SMSList::saveToCSV() const
{
    QString saveFile;

    saveFile = KFileDialog::getSaveFileName (QDir::homePath(), "*.csv",  0, i18n("Save file to disk"));
    if(saveFile.isEmpty() ) return -1;

    if ( QFile::exists(saveFile)) {
        kDebug() << "SMSList::saveToCSV(): File already exists " << endl;

        int retval;
        retval=KMessageBox::warningContinueCancel(NULL,
                                                  i18n("<qt>The file already exists\nOverwrite the current file?</qt>"),
                                                  "KMobileTools" );

        if(retval == KMessageBox::Continue) {
            QFile::remove(saveFile);
        }
        else {
            return -1;
        }

    }
    return saveToCSV(saveFile);
}

void SMSList::append( SMS *item )
{
    QList<SMS*>::append(item);
//     connect(item, SIGNAL(updated()), this, SIGNAL(updated()) );
}

void SMSList::resetCount() const {
    d->i_unread_phone=d->i_unread_sim=d->i_read_phone=d->i_read_sim=d->i_unsent_phone=d->i_unsent_sim=d->i_sent_phone=d->i_sent_sim=0;
}

void SMSList::setEngineName(const QString &enginename) { d->s_enginename=enginename; }
QString SMSList::engineName() const { return d->s_enginename; }
