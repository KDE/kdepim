/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/


#include "desktop.h"

#include <qregexp.h>


using namespace OpieHelper;

Desktop::Desktop( CategoryEdit* edit )
    : Base( edit )
{}

Desktop::~Desktop()
{}


KSync::OpieDesktopSyncee* Desktop::toSyncee( const QString& str)
{
    KSync::OpieDesktopSyncee* syncee;
    syncee = new KSync::OpieDesktopSyncee();

    // convert the string
    QString string ( str );
    string.remove(0, 35 );
    string.replace(QRegExp("&amp;"), "&" );
    string.replace(QRegExp("&0x20;"), " ");
    string.replace(QRegExp("&0x0d;"), "\n");
    string.replace(QRegExp("&0x0a;"), "\r");
    string.replace(QRegExp("\r\n"), "\n" ); // hell we're on unix

    if ( !str.contains("[Desktop Entry]") ) {
        delete syncee;
        return 0l;
    }
    QStringList list = QStringList::split('\n', string );
    QStringList::Iterator it;
    it = list.begin();
    list.remove( it ); // remove the [Desktop Entry]

    KSync::OpieDesktopSyncEntry *entry;
    QString name,  type,  fileName,  size;
    QStringList category;

    for ( it = list.begin(); it != list.end(); ++it ) { // QAsciiDict?
        QString con( (*it).stripWhiteSpace() );

        if (con.startsWith("Categories = ") ) {
            con = con.remove(0, 13 );
            //con = con.remove( con.length() -1, 1 ); // remove the last char ;
            QStringList catList = QStringList::split(';',con ); // no empty cats
            category = edit()->categoriesByIds( catList, QString::null );

        }else if ( con.startsWith("Name = ") ) {
            con = con.remove(0, 7 );
            name = con.stripWhiteSpace();
        }else if ( con.startsWith("Type = ") ) {
            con = con.remove( 0, 7 );
            type = con.stripWhiteSpace() ;
        }else if ( con.startsWith("File = ") ) {
            con = con.remove( 0, 7 );
            fileName = con.stripWhiteSpace();
        }else if ( con.startsWith("Size = ") ) {
            con = con.remove(0, 7 );
            size = con.stripWhiteSpace();
        }else if ( (*it).stripWhiteSpace() == "[Desktop Entry]" ) {
            entry= new KSync::OpieDesktopSyncEntry( category,
                                                    fileName,
                                                    name,
                                                    type,
                                                    size, syncee );
            syncee->addEntry( entry );
        }

    }
    // one is missing
    entry = new KSync::OpieDesktopSyncEntry( category,
                                             fileName,
                                             name,
                                             type,
                                             size, syncee );
    syncee->addEntry( entry );
    return syncee;
}

