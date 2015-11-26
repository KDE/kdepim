/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "knotefinddialog.h"
#include "widget/notelistwidget.h"

#include <AkonadiSearch/PIM/notequery.h>
#include <AkonadiSearch/PIM/resultiterator.h>

#include <KLocalizedString>
#include <QLineEdit>
#include <QPushButton>
#include <QIcon>

#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <KConfigGroup>

KNoteFindDialog::KNoteFindDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Search Notes"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KNoteFindDialog::reject);
    setAttribute(Qt::WA_DeleteOnClose);
    mNoteFindWidget = new KNoteFindWidget;
    connect(mNoteFindWidget, &KNoteFindWidget::noteSelected, this, &KNoteFindDialog::noteSelected);
    mainLayout->addWidget(mNoteFindWidget);
    mainLayout->addWidget(buttonBox);
    readConfig();
}

KNoteFindDialog::~KNoteFindDialog()
{
    writeConfig();
}

void KNoteFindDialog::setExistingNotes(const QHash<Akonadi::Item::Id, Akonadi::Item> &notes)
{
    mNoteFindWidget->setExistingNotes(notes);
}

void KNoteFindDialog::writeConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "KNoteFindDialog");
    grp.writeEntry("Size", size());
    grp.sync();
}

void KNoteFindDialog::readConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "KNoteFindDialog");
    const QSize size = grp.readEntry("Size", QSize(600, 300));
    if (size.isValid()) {
        resize(size);
    }
}

KNoteFindWidget::KNoteFindWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->addLayout(hbox);
    QLabel *lab = new QLabel(i18n("Search notes:"));
    hbox->addWidget(lab);
    mSearchLineEdit = new QLineEdit;
    mSearchLineEdit->setClearButtonEnabled(true);
    connect(mSearchLineEdit, &QLineEdit::returnPressed, this, &KNoteFindWidget::slotSearchNote);
    connect(mSearchLineEdit, &QLineEdit::textChanged, this, &KNoteFindWidget::slotTextChanged);
    hbox->addWidget(mSearchLineEdit);

    mSearchButton = new QPushButton(QIcon::fromTheme(QStringLiteral("edit-find")), i18n("Search..."));
    connect(mSearchButton, &QPushButton::clicked, this, &KNoteFindWidget::slotSearchNote);
    hbox->addWidget(mSearchButton);
    mSearchButton->setEnabled(false);

    //Result
    mNoteList = new NoteShared::NoteListWidget;
    mNoteList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(mNoteList, &NoteShared::NoteListWidget::itemDoubleClicked, this, &KNoteFindWidget::slotItemDoubleClicked);
    vbox->addWidget(mNoteList);

    mResultSearch = new QLabel;
    vbox->addWidget(mResultSearch);

    mSearchLineEdit->setFocus();

    setLayout(vbox);
}

KNoteFindWidget::~KNoteFindWidget()
{

}

void KNoteFindWidget::setExistingNotes(const QHash<Akonadi::Item::Id, Akonadi::Item> &notes)
{
    mNotes = notes;
}

void KNoteFindWidget::slotItemDoubleClicked(QListWidgetItem *item)
{
    Q_EMIT noteSelected(mNoteList->itemId(item));
}

void KNoteFindWidget::slotSearchNote()
{
    const QString searchStr = mSearchLineEdit->text().trimmed();
    if (searchStr.trimmed().isEmpty()) {
        return;
    }
    Akonadi::Search::PIM::NoteQuery query;
    query.matchNote(searchStr);
    query.matchTitle(searchStr);

    Akonadi::Search::PIM::ResultIterator it = query.exec();

    Akonadi::Item::List lst;
    while (it.next()) {
        const Akonadi::Item::Id id = it.id();
        if (mNotes.contains(id)) {
            lst << mNotes.value(id);
        }
    }
    mNoteList->setNotes(lst);
    if (lst.isEmpty()) {
        mResultSearch->setText(i18n("No Result found."));
    } else {
        mResultSearch->clear();
    }
}

void KNoteFindWidget::slotTextChanged(const QString &text)
{
    mSearchButton->setEnabled(!text.trimmed().isEmpty());
}
