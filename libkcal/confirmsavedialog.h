/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
*/
#ifndef KCAL_CONFIRMSAVEDIALOG_H
#define KCAL_CONFIRMSAVEDIALOG_H

#include <QTreeWidget>

#include <libkcal/incidence.h>

#include <kdialogbase.h>
#include <kdepimmacros.h>

namespace KCal {

class KDE_EXPORT ConfirmSaveDialog : public KDialogBase
{
  public:
    ConfirmSaveDialog( const QString &destination, QWidget *parent,
                       const char *name = 0 );

    void addIncidences( const Incidence::List &incidences,
                        const QString &operation );

  private:
    QTreeWidget *mListView;
};

}

#endif
