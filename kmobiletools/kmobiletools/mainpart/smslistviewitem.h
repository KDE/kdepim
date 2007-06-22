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
#ifndef SMSLISTVIEWITEM_H
#define SMSLISTVIEWITEM_H

#include <k3listview.h>
#include <libkmobiletools/sms.h>
#include <libkmobiletools/kmobiletoolshelper.h>

/**
	@author Marco Gulino <marco@kmobiletools.org>
*/


class SMSFolderListViewItem : public K3ListViewItem
{
    public:
        SMSFolderListViewItem(Q3ListView *parent, int memslot, int smsType, QString text)
    : K3ListViewItem (parent,text)
        {
            setMemSlot(memslot);
            setSMSType(smsType);
        }
        SMSFolderListViewItem(Q3ListViewItem *parent, int memslot, int smsType, QString text)
    : K3ListViewItem (parent,text)
        {
            setMemSlot(memslot);
            setSMSType(smsType);
        }
        void setMemSlot(int memslot) { i_memslot=memslot;}
        void setSMSType(int smstype) { i_smsType=smstype;}
        int memSlot() { return i_memslot;}
        int smsType() { return i_smsType; }
        bool isIncoming() { return bool((i_smsType & KMobileTools::SMS::Unread) || (i_smsType & KMobileTools::SMS::Read) ); }
    private:
        int i_memslot;
        int i_smsType;
    protected:
        virtual void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align );
};


class SMSListViewItem : public QObject, public K3ListViewItem
{
    Q_OBJECT
public:
    SMSListViewItem(K3ListView *parent, KMobileTools::SMS *sms, KMobileTools::ContactsList *phoneBook, const char *name = 0);

    ~SMSListViewItem();
    KMobileTools::SMS *sms() { return p_sms;}
    void setSMS(KMobileTools::SMS *sms) {p_sms=sms;}
    virtual int compare( Q3ListViewItem *i, int col, bool ) const;
    private:
        KMobileTools::SMS *p_sms;
        KMobileTools::ContactsList *p_phoneBook;
    protected:
        virtual void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align );
    public slots:
        void selected();
        void markRead();
};

#endif
