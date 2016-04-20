/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

 This library is free software; you can redistribute it and/or modify it
 under the terms of the GNU Library General Public License as published by
 the Free Software Foundation; either version 2 of the License, or (at your
 option) any later version.

 This library is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to the
 Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301, USA.

*/

#include "quicksearchline.h"
#include "messagelistsettings.h"
#include <Akonadi/KMime/MessageStatus>

#include "PimCommon/LineEditWithCompleter"
#include <KLocalizedString>
#include <KLineEdit>
#include <KComboBox>

#include <QIcon>
#include <KIconLoader>
#include <QPushButton>

#include <QToolButton>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QLabel>
#include <QSignalMapper>
#include <QStandardPaths>

using namespace MessageList::Core;
QuickSearchLine::QuickSearchLine(QWidget *parent)
    : QWidget(parent),
      mContainsOutboundMessages(false),
      mFilterStatusMapper(Q_NULLPTR)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setMargin(0);
    vbox->setSpacing(0);
    setLayout(vbox);

    QWidget *w = new QWidget;
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(0);
    w->setLayout(hbox);
    vbox->addWidget(w);

    listKeyword << i18n("in") << i18n("ago") << i18n("yesterday") << i18n("today") << i18n("first") << i18n("last") << i18n("next") << i18n("PM") << i18n("AM")
                << i18n("pm") << i18n("am") << i18n("at") << i18n("containing") << i18n("contains") << i18n("greater than") << i18n("bigger than") << i18n("more than")
                << i18n("at least") << i18n(">") << i18n("after") << i18n("since") << i18n("smaller than") << i18n("less than") << i18n("lesser than") << i18n("at most")
                << i18n("<") << i18n("Before") << i18n("until") << i18n("equal to") << i18n("equal") << i18n("equals") << i18n("=") << i18n("rated as") << i18n("rated")
                << i18n("score is") << i18n("score") << i18n("scored") << i18n("having") << i18n("stars") << i18n("star") << i18n("Comment") << i18n("described as")
                << i18n("description is") << i18n("comment is") << i18n("description") << i18n("described") << i18n("comment") << i18n("sent by") << i18n("from")
                << i18n("sender is") << i18n("sender") << i18n("title is") << i18n("subject is") << i18n("subject") << i18n("title") << i18n("titled") << i18n("sent to")
                << i18n("recipient is") << i18n("recipient") << i18n("sent at") << i18n("sent on") << i18n("sent") << i18n("received at") << i18n("received on")
                << i18n("received") << i18n("reception is") << i18n("written by") << i18n("created by") << i18n("composed by") << i18n("author is") << i18n("by")
                << i18n("size is") << i18n("size") << i18n("being") << i18n("large") << i18n("name is") << i18n("name") << i18n("named") << i18n("name is")
                << i18n("created at") << i18n("dated at") << i18n("created on") << i18n("dated on") << i18n("created in") << i18n("dated in") << i18n("created of")
                << i18n("dated of") << i18n("created") << i18n("dated") << i18n("creation date is") << i18n("creation time is") << i18n("creation datetime is")
                << i18n("modification date is") << i18n("modification time is") << i18n("modification datetime is") << i18n("edition date is") << i18n("edition time is")
                << i18n("edition datetime is") << i18n("edited") << i18n("modified") << i18n("modified at") << i18n("modified on") << i18n("edited on") << i18n("edited at")
                << i18n("tagged as") << i18n("has tag") << i18n("tag is") << i18n("#") << i18n("related to") << i18n("a") << i18n("year") << i18n("week") << i18n("month")
                << i18n("day") << i18n("hour") << i18n("of") << i18n("by") << i18n("and") << i18n("on") << i18n("in") << i18n("or") << i18n("third") << i18n("second") << i18n("fourth")
                << i18n("then");



    mSearchEdit = new PimCommon::LineEditWithCompleter(this);
    mSearchEdit->setPlaceholderText(i18nc("Search for messages.", "Search"));
    mSearchEdit->setObjectName(QStringLiteral("quicksearch"));
    mSearchEdit->setClearButtonShown(true);
    connect(mSearchEdit, &KLineEdit::textChanged, this, &QuickSearchLine::slotSearchEditTextEdited);
    connect(mSearchEdit, &KLineEdit::clearButtonClicked, this, &QuickSearchLine::slotClearButtonClicked);
    Q_FOREACH(const QString &str, listKeyword) {
        searchEdit()->completionObject()->addItem(str);
    }
    hbox->addWidget(mSearchEdit);

    // The status filter button. Will be populated later, as populateStatusFilterCombo() is virtual
    mTagFilterCombo = new KComboBox(this);
    mTagFilterCombo->setMaximumWidth(300);
    mTagFilterCombo->setMaximumWidth(200);
    mTagFilterCombo->hide();
    hbox->addWidget(mTagFilterCombo);

    //Be disable until we have a storageModel => logical that it's disable.
    mSearchEdit->setEnabled(false);
    mTagFilterCombo->setEnabled(false);

    installEventFilter(this);
    mTagFilterCombo->installEventFilter(this);
    changeQuicksearchVisibility(MessageListSettings::self()->showQuickSearch());

}

QuickSearchLine::~QuickSearchLine()
{

}

void QuickSearchLine::slotSearchEditTextEdited(const QString &text)
{
    int minimumStringLength = 3;
    if (text.startsWith(QLatin1Char('"')) && text.endsWith(QLatin1Char('"'))) {
        minimumStringLength = 5;
    }
    if (!text.trimmed().isEmpty()) {
        if (text.length() >= minimumStringLength) {
            Q_EMIT searchEditTextEdited(text);
        }
    } else {
        slotClearButtonClicked();
    }
}

void QuickSearchLine::slotClearButtonClicked()
{
    if (mTagFilterCombo->isVisible()) {
        mTagFilterCombo->setCurrentIndex(0);
    }
    Q_EMIT clearButtonClicked();
}

void QuickSearchLine::focusQuickSearch(const QString &selectedText)
{
    if (!selectedText.isEmpty()) {
        mSearchEdit->setText(selectedText);
    }
    mSearchEdit->setFocus();
}

KComboBox *QuickSearchLine::tagFilterComboBox() const
{
    return mTagFilterCombo;
}

KLineEdit *QuickSearchLine::searchEdit() const
{
    return mSearchEdit;
}


void QuickSearchLine::resetFilter()
{
    Q_FOREACH (QToolButton *button, mListStatusButton) {
        button->setChecked(false);
    }
    if (mTagFilterCombo->isVisible()) {
        mTagFilterCombo->setCurrentIndex(0);
    }
}

void QuickSearchLine::createQuickSearchButton(const QIcon &icon, const QString &text, int value, QLayout *quickSearchButtonLayout)
{
    QToolButton *button = new QToolButton;
    button->setIcon(icon);
    button->setText(text);
    button->setAutoRaise(true);
    button->setToolTip(text);
    button->setCheckable(true);
    button->setChecked(false);
    button->setProperty("statusvalue", value);
    mFilterStatusMapper->setMapping(button, value);
    connect(button, SIGNAL(clicked(bool)), mFilterStatusMapper, SLOT(map()));
    quickSearchButtonLayout->addWidget(button);
    button->installEventFilter(this);
    button->setFocusPolicy(Qt::StrongFocus);
    mListStatusButton.append(button);
}

bool QuickSearchLine::containsOutboundMessages() const
{
    return mContainsOutboundMessages;
}

void QuickSearchLine::setContainsOutboundMessages(bool containsOutboundMessages)
{
    if (mContainsOutboundMessages != containsOutboundMessages) {
        mContainsOutboundMessages = containsOutboundMessages;
    }
}

void QuickSearchLine::initializeStatusSearchButton(QLayout *quickSearchButtonLayout)
{
    //Bug Qt we can't use QButtonGroup + QToolButton + change focus. => use QSignalMapper();
    mFilterStatusMapper = new QSignalMapper(this);
    connect(mFilterStatusMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), this, &QuickSearchLine::statusButtonsClicked);

    createQuickSearchButton(QIcon::fromTheme(QStringLiteral("mail-unread")), i18nc("@action:inmenu Status of a message", "Unread"), Akonadi::MessageStatus::statusUnread().toQInt32(), quickSearchButtonLayout);

    createQuickSearchButton(QIcon::fromTheme(QStringLiteral("mail-replied")),
                            i18nc("@action:inmenu Status of a message", "Replied"),
                            Akonadi::MessageStatus::statusReplied().toQInt32(), quickSearchButtonLayout);

    createQuickSearchButton(QIcon::fromTheme(QStringLiteral("mail-forwarded")),
                            i18nc("@action:inmenu Status of a message", "Forwarded"),
                            Akonadi::MessageStatus::statusForwarded().toQInt32(), quickSearchButtonLayout);

    createQuickSearchButton(QIcon::fromTheme(QStringLiteral("emblem-important")),
                            i18nc("@action:inmenu Status of a message", "Important"),
                            Akonadi::MessageStatus::statusImportant().toQInt32(), quickSearchButtonLayout);

    createQuickSearchButton(QIcon::fromTheme(QStringLiteral("mail-task")),
                            i18nc("@action:inmenu Status of a message", "Action Item"),
                            Akonadi::MessageStatus::statusToAct().toQInt32(), quickSearchButtonLayout);

    createQuickSearchButton(QIcon(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("messagelist/pics/mail-thread-watch.png"))),
                            i18nc("@action:inmenu Status of a message", "Watched"),
                            Akonadi::MessageStatus::statusWatched().toQInt32(), quickSearchButtonLayout);

    createQuickSearchButton(QIcon(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("messagelist/pics/mail-thread-ignored.png"))),
                            i18nc("@action:inmenu Status of a message", "Ignored"),
                            Akonadi::MessageStatus::statusIgnored().toQInt32(), quickSearchButtonLayout);

    createQuickSearchButton(QIcon::fromTheme(QStringLiteral("mail-attachment")),
                            i18nc("@action:inmenu Status of a message", "Has Attachment"),
                            Akonadi::MessageStatus::statusHasAttachment().toQInt32(), quickSearchButtonLayout);

    createQuickSearchButton(QIcon::fromTheme(QStringLiteral("mail-invitation")),
                            i18nc("@action:inmenu Status of a message", "Has Invitation"),
                            Akonadi::MessageStatus::statusHasInvitation().toQInt32(), quickSearchButtonLayout);

    createQuickSearchButton(QIcon::fromTheme(QStringLiteral("mail-mark-junk")),
                            i18nc("@action:inmenu Status of a message", "Spam"),
                            Akonadi::MessageStatus::statusSpam().toQInt32(), quickSearchButtonLayout);

    createQuickSearchButton(QIcon::fromTheme(QStringLiteral("mail-mark-notjunk")),
                            i18nc("@action:inmenu Status of a message", "Ham"),
                            Akonadi::MessageStatus::statusHam().toQInt32(), quickSearchButtonLayout);
}

QList<Akonadi::MessageStatus> QuickSearchLine::status() const
{
    QList<Akonadi::MessageStatus> lstStatus;

    Q_FOREACH (QToolButton *button, mListStatusButton) {
        if (button->isChecked()) {
            Akonadi::MessageStatus status;
            status.fromQInt32(static_cast< qint32 >(button->property("statusvalue").toInt()));
            lstStatus.append(status);
        }
    }
    return lstStatus;
}

void QuickSearchLine::updateComboboxVisibility()
{
    mTagFilterCombo->setVisible(!mSearchEdit->isHidden() && mTagFilterCombo->count());
}

bool QuickSearchLine::eventFilter(QObject *object, QEvent *e)
{
    const bool shortCutOverride = (e->type() == QEvent::ShortcutOverride);
    if (shortCutOverride) {
        e->accept();
        return true;
    }
    return QWidget::eventFilter(object, e);
}

void QuickSearchLine::changeQuicksearchVisibility(bool show)
{
    mSearchEdit->setVisible(show);
    mTagFilterCombo->setVisible(show && mTagFilterCombo->count());
}
