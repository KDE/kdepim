/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef KCAL_CONFIRMSAVEDIALOG_H
#define KCAL_CONFIRMSAVEDIALOG_H

#include <libkcal/incidence.h>

#include <kdialogbase.h>
#include <kdepimmacros.h>

class KListView;

namespace KCal {

class KDE_EXPORT ConfirmSaveDialog : public KDialogBase
{
  public:
    ConfirmSaveDialog( const QString &destination, QWidget *parent,
                       const char *name = 0 );

    void addIncidences( const Incidence::List &incidences,
                        const QString &operation );

  private:
    KListView *mListView;
};

}

#endif
