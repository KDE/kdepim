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

    mLockSearch = new QToolButton(this);
    mLockSearch->setCheckable(true);
    mLockSearch->setText(i18nc("@action:button", "Lock search"));
    mLockSearch->setFocusPolicy(Qt::StrongFocus);
    mLockSearch->setWhatsThis(
        i18nc("@info:whatsthis",
              "Toggle this button if you want to keep your quick search "
              "locked when moving to other folders or when narrowing the search "
              "by message status."));
    slotLockSearchClicked(false);
    connect(mLockSearch, &QToolButton::toggled, this, &QuickSearchLine::slotLockSearchClicked);
    hbox->addWidget(mLockSearch);

    mQuickSearchFilterWidget = new QWidget;
    mQuickSearchFilterWidget->setObjectName(QStringLiteral("quicksearchfilterwidget"));
    QHBoxLayout *quickSearchButtonLayout = new QHBoxLayout;
    mQuickSearchFilterWidget->setLayout(quickSearchButtonLayout);
    quickSearchButtonLayout->addStretch(0);
    QLabel *quickLab = new QLabel(i18n("Quick Filter:"));
    quickSearchButtonLayout->addWidget(quickLab);
    initializeStatusSearchButton(quickSearchButtonLayout);
    vbox->addWidget(mQuickSearchFilterWidget);

    mSearchEdit = new PimCommon::LineEditWithCompleter(this);
    mSearchEdit->setPlaceholderText(i18nc("Search for messages.", "Search"));
    mSearchEdit->setObjectName(QStringLiteral("quicksearch"));
    mSearchEdit->setClearButtonShown(true);

    connect(mSearchEdit, &KLineEdit::textChanged, this, &QuickSearchLine::slotSearchEditTextEdited);
    connect(mSearchEdit, &KLineEdit::clearButtonClicked, this, &QuickSearchLine::slotClearButtonClicked);

    hbox->addWidget(mSearchEdit);

    mMoreOptions = new QPushButton(QIcon::fromTheme(QStringLiteral("arrow-down-double")), i18n("More..."), this);
    mMoreOptions->setObjectName(QStringLiteral("moreoptions"));
    mMoreOptions->setFlat(true);
    mMoreOptions->setCheckable(true);
    connect(mMoreOptions, &QPushButton::toggled, this, &QuickSearchLine::slotMoreOptionClicked);
    hbox->addWidget(mMoreOptions);

    // The status filter button. Will be populated later, as populateStatusFilterCombo() is virtual
    mTagFilterCombo = new KComboBox(this);
    mTagFilterCombo->setMaximumWidth(300);
    mTagFilterCombo->setMaximumWidth(200);
    mTagFilterCombo->hide();
    hbox->addWidget(mTagFilterCombo);

    //Be disable until we have a storageModel => logical that it's disable.
    mSearchEdit->setEnabled(false);
    mTagFilterCombo->setEnabled(false);

    mExtraOption = new QWidget;
    mExtraOption->setObjectName(QStringLiteral("extraoptions"));
    hbox = new QHBoxLayout;
    hbox->setMargin(0);
    vbox->addWidget(mExtraOption);
    mExtraOption->setLayout(hbox);
    mExtraOption->hide();

    hbox->addStretch(0);
    QLabel *lab = new QLabel(i18n("Filter message by:"));
    hbox->addWidget(lab);

    mSearchEveryWhere = new QPushButton(i18n("Full Message"));
    mSearchEveryWhere->setObjectName(QStringLiteral("full_message"));
    mSearchEveryWhere->setFlat(true);
    mSearchEveryWhere->setCheckable(true);
    mSearchEveryWhere->setChecked(true);
    connect(mSearchEveryWhere, &QPushButton::clicked, this, &QuickSearchLine::slotSearchBy);
    hbox->addWidget(mSearchEveryWhere);

    mSearchAgainstBody = new QPushButton(i18n("Body"));
    mSearchAgainstBody->setObjectName(QStringLiteral("body"));
    mSearchAgainstBody->setFlat(true);
    mSearchAgainstBody->setCheckable(true);
    connect(mSearchAgainstBody, &QPushButton::clicked, this, &QuickSearchLine::slotSearchBy);
    hbox->addWidget(mSearchAgainstBody);

    mSearchAgainstSubject = new QPushButton(i18n("Subject"));
    mSearchAgainstSubject->setCheckable(true);
    mSearchAgainstSubject->setFlat(true);
    mSearchAgainstSubject->setObjectName(QStringLiteral("subject"));
    connect(mSearchAgainstSubject, &QPushButton::clicked, this, &QuickSearchLine::slotSearchBy);
    hbox->addWidget(mSearchAgainstSubject);

    mSearchAgainstFromOrTo = new QPushButton;
    changeSearchAgainstFromOrToText();
    mSearchAgainstFromOrTo->setObjectName(QStringLiteral("fromorto"));
    mSearchAgainstFromOrTo->setCheckable(true);
    mSearchAgainstFromOrTo->setFlat(true);
    connect(mSearchAgainstFromOrTo, &QPushButton::clicked, this, &QuickSearchLine::slotSearchBy);
    hbox->addWidget(mSearchAgainstFromOrTo);

    mSearchAgainstBcc = new QPushButton(i18n("Bcc"));
    mSearchAgainstBcc->setObjectName(QStringLiteral("bcc"));
    mSearchAgainstBcc->setCheckable(true);
    mSearchAgainstBcc->setFlat(true);
    connect(mSearchAgainstBcc, &QPushButton::clicked, this, &QuickSearchLine::slotSearchBy);
    hbox->addWidget(mSearchAgainstBcc);

    mQuickSearchFilterWidget->hide();

    installEventFilter(this);
    mMoreOptions->installEventFilter(this);
    mTagFilterCombo->installEventFilter(this);
    mLockSearch->installEventFilter(this);
    mSearchEveryWhere->installEventFilter(this);
    mSearchAgainstBody->installEventFilter(this);
    mSearchAgainstSubject->installEventFilter(this);
    mSearchAgainstFromOrTo->installEventFilter(this);
    mSearchAgainstBcc->installEventFilter(this);
    mQuickSearchFilterWidget->installEventFilter(this);
    mExtraOption->installEventFilter(this);
    changeQuicksearchVisibility(MessageListSettings::self()->showQuickSearch());
}

QuickSearchLine::~QuickSearchLine()
{

}

void QuickSearchLine::slotSearchBy()
{
    QObject *button = sender();
    if (mSearchEveryWhere != button) {
        mSearchEveryWhere->setChecked(false);
    }
    if (mSearchAgainstBody != button) {
        mSearchAgainstBody->setChecked(false);
    }
    if (mSearchAgainstSubject != button) {
        mSearchAgainstSubject->setChecked(false);
    }
    if (mSearchAgainstFromOrTo != button) {
        mSearchAgainstFromOrTo->setChecked(false);
    }
    if (mSearchAgainstBcc != button) {
        mSearchAgainstBcc->setChecked(false);
    }

    slotSearchOptionChanged();
}

void QuickSearchLine::slotMoreOptionClicked(bool b)
{
    mQuickSearchFilterWidget->setVisible(b);
    if (b) {
        mMoreOptions->setIcon(QIcon::fromTheme(QStringLiteral("arrow-up-double")));
    } else {
        mMoreOptions->setIcon(QIcon::fromTheme(QStringLiteral("arrow-down-double")));
    }
}

void QuickSearchLine::slotSearchEditTextEdited(const QString &text)
{
    int minimumStringLength = 3;
    if (text.startsWith(QLatin1Char('"')) && text.endsWith(QLatin1Char('"'))) {
        minimumStringLength = 5;
    }
    if (!text.trimmed().isEmpty()) {
        if (text.length() >= minimumStringLength) {
            mExtraOption->show();
            Q_EMIT searchEditTextEdited(text);
        } else {
            mExtraOption->hide();
        }
    } else {
        slotClearButtonClicked();
    }
}

void QuickSearchLine::slotClearButtonClicked()
{
    mExtraOption->hide();
    mSearchEveryWhere->setChecked(true);
    if (mTagFilterCombo->isVisible()) {
        mTagFilterCombo->setCurrentIndex(0);
    }
    Q_EMIT clearButtonClicked();
}

void QuickSearchLine::slotSearchOptionChanged()
{
    Q_EMIT searchOptionChanged();
}

QuickSearchLine::SearchOptions QuickSearchLine::searchOptions() const
{
    QuickSearchLine::SearchOptions searchOptions;
    if (mSearchEveryWhere->isChecked()) {
        searchOptions |= SearchEveryWhere;
    }
    if (mSearchAgainstBody->isChecked()) {
        searchOptions |= SearchAgainstBody;
    }
    if (mSearchAgainstSubject->isChecked()) {
        searchOptions |= SearchAgainstSubject;
    }
    if (mSearchAgainstFromOrTo->isChecked()) {
        if (mContainsOutboundMessages) {
            searchOptions |= SearchAgainstTo;
        } else {
            searchOptions |= SearchAgainstFrom;
        }
    }
    if (mSearchAgainstBcc->isChecked()) {
        searchOptions |= SearchAgainstBcc;
    }
    return searchOptions;
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

QToolButton *QuickSearchLine::lockSearch() const
{
    return mLockSearch;
}

void QuickSearchLine::slotLockSearchClicked(bool locked)
{
    if (locked) {
        mLockSearch->setIcon(QIcon::fromTheme(QStringLiteral("object-locked")));
        mLockSearch->setToolTip(i18nc("@info:tooltip", "Clear the quick search field when changing folders"));
    } else {
        mLockSearch->setIcon(QIcon::fromTheme(QStringLiteral("object-unlocked")));
        mLockSearch->setToolTip(i18nc("@info:tooltip",
                                      "Prevent the quick search field from being cleared when changing folders"));
    }
}

void QuickSearchLine::resetFilter()
{
    Q_FOREACH (QToolButton *button, mListStatusButton) {
        button->setChecked(false);
    }
    if (mTagFilterCombo->isVisible()) {
        mTagFilterCombo->setCurrentIndex(0);
    }
    mLockSearch->setChecked(false);
    mSearchEveryWhere->setChecked(true);
    mSearchAgainstBody->setChecked(false);
    mSearchAgainstSubject->setChecked(false);
    mSearchAgainstFromOrTo->setChecked(false);
    mSearchAgainstBcc->setChecked(false);

    mExtraOption->hide();
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
        changeSearchAgainstFromOrToText();
    }
}

void QuickSearchLine::changeSearchAgainstFromOrToText()
{
    if (mContainsOutboundMessages) {
        mSearchAgainstFromOrTo->setText(i18n("To"));
    } else {
        mSearchAgainstFromOrTo->setText(i18n("From"));
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
    mLockSearch->setVisible(show);
    mMoreOptions->setVisible(show);
}
