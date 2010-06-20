/*
  This file is part of libkcal.

  Copyright (c) 2008 Kevin Ottens <ervin@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/
/**
  @file
  This file is part of the API for handling calendar data and provides a
  dialog for asking the user to select from a list of available calendars.

  @author Kevin Ottens \<ervin@kde.org\>
  @author Allen Winter \<allen@kdab.net\>
*/

#ifndef CALSELECTDIALOG_H
#define CALSELECTDIALOG_H

#include <kdialogbase.h>

namespace KCal {

class CalSelectDialog : public KDialogBase
{
  private:
    CalSelectDialog( const QString &caption, const QString &label,
                     const QStringList &list );

  public:
    static QString getItem( const QString &caption, const QString &label,
                            const QStringList &list );

  protected:
    virtual void closeEvent( QCloseEvent *event );
    void reject();

  private:
    KListBox *mListBox;
};

}

#endif
