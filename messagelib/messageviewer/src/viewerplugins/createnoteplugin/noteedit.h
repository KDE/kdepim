/*
  Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

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

#ifndef NOTEEDIT_H
#define NOTEEDIT_H

#include <QWidget>

#include <AkonadiCore/Collection>
#include <Akonadi/Notes/NoteUtils>
#include <KMime/KMimeMessage>

class QLineEdit;
class QPushButton;
namespace Akonadi
{
class CollectionComboBox;
}

namespace MessageViewer
{
class NoteEdit : public QWidget
{
    Q_OBJECT
public:
    explicit NoteEdit(QWidget *parent = Q_NULLPTR);
    ~NoteEdit();

    Akonadi::Collection collection() const;
    void setCollection(const Akonadi::Collection &value);

    KMime::Message::Ptr message() const;
    void setMessage(const KMime::Message::Ptr &value);

    void writeConfig();
    void showNoteEdit();

public Q_SLOTS:
    void slotCloseWidget();

private Q_SLOTS:
    void slotReturnPressed();
    void slotCollectionChanged(int);
    void slotUpdateButtons(const QString &subject);

Q_SIGNALS:
    void createNote(const KMime::Message::Ptr &note, const Akonadi::Collection &collection);
    void collectionChanged(const Akonadi::Collection &col);
    void messageChanged(const KMime::Message::Ptr &msg);

protected:
    bool eventFilter(QObject *object, QEvent *e) Q_DECL_OVERRIDE;
private:
    void readConfig();
    Akonadi::Collection mCollection;
    KMime::Message::Ptr mMessage;
    QLineEdit *mNoteEdit;
    Akonadi::CollectionComboBox *mCollectionCombobox;
    QPushButton *mSaveButton;
};
}

#endif // NOTEEDIT_H
