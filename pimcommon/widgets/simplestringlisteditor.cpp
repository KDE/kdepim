/*  -*- mode: C++; c-file-style: "gnu" -*-
    simplestringlisteditor.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2001 Marc Mutz <mutz@kde.org>

    Copyright (c) 2013 Laurent Montel <montel@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "simplestringlisteditor.h"

#include <qinputdialog.h>
#include <kiconloader.h>
#include <QIcon>
#include <KLocalizedString>
#include "pimcommon_debug.h"
#include <QPushButton>
#include <QDialog>
#include <QMenu>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <KConfigGroup>

//********************************************************
// SimpleStringListEditor
//********************************************************
using namespace PimCommon;
SimpleStringListEditor::SimpleStringListEditor(QWidget *parent,
        ButtonCode buttons,
        const QString &addLabel,
        const QString &removeLabel,
        const QString &modifyLabel,
        const QString &addDialogLabel)
    : QWidget(parent),
      mAddButton(Q_NULLPTR), mRemoveButton(Q_NULLPTR), mModifyButton(Q_NULLPTR),
      mUpButton(Q_NULLPTR), mDownButton(Q_NULLPTR),
      mAddDialogLabel(addDialogLabel.isEmpty() ?
                      i18n("New entry:") : addDialogLabel)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QHBoxLayout *hlay = new QHBoxLayout(this);
    hlay->setMargin(0);

    mListBox = new QListWidget(this);

    mListBox->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mListBox, &QListWidget::customContextMenuRequested, this, &SimpleStringListEditor::slotContextMenu);

    mListBox->setSelectionMode(QAbstractItemView::ExtendedSelection);
    hlay->addWidget(mListBox, 1);

    if (buttons == None) {
        qCDebug(PIMCOMMON_LOG) << "SimpleStringListBox called with no buttons."
                 "Consider using a plain QListBox instead!";
    }

    mButtonLayout = new QVBoxLayout(); // inherits spacing
    hlay->addLayout(mButtonLayout);

    if (buttons & Add) {
        if (addLabel.isEmpty()) {
            mAddButton = new QPushButton(i18n("&Add..."), this);
        } else {
            mAddButton = new QPushButton(addLabel, this);
        }
        mAddButton->setAutoDefault(false);
        mButtonLayout->addWidget(mAddButton);
        connect(mAddButton, &QPushButton::clicked, this, &SimpleStringListEditor::slotAdd);
    }

    if (buttons & Modify) {
        if (modifyLabel.isEmpty()) {
            mModifyButton = new QPushButton(i18n("&Modify..."), this);
        } else {
            mModifyButton = new QPushButton(modifyLabel, this);
        }
        mModifyButton->setAutoDefault(false);
        mModifyButton->setEnabled(false);   // no selection yet
        mButtonLayout->addWidget(mModifyButton);
        connect(mModifyButton, &QPushButton::clicked, this, &SimpleStringListEditor::slotModify);
        connect(mListBox, &QListWidget::itemDoubleClicked, this, &SimpleStringListEditor::slotModify);
    }

    if (buttons & Remove) {
        if (removeLabel.isEmpty()) {
            mRemoveButton = new QPushButton(i18n("&Remove"), this);
        } else {
            mRemoveButton = new QPushButton(removeLabel, this);
        }
        mRemoveButton->setAutoDefault(false);
        mRemoveButton->setEnabled(false);   // no selection yet
        mButtonLayout->addWidget(mRemoveButton);
        connect(mRemoveButton, &QPushButton::clicked, this, &SimpleStringListEditor::slotRemove);
    }

    if (buttons & Up) {
        if (!(buttons & Down)) {
            qCDebug(PIMCOMMON_LOG) << "Are you sure you want to use an Up button"
                     "without a Down button??";
        }
        mUpButton = new QPushButton(QString(), this);
        mUpButton->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
        mUpButton->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
        mUpButton->setAutoDefault(false);
        mUpButton->setEnabled(false);   // no selection yet
        mButtonLayout->addWidget(mUpButton);
        connect(mUpButton, &QPushButton::clicked, this, &SimpleStringListEditor::slotUp);
    }

    if (buttons & Down) {
        if (!(buttons & Up)) {
            qCDebug(PIMCOMMON_LOG) << "Are you sure you want to use a Down button"
                     "without an Up button??";
        }
        mDownButton = new QPushButton(QString(), this);
        mDownButton->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
        mDownButton->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
        mDownButton->setAutoDefault(false);
        mDownButton->setEnabled(false);   // no selection yet
        mButtonLayout->addWidget(mDownButton);
        connect(mDownButton, &QPushButton::clicked, this, &SimpleStringListEditor::slotDown);
    }

    mButtonLayout->addStretch(1);   // spacer

    connect(mListBox, &QListWidget::currentItemChanged, this, &SimpleStringListEditor::slotSelectionChanged);
    connect(mListBox, &QListWidget::itemSelectionChanged, this, &SimpleStringListEditor::slotSelectionChanged);
}

void SimpleStringListEditor::setUpDownAutoRepeat(bool b)
{
    if (mUpButton) {
        mUpButton->setAutoRepeat(b);
    }
    if (mDownButton) {
        mDownButton->setAutoRepeat(b);
    }
}

void SimpleStringListEditor::setStringList(const QStringList &strings)
{
    mListBox->clear();
    mListBox->addItems(strings);
}

void SimpleStringListEditor::appendStringList(const QStringList &strings)
{
    mListBox->addItems(strings);
}

QStringList SimpleStringListEditor::stringList() const
{
    QStringList result;
    const int numberOfItem(mListBox->count());
    for (int i = 0; i < numberOfItem; ++i) {
        result << (mListBox->item(i)->text());
    }
    return result;
}

bool SimpleStringListEditor::containsString(const QString &str)
{
    const int numberOfItem(mListBox->count());
    for (int i = 0; i < numberOfItem; ++i) {
        if (mListBox->item(i)->text() == str) {
            return true;
        }
    }
    return false;
}

void SimpleStringListEditor::setButtonText(ButtonCode button, const QString &text)
{
    switch (button) {
    case Add:
        if (!mAddButton) {
            break;
        }
        mAddButton->setText(text);
        return;
    case Remove:
        if (!mRemoveButton) {
            break;
        }
        mRemoveButton->setText(text);
        return;
    case Modify:
        if (!mModifyButton) {
            break;
        }
        mModifyButton->setText(text);
        return;
    case Up:
    case Down:
        qCDebug(PIMCOMMON_LOG) << "SimpleStringListEditor: Cannot change text of"
                 "Up and Down buttons: they don't contains text!";
        return;
    default:
        if (button & All) {
            qCDebug(PIMCOMMON_LOG) << "No such button!";
        } else {
            qCDebug(PIMCOMMON_LOG) << "Can only set text for one button at a time!";
        }
        return;
    }

    qCDebug(PIMCOMMON_LOG) << "The requested button has not been created!";
}

void SimpleStringListEditor::addNewEntry()
{
    bool ok = false;
    QString newEntry = QInputDialog::getText(this, i18n("New Value"),
                       mAddDialogLabel, QLineEdit::Normal, QString(),
                       &ok);
    if (ok) {
        insertNewEntry(newEntry);
    }
}

void SimpleStringListEditor::insertNewEntry(const QString &entry)
{
    QString newEntry = entry;
    // let the user verify the string before adding
    emit aboutToAdd(newEntry);
    if (!newEntry.isEmpty() && !containsString(newEntry)) {
        mListBox->addItem(newEntry);
        slotSelectionChanged();
        emit changed();
    }
}

void SimpleStringListEditor::slotAdd()
{
    addNewEntry();
}

void SimpleStringListEditor::slotRemove()
{
    QList<QListWidgetItem *> selectedItems = mListBox->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }
    Q_FOREACH (QListWidgetItem *item, selectedItems) {
        delete mListBox->takeItem(mListBox->row(item));
    }
    slotSelectionChanged();
    emit changed();
}

QString SimpleStringListEditor::modifyEntry(const QString &text)
{
    bool ok = false;
    QString newText = QInputDialog::getText(this, i18n("Change Value"),
                                            mAddDialogLabel, QLineEdit::Normal, text,
                                            &ok);
    emit aboutToAdd(newText);

    if (!ok || newText.isEmpty() || newText == text) {
        return QString();
    }

    return newText;
}

void SimpleStringListEditor::slotModify()
{
    QListWidgetItem *item = mListBox->currentItem();
    if (!item) {
        return;
    }
    const QString newText = modifyEntry(item->text());
    if (!newText.isEmpty()) {
        item->setText(newText);
        emit changed();
    }
}

QList<QListWidgetItem *> SimpleStringListEditor::selectedItems() const
{
    QList<QListWidgetItem *> listWidgetItem;
    const int numberOfFilters = mListBox->count();
    for (int i = 0; i < numberOfFilters; ++i) {
        if (mListBox->item(i)->isSelected()) {
            listWidgetItem << mListBox->item(i);
        }
    }
    return listWidgetItem;
}

void SimpleStringListEditor::slotUp()
{
    QList<QListWidgetItem *> listWidgetItem = selectedItems();
    if (listWidgetItem.isEmpty()) {
        return;
    }

    const int numberOfItem(listWidgetItem.count());
    const int currentRow = mListBox->currentRow();
    if ((numberOfItem == 1) && (currentRow == 0)) {
        qCDebug(PIMCOMMON_LOG) << "Called while the _topmost_ filter is selected, ignoring.";
        return;
    }
    bool wasMoved = false;

    for (int i = 0; i < numberOfItem; ++i) {
        const int posItem = mListBox->row(listWidgetItem.at(i));
        if (posItem == i) {
            continue;
        }
        QListWidgetItem *item = mListBox->takeItem(posItem);
        mListBox->insertItem(posItem - 1, item);

        wasMoved = true;
    }
    if (wasMoved) {
        emit changed();
        mListBox->setCurrentRow(currentRow - 1);
    }
}

void SimpleStringListEditor::slotDown()
{
    QList<QListWidgetItem *> listWidgetItem = selectedItems();
    if (listWidgetItem.isEmpty()) {
        return;
    }

    const int numberOfElement(mListBox->count());
    const int numberOfItem(listWidgetItem.count());
    const int currentRow = mListBox->currentRow();
    if ((numberOfItem == 1) && (currentRow == numberOfElement - 1)) {
        qCDebug(PIMCOMMON_LOG) << "Called while the _last_ filter is selected, ignoring.";
        return;
    }

    int j = 0;
    bool wasMoved = false;
    for (int i = numberOfItem - 1; i >= 0; --i, j++) {
        const int posItem = mListBox->row(listWidgetItem.at(i));
        if (posItem == (numberOfElement - 1 - j)) {
            continue;
        }
        QListWidgetItem *item = mListBox->takeItem(posItem);
        mListBox->insertItem(posItem + 1, item);
        wasMoved = true;
    }
    if (wasMoved) {
        emit changed();
        mListBox->setCurrentRow(currentRow + 1);
    }
}

void SimpleStringListEditor::slotSelectionChanged()
{

    QList<QListWidgetItem *> lstSelectedItems = mListBox->selectedItems();
    const int numberOfItemSelected(lstSelectedItems.count());
    const bool uniqItemSelected = (numberOfItemSelected == 1);
    // if there is one, item will be non-null (ie. true), else 0
    // (ie. false):
    if (mRemoveButton) {
        mRemoveButton->setEnabled(!lstSelectedItems.isEmpty());
    }

    if (mModifyButton) {
        mModifyButton->setEnabled(uniqItemSelected);
    }

    const int currentIndex = mListBox->currentRow();

    const bool aItemIsSelected = !lstSelectedItems.isEmpty();
    const bool allItemSelected = (mListBox->count() == numberOfItemSelected);
    const bool theLast = (currentIndex >= mListBox->count() - 1);
    const bool theFirst = (currentIndex == 0);

    if (mUpButton) {
        mUpButton->setEnabled(aItemIsSelected && ((uniqItemSelected && !theFirst) ||
                              (!uniqItemSelected)) && !allItemSelected);
    }
    if (mDownButton) {
        mDownButton->setEnabled(aItemIsSelected &&
                                ((uniqItemSelected && !theLast) ||
                                 (!uniqItemSelected)) && !allItemSelected);
    }
}

void SimpleStringListEditor::slotContextMenu(const QPoint &pos)
{
    QList<QListWidgetItem *> lstSelectedItems = mListBox->selectedItems();
    const bool hasItemsSelected = !lstSelectedItems.isEmpty();
    QMenu *menu = new QMenu(this);
    if (mAddButton) {
        menu->addAction(mAddButton->text(), this, SLOT(slotAdd()));
    }
    if (mModifyButton && (lstSelectedItems.count() == 1)) {
        menu->addAction(mModifyButton->text(), this, SLOT(slotModify()));
    }
    if (mRemoveButton && hasItemsSelected) {
        menu->addSeparator();
        menu->addAction(mRemoveButton->text(), this, SLOT(slotRemove()));
    }
    menu->exec(mListBox->mapToGlobal(pos));
    delete menu;
}

QSize SimpleStringListEditor::sizeHint() const
{
    // Override height because we want the widget to be tall enough to fit the
    // button columns, but we want to allow it to be made smaller than list
    // sizeHint().height()
    QSize sh = QWidget::sizeHint();
    sh.setHeight(mButtonLayout->minimumSize().height());
    return sh;
}

