/*-
 * Copyright 2009 KDAB and Guillermo A. Amaral B. gamaral@amaral.com.mx
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include <QDebug>

#include <StickyNotes/MemoryNoteItem>
#include <StickyNotes/CacheNoteItem>

using namespace StickyNotes;

int
main(int argc, char *argv[])
{
    MemoryNoteItem noteItem1("Test", "Content goes here.");
    noteItem1.setAttribute("readonly", true);
    BaseNoteItem   *noteItem2 = new BaseNoteItem(&noteItem1);
    CacheNoteItem  *noteItem3 = new CacheNoteItem(&noteItem1);
    BaseNoteItem   *noteItem4 = new BaseNoteItem(noteItem3);

    qDebug() << "1   MemoryNoteItem: " << noteItem1.subject() << ": " << noteItem1.content();
    qDebug() << "1-2   BaseNoteItem: " << noteItem2->subject() << ": " << noteItem2->content();
    qDebug() << "1-3  CacheNoteItem: " << noteItem3->subject() << ": " << noteItem3->content();
    qDebug() << "1-3-4 BaseNoteItem: " << noteItem4->subject() << ": " << noteItem4->content();

    qDebug() << "Changed to 1-3-4 BaseNoteItem";
    noteItem4->setSubject("Modified");

    qDebug() << "1   MemoryNoteItem: " << noteItem1.subject() << ": " << noteItem1.content();
    qDebug() << "1-2   BaseNoteItem: " << noteItem2->subject() << ": " << noteItem2->content();
    qDebug() << "1-3  CacheNoteItem: " << noteItem3->subject() << ": " << noteItem3->content();
    qDebug() << "1-3-4 BaseNoteItem: " << noteItem4->subject() << ": " << noteItem4->content();

    qDebug() << "1-3-4 BaseNoteItem RO: " << noteItem4->attribute("readonly");

    return (0);
}

