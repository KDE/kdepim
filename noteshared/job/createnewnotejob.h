/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef CREATENEWNOTEJOB_H
#define CREATENEWNOTEJOB_H

#include <QWidget>

#include "noteshared_export.h"

namespace NoteShared {
class NOTESHARED_EXPORT CreateNewNoteJob : public QObject
{
    Q_OBJECT
public:
    explicit CreateNewNoteJob(QObject *parent = 0, QWidget *widget=0);
    ~CreateNewNoteJob();

    void start();

private:
    QWidget *mWidget;
};
}

#endif // CREATENEWNOTEJOB_H
