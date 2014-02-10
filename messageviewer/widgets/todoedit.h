/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef TODOEDIT_H
#define TODOEDIT_H

#include <QWidget>
#include <Akonadi/Collection>
#include <KMime/Message>
#include <KCalCore/Todo>
#include "messageviewer_export.h"

class KLineEdit;

namespace Akonadi {
class CollectionComboBox;
}

namespace MessageViewer {
class MESSAGEVIEWER_EXPORT TodoEdit : public QWidget
{
    Q_OBJECT
public:
    explicit TodoEdit(QWidget *parent = 0);

    Akonadi::Collection collection() const;
    void setCollection(const Akonadi::Collection &value);

    KMime::Message::Ptr message() const;
    void setMessage(const KMime::Message::Ptr &value);

public Q_SLOTS:
    void slotCloseWidget();

private Q_SLOTS:
    void slotReturnPressed();
    void slotCollectionChanged(int);
Q_SIGNALS:
    void createTodo(const KCalCore::Todo::Ptr &);
    void collectionChanged(const Akonadi::Collection &col);
    void messageChanged(const KMime::Message::Ptr &msg);

private:
    Akonadi::Collection mCollection;
    KMime::Message::Ptr mMessage;
    KLineEdit *mNoteEdit;
    Akonadi::CollectionComboBox *mCollectionCombobox;
};
}
#endif // TODOEDIT_H
