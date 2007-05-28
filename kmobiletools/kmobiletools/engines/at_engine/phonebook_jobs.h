/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>,
   Alexander Rensmann <zerraxys@gmx.net>

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
#ifndef PHONEBOOKJOBSS_H
#define PHONEBOOKJOBSS_H

#include <libkmobiletools/contactslist.h>

#include "at_jobs.h"
//Added by qt3to4:
#include <Q3ValueList>

class FetchAddressee : public kmobiletoolsATJob
{
Q_OBJECT
    public:
      FetchAddressee(KMobileTools::Job *pjob, int memslots, KMobileTools::SerialManager *device, AT_Engine* parent = 0 );
      JobType type()            { return KMobileTools::Job::fetchAddressBook; }
        const KMobileTools::ContactsList& fullAddresseeList() { return p_fullAddresseeList; }
        bool partialUpdates() { return b_partialUpdates; }
    protected:
        void run ();
        void fetchMemSlot(int, bool updatePercent=true);
        int i_slots, i_curslot;
        bool b_partialUpdates;
        KMobileTools::ContactsList addresseeList;
        KMobileTools::ContactsList p_fullAddresseeList;
        int total, jdone;
    protected slots:
        void execSPR();
    signals:
        void gotAddresseeList(int,const KMobileTools::ContactsList&);
};

/*
 * class FetchAddresseeSiemens
 * Description:
 * Fetches the Siemens phone book.
 */

class FetchAddresseeSiemens : public FetchAddressee
{
    public:
      FetchAddresseeSiemens(KMobileTools::Job *pjob, KMobileTools::SerialManager *device, AT_Engine* parent = 0 );
  protected:
        void run ();
        void fetchVCF();
        void fetchSDBR();
        AT_Engine *engine;
};

class EditAddressees :  public kmobiletoolsATJob
{
    Q_OBJECT
    public:
      EditAddressees( KMobileTools::Job *pjob, const QList<KABC::Addressee>& abclist, KMobileTools::SerialManager *device, bool b_todelete=false, AT_Engine *parent=0);
      EditAddressees(  KMobileTools::Job *pjob, const KABC::Addressee& oldAddressee, const KABC::Addressee& newAddressee, KMobileTools::SerialManager *device, AT_Engine *parent=0);

        JobType type() {
            if( !p_oldAddressee.isEmpty() && !p_newAddressee.isEmpty() )
                return KMobileTools::Job::editAddressee;
            if( todelete )
                return KMobileTools::Job::delAddressee;
            return KMobileTools::Job::addAddressee;
        }
        bool pbIsFull() { return pb_full; }
    protected:
        void run();
        int addAddressee ( const KABC::Addressee& addressee, int start=0);
        void delAddressee (const KABC::Addressee& addressee);
        int findFreeIndex(int startpoint=0);
    protected slots:
        void updateProgress(int);
    private:
        QList<KABC::Addressee> p_abclist;
        KABC::Addressee p_oldAddressee;
        KABC::Addressee p_newAddressee;
        bool todelete;
        int totaljobs, totalprogress;
        bool pb_full;

    signals:
        void partialProgress(int);
};


#endif
