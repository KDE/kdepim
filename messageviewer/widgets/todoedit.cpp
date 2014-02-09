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

#include "todoedit.h"

#include <KLocalizedString>
#include <QHBoxLayout>
#include <QLineEdit>
#include <Akonadi/CollectionComboBox>

namespace MessageViewer {
MESSAGEVIEWER_EXPORT QAbstractItemModel *_k_todoEditStubModel = 0;
}

using namespace MessageViewer;

TodoEdit::TodoEdit(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    setLayout(hbox);
    mNoteEdit = new QLineEdit;
    mNoteEdit->setObjectName(QLatin1String("noteedit"));
    connect(mNoteEdit, SIGNAL(returnPressed()), SLOT(slotReturnPressed()));
    hbox->addWidget(mNoteEdit);
    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_todoEditStubModel);
    mCollectionCombobox->setObjectName(QLatin1String("akonadicombobox"));
    hbox->addWidget(mCollectionCombobox);
}

Akonadi::Collection TodoEdit::collection() const
{
    return mCollection;
}

void TodoEdit::setCollection(const Akonadi::Collection &value)
{
    if (mCollection != value) {
        mCollection = value;
        Q_EMIT collectionChanged(mCollection);
    }
}
KMime::Message::Ptr TodoEdit::message() const
{
    return mMessage;
}

void TodoEdit::setMessage(const KMime::Message::Ptr &value)
{
    if (mMessage != value) {
        mMessage = value;
        const KMime::Headers::Subject * const subject = mMessage ? mMessage->subject(false) : 0;
        if (subject) {
            mNoteEdit->setText(i18n("Reply to \"%1\"", subject->asUnicodeString()));
            mNoteEdit->selectAll();
        } else {
            mNoteEdit->clear();
        }
        Q_EMIT messageChanged(mMessage);
    }
}

void TodoEdit::slotReturnPressed()
{
    if (!mNoteEdit->text().isEmpty()) {
        KCalCore::Todo::Ptr todo( new KCalCore::Todo );
        todo->setSummary(mNoteEdit->text());
        Q_EMIT createTodo(todo);
    }
}


