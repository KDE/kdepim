/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2007 Mike McQuaid <mike@mikemcquaid.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

// Journal Entry

#include "journalframe.h"

#include <CalendarSupport/Utils>

#include <KCalCore/Journal>

#include <KCalUtils/IncidenceFormatter>
#include <Akonadi/Calendar/ETMCalendar>

#include <QTextBrowser>
#include <KLocalizedString>
#include <KIconLoader>
#include "calendarview_debug.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFontDatabase>

using namespace EventViews;

JournalDateView::JournalDateView(const Akonadi::ETMCalendar::Ptr &calendar, QWidget *parent)
    : KVBox(parent), mCalendar(calendar), mChanger(Q_NULLPTR)
{
}

JournalDateView::~JournalDateView()
{
}

void JournalDateView::setDate(const QDate &date)
{
    mDate = date;
    Q_EMIT setDateSignal(date);
}

void JournalDateView::clear()
{
    qDeleteAll(mEntries);
    mEntries.clear();
}

// should only be called by the JournalView now.
void JournalDateView::addJournal(const Akonadi::Item &j)
{
    QMap<Akonadi::Item::Id, JournalFrame *>::Iterator pos = mEntries.find(j.id());
    if (pos != mEntries.end()) {
        return;
    }

    QWidget *container = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->addStretch(1);
    JournalFrame *entry = new JournalFrame(j, mCalendar, this);
    layout->addWidget(entry, 3/*stretch*/);
    layout->addStretch(1);

    entry->show();
    entry->setDate(mDate);
    entry->setIncidenceChanger(mChanger);

    mEntries.insert(j.id(), entry);
    connect(this, &JournalDateView::setIncidenceChangerSignal,
            entry, &JournalFrame::setIncidenceChanger);
    connect(this, &JournalDateView::setDateSignal,
            entry, &JournalFrame::setDate);
    connect(entry, &JournalFrame::deleteIncidence,
            this, &JournalDateView::deleteIncidence);
    connect(entry, &JournalFrame::editIncidence,
            this, &JournalDateView::editIncidence);
    connect(entry, &JournalFrame::incidenceSelected,
            this, &JournalDateView::incidenceSelected);
    connect(entry, SIGNAL(printJournal(KCalCore::Journal::Ptr,bool)),
            SIGNAL(printJournal(KCalCore::Journal::Ptr,bool)));
}

Akonadi::Item::List JournalDateView::journals() const
{
    Akonadi::Item::List l;
    l.reserve(mEntries.count());
    Q_FOREACH (const JournalFrame *const i, mEntries) {
        l.push_back(i->journal());
    }
    return l;
}

void JournalDateView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    mChanger = changer;
    Q_EMIT setIncidenceChangerSignal(changer);
}

void JournalDateView::emitNewJournal()
{
    Q_EMIT newJournal(mDate);
}

void JournalDateView::journalEdited(const Akonadi::Item &journal)
{
    QMap<Akonadi::Item::Id, JournalFrame *>::Iterator pos = mEntries.find(journal.id());
    if (pos == mEntries.end()) {
        return;
    }

    pos.value()->setJournal(journal);
}

void JournalDateView::journalDeleted(const Akonadi::Item &journal)
{
    QMap<Akonadi::Item::Id, JournalFrame *>::Iterator pos = mEntries.find(journal.id());
    if (pos == mEntries.end()) {
        return;
    }

    delete pos.value();
    mEntries.remove(journal.id());
}

JournalFrame::JournalFrame(const Akonadi::Item &j,
                           const Akonadi::ETMCalendar::Ptr &calendar,
                           QWidget *parent)
    : QFrame(parent), mJournal(j), mCalendar(calendar)
{
    mDirty = false;
    mWriteInProgress = false;
    mChanger = Q_NULLPTR;

    QVBoxLayout *verticalLayout = new QVBoxLayout(this);

    mBrowser = new QTextBrowser(this);
    mBrowser->viewport()->installEventFilter(this);
    mBrowser->setFrameStyle(QFrame::NoFrame);
    verticalLayout->addWidget(mBrowser);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    verticalLayout->addLayout(buttonsLayout);
    buttonsLayout->addStretch();

    mEditButton = new QPushButton(this);
    mEditButton->setObjectName(QStringLiteral("editButton"));
    mEditButton->setText(i18n("&Edit"));
    mEditButton->setIcon(QIcon::fromTheme(QStringLiteral("document-properties")));
    mEditButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mEditButton->setToolTip(i18n("Edit this journal entry"));
    mEditButton->setWhatsThis(i18n("Opens an editor dialog for this journal entry"));
    buttonsLayout->addWidget(mEditButton);
    connect(mEditButton, &QPushButton::clicked, this, &JournalFrame::editItem);

    mDeleteButton = new QPushButton(this);
    mDeleteButton->setObjectName(QStringLiteral("deleteButton"));
    mDeleteButton->setText(i18n("&Delete"));
    mDeleteButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    mDeleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mDeleteButton->setToolTip(i18n("Delete this journal entry"));
    mDeleteButton->setWhatsThis(i18n("Delete this journal entry"));
    buttonsLayout->addWidget(mDeleteButton);
    connect(mDeleteButton, &QPushButton::pressed, this, &JournalFrame::deleteItem);

    mPrintButton = new QPushButton(this);
    mPrintButton->setText(i18n("&Print"));
    mPrintButton->setObjectName(QStringLiteral("printButton"));
    mPrintButton->setIcon(QIcon::fromTheme(QStringLiteral("document-print")));
    mPrintButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mPrintButton->setToolTip(i18n("Print this journal entry"));
    mPrintButton->setWhatsThis(i18n("Opens a print dialog for this journal entry"));
    buttonsLayout->addWidget(mPrintButton);
    connect(mPrintButton, SIGNAL(clicked()), this, SLOT(printJournal()));

    mPrintPreviewButton = new QPushButton(this);
    mPrintPreviewButton->setText(i18n("Print preview"));
    mPrintPreviewButton->setObjectName(QStringLiteral("printButton"));
    mPrintPreviewButton->setIcon(QIcon::fromTheme(QStringLiteral("document-print-preview")));
    mPrintPreviewButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mPrintPreviewButton->setToolTip(i18n("Print preview this journal entry"));
    buttonsLayout->addWidget(mPrintPreviewButton);
    connect(mPrintPreviewButton, &QAbstractButton::clicked, this, &JournalFrame::printPreviewJournal);

    readJournal(mJournal);
    mDirty = false;
    setFrameStyle(QFrame::Box);
    // These probably shouldn't be hardcoded
    setStyleSheet(QStringLiteral("QFrame { border: 1px solid; border-radius: 7px; } "));
    mBrowser->setStyleSheet(QStringLiteral("QFrame { border: 0px solid white } "));
}

JournalFrame::~JournalFrame()
{
}

bool JournalFrame::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);

    // object is our QTextBrowser
    if (!mJournal.isValid()) {
        return false;
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress:
        Q_EMIT incidenceSelected(mJournal, mDate);
        break;
    case QEvent::MouseButtonDblClick:
        Q_EMIT editIncidence(mJournal);
        break;
    default:
        break;
    }

    return false;
}

void JournalFrame::deleteItem()
{
    if (CalendarSupport::hasJournal(mJournal)) {
        Q_EMIT deleteIncidence(mJournal);
    }
}

void JournalFrame::editItem()
{
    if (CalendarSupport::hasJournal(mJournal)) {
        Q_EMIT editIncidence(mJournal);
    }
}

void JournalFrame::setCalendar(const Akonadi::ETMCalendar::Ptr &calendar)
{
    mCalendar = calendar;
}

void JournalFrame::setDate(const QDate &date)
{
    mDate = date;
}

void JournalFrame::setJournal(const Akonadi::Item &journal)
{
    if (!CalendarSupport::hasJournal(journal)) {
        return;
    }

    mJournal = journal;
    readJournal(journal);

    mDirty = false;
}

void JournalFrame::setDirty()
{
    mDirty = true;
    qCDebug(CALENDARVIEW_LOG);
}

void JournalFrame::printJournal()
{
    Q_EMIT printJournal(CalendarSupport::journal(mJournal), false);
}

void JournalFrame::printPreviewJournal()
{
    Q_EMIT printJournal(CalendarSupport::journal(mJournal), true);
}

void JournalFrame::readJournal(const Akonadi::Item &j)
{
    int baseFontSize = QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSize();
    mJournal = j;
    const KCalCore::Journal::Ptr journal = CalendarSupport::journal(j);
    mBrowser->clear();
    QTextCursor cursor = QTextCursor(mBrowser->textCursor());
    cursor.movePosition(QTextCursor::Start);

    QTextBlockFormat bodyBlock = QTextBlockFormat(cursor.blockFormat());
    //FIXME: Do padding
    bodyBlock.setTextIndent(2);
    QTextCharFormat bodyFormat = QTextCharFormat(cursor.charFormat());
    if (!journal->summary().isEmpty()) {
        QTextCharFormat titleFormat = bodyFormat;
        titleFormat.setFontWeight(QFont::Bold);
        titleFormat.setFontPointSize(baseFontSize + 4);
        cursor.insertText(journal->summary(), titleFormat);
        cursor.insertBlock();
    }
    QTextCharFormat dateFormat = bodyFormat;
    dateFormat.setFontWeight(QFont::Bold);
    dateFormat.setFontPointSize(baseFontSize + 1);
    cursor.insertText(KCalUtils::IncidenceFormatter::dateTimeToString(
                          journal->dtStart(), journal->allDay()), dateFormat);
    cursor.insertBlock();
    cursor.insertBlock();
    cursor.setBlockCharFormat(bodyFormat);
    const QString description = journal->description();
    if (journal->descriptionIsRich()) {
        mBrowser->insertHtml(description);
    } else {
        mBrowser->insertPlainText(description);
    }

    if (mCalendar) {
        mEditButton->setEnabled(mCalendar->hasRight(j, Akonadi::Collection::CanChangeItem));
        mDeleteButton->setEnabled(mCalendar->hasRight(j, Akonadi::Collection::CanDeleteItem));
    }

}

