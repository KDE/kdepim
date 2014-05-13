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


#ifndef EVENTEDIT_H
#define EVENTEDIT_H

#include <QWidget>
#include "messageviewer_export.h"

#include <AkonadiCore/Collection>
#include <KMime/Message>
#include <KCalCore/Event>

class QLineEdit;
class KDateTimeEdit;
class KPushButton;
namespace Akonadi {
class CollectionComboBox;
}

namespace MessageViewer {
class MESSAGEVIEWER_EXPORT EventEdit : public QWidget
{
    Q_OBJECT
public:
    explicit EventEdit(QWidget *parent = 0);
    ~EventEdit();

    Akonadi::Collection collection() const;
    void setCollection(const Akonadi::Collection &value);

    KMime::Message::Ptr message() const;
    void setMessage(const KMime::Message::Ptr &value);

    void writeConfig();
    void showEventEdit();

public Q_SLOTS:
    void slotCloseWidget();

private Q_SLOTS:
    void slotReturnPressed();
    void slotCollectionChanged(int);
    void slotOpenEditor();
    void slotStartDateTimeChanged(const KDateTime &newDateTime);
    void slotUpdateButtons(const QString &subject);

Q_SIGNALS:
    void createEvent(const KCalCore::Event::Ptr &event, const Akonadi::Collection &collection);
    void collectionChanged(const Akonadi::Collection &col);
    void messageChanged(const KMime::Message::Ptr &msg);

protected:
    bool eventFilter(QObject *object, QEvent *e);
private:
    void readConfig();
    Akonadi::Collection mCollection;
    KMime::Message::Ptr mMessage;
    QLineEdit *mEventEdit;
    Akonadi::CollectionComboBox *mCollectionCombobox;
    KDateTimeEdit *mStartDateTimeEdit;
    KDateTimeEdit *mEndDateTimeEdit;
    KPushButton *mSaveButton;
    KPushButton *mOpenEditorButton;
};
}

#endif // EVENTEDIT_H
