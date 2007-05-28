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
#include "smslistviewitem.h"
#include <libkmobiletools/contactslist.h>
#include <qfont.h>
#include <q3header.h>
#include <qpainter.h>
#include <qstring.h>
#include <qtimer.h>
#include <kstringhandler.h>

#include <kdebug.h>
#define COLUMN_DATE 2

SMSListViewItem::SMSListViewItem(K3ListView *parent, SMS *sms, KMobileTools::ContactsList *phoneBook, const char *name)
    : QObject(parent), K3ListViewItem(parent, name)
{
    p_sms=sms;
    p_phoneBook=phoneBook;
    if(! sms->getFrom().isNull()) setText(0, KMobileTools::KMobiletoolsHelper::translateNumber(sms->getFrom() ) );
    if(!sms->getTo().isEmpty()) setText(1, KMobileTools::KMobiletoolsHelper::translateNumber(sms->getTo  ().join(",") ) );
    setText(2, sms->getDateTime().toString(Qt::LocalDate) );
    setText(3, sms->getText().replace( '\n', ' ').replace( '\r', ' ').trimmed() );
}

#include "smslistviewitem.moc"

SMSListViewItem::~SMSListViewItem()
{
}

int SMSListViewItem::compare( Q3ListViewItem *i, int col, bool asc ) const
{
    if(col!=COLUMN_DATE) return K3ListViewItem::compare(i, col, asc);
    SMSListViewItem *p_i= (SMSListViewItem*) i;
    if( p_i->sms()->getDateTime() == p_sms->getDateTime() ) return 0;
    if( p_i->sms()->getDateTime() > p_sms->getDateTime() ) return -1; else return +1;
}


// This method is partially derived from akregator - articlelistview.cpp, See copyright statement below
//    Copyright (C) 2007 Frank Osterfeld <frank.osterfeld at kdemail.net>


void SMSListViewItem::paintCell( QPainter * p, const QColorGroup & cg,
                                 int column, int width, int align )

{
    if (! ((p_sms->type() & SMS::Unread) || (p_sms->type() & SMS::Unsent)) )
    {
        K3ListViewItem::paintCell(p,cg,column,width,align);
        return;
    }
    QColorGroup cg2(cg);
    if(p_sms->type() & SMS::Unsent) cg2.setColor(QColorGroup::Text, Qt::red);
    else cg2.setColor(QColorGroup::Text, Qt::blue);
    K3ListViewItem::paintCell( p, cg2, column, width, align );
}

void SMSListViewItem::selected()
{
    QTimer::singleShot( 6000, this, SLOT(markRead() ) );
}

void SMSListViewItem::markRead()
{
    if(!isSelected()) return;
    if(p_sms->type()==SMS::Unread ) p_sms->setType( SMS::Read );
}

