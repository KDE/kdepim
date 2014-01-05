/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef NOTELISTWIDGET_H
#define NOTELISTWIDGET_H

#include "noteshared_export.h"
#include <QListWidget>
#include <Akonadi/Item>

namespace NoteShared {
class NOTESHARED_EXPORT NoteListWidget : public QListWidget
{
public:
    explicit NoteListWidget(QWidget *parent=0);
    ~NoteListWidget();

    void setNotes(const Akonadi::Item::List &notes);
    void addNotes(const Akonadi::Item::List &notes);
    void removeNote(const Akonadi::Item &note);
    QStringList selectedNotes() const;

    Akonadi::Item::Id currentItemId() const;

private:
    enum listViewData {
        AkonadiId = Qt::UserRole + 1
    };
    Akonadi::Item::List mNotes;
    void createItem(const Akonadi::Item &note);
};
}

#endif // NOTELISTWIDGET_H
