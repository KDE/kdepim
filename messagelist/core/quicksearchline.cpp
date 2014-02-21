/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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
#include "core/settings.h"
#include <akonadi/kmime/messagestatus.h>

#include <KLocalizedString>
#include <KLineEdit>
#include <KComboBox>
#include <KStandardDirs>
#include <KIcon>
#include <KIconLoader>

#include <QToolButton>
#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>

//#define SEARCH_EVERYWHERE 1

using namespace MessageList::Core;
QuickSearchLine::QuickSearchLine(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setMargin(0);
    setLayout(vbox);

    QWidget *w = new QWidget;
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    w->setLayout(hbox);
    vbox->addWidget(w);

    mLockSearch = new QToolButton( this );
    mLockSearch->setCheckable( true );
    mLockSearch->setText( i18nc( "@action:button", "Lock search" ) );
    mLockSearch->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "Toggle this button if you want to keep your quick search "
                       "locked when moving to other folders or when narrowing the search "
                       "by message status." ) );
    slotLockSearchClicked(false);
    mLockSearch->setVisible( Settings::self()->showQuickSearch() );
    connect( mLockSearch, SIGNAL(toggled(bool)), SLOT(slotLockSearchClicked(bool)));
    hbox->addWidget( mLockSearch );

    QHBoxLayout *quickSearchButtonLayout = new QHBoxLayout;
    QLabel *quickLab = new QLabel(i18n("Quick Filter:"));
    quickSearchButtonLayout->addWidget(quickLab);
    initializeStatusSearchButton(quickSearchButtonLayout);
    hbox->addLayout(quickSearchButtonLayout);

    mSearchEdit = new KLineEdit( this );
    mSearchEdit->setClickMessage( i18nc( "Search for messages.", "Search" ) );
    mSearchEdit->setObjectName( QLatin1String( "quicksearch" ) );
    mSearchEdit->setClearButtonShown( true );
    mSearchEdit->setVisible( Settings::self()->showQuickSearch() );

    connect( mSearchEdit, SIGNAL(textChanged(QString)), this, SLOT(slotSearchEditTextEdited(QString)));
    connect( mSearchEdit, SIGNAL(clearButtonClicked()), this, SLOT(slotClearButtonClicked()));


    hbox->addWidget( mSearchEdit );

    // The status filter button. Will be populated later, as populateStatusFilterCombo() is virtual
    mTagFilterCombo = new KComboBox( this ) ;
    mTagFilterCombo->setVisible( Settings::self()->showQuickSearch() );
    mTagFilterCombo->setMaximumWidth(300);
    mTagFilterCombo->setMaximumWidth(200);
    mTagFilterCombo->hide();
    hbox->addWidget( mTagFilterCombo );

    // The "Open Full Search" button
    mOpenFullSearchButton = new QToolButton( this );
    mOpenFullSearchButton->setIcon( KIcon( QLatin1String( "edit-find-mail" ) ) );
    mOpenFullSearchButton->setText( i18n( "Open Full Search" ) );
    mOpenFullSearchButton->setToolTip( mOpenFullSearchButton->text() );
    mOpenFullSearchButton->setVisible( Settings::self()->showQuickSearch() );
    hbox->addWidget( mOpenFullSearchButton );

    connect( mOpenFullSearchButton, SIGNAL(clicked()), this, SIGNAL(fullSearchRequest()) );
    mSearchEdit->setEnabled( false );
    mTagFilterCombo->setEnabled( false );

    mExtraOption = new QWidget;
    mExtraOption->setObjectName(QLatin1String("extraoptions"));
    hbox = new QHBoxLayout;
    hbox->setMargin(0);
    vbox->addWidget(mExtraOption);
    mExtraOption->setLayout(hbox);
    mExtraOption->hide();

    hbox->addStretch(0);
    QLabel *lab = new QLabel(i18n("Filter message by:"));
    hbox->addWidget(lab);

#ifdef SEARCH_EVERYWHERE
    mSearchEveryWhere = new QPushButton(i18n("Full Message"));
    mSearchEveryWhere->setObjectName(QLatin1String("full_message"));
    mSearchEveryWhere->setFlat(true);
    mSearchEveryWhere->setCheckable(true);
    hbox->addWidget(mSearchEveryWhere);
#endif

    mSearchAgainstBody = new QPushButton(i18n("Body"));
    mSearchAgainstBody->setObjectName(QLatin1String("body"));
    mSearchAgainstBody->setFlat(true);
#ifdef SEARCH_EVERYWHERE
    mSearchAgainstBody->setCheckable(false);
#else
    mSearchAgainstBody->setCheckable(true);
#endif
    hbox->addWidget(mSearchAgainstBody);

    mSearchAgainstSubject = new QPushButton(i18n("Subject"));
    mSearchAgainstSubject->setCheckable(true);
    mSearchAgainstSubject->setFlat(true);
    mSearchAgainstSubject->setObjectName(QLatin1String("subject"));
    hbox->addWidget(mSearchAgainstSubject);

    mSearchAgainstFrom = new QPushButton(i18n("From"));
    mSearchAgainstFrom->setObjectName(QLatin1String("from"));
    mSearchAgainstFrom->setCheckable(true);
    mSearchAgainstFrom->setFlat(true);
    hbox->addWidget(mSearchAgainstFrom);

    mSearchAgainstBcc = new QPushButton(i18n("Bcc"));
    mSearchAgainstBcc->setObjectName(QLatin1String("bcc"));
    mSearchAgainstBcc->setCheckable(true);
    mSearchAgainstBcc->setFlat(true);
    hbox->addWidget(mSearchAgainstBcc);
    mButtonSearchAgainstGroup = new QButtonGroup(this);
#ifdef SEARCH_EVERYWHERE
    mButtonSearchAgainstGroup->addButton(mSearchEveryWhere, 0);
#else
    mButtonSearchAgainstGroup->addButton(mSearchAgainstBody, 0);
#endif
    mButtonSearchAgainstGroup->addButton(mSearchAgainstSubject);
    mButtonSearchAgainstGroup->addButton(mSearchAgainstFrom);
    mButtonSearchAgainstGroup->addButton(mSearchAgainstBcc);
    mButtonSearchAgainstGroup->button(0)->setChecked(true);
    connect(mButtonSearchAgainstGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotSearchOptionChanged()));
    mButtonSearchAgainstGroup->setExclusive(true);
}

QuickSearchLine::~QuickSearchLine()
{

}

void QuickSearchLine::slotSearchEditTextEdited(const QString &text)
{
    if (text.isEmpty()) {
        mExtraOption->hide();
    } else {
        mExtraOption->show();
    }
    Q_EMIT searchEditTextEdited(text);
}

void QuickSearchLine::slotClearButtonClicked()
{
    mExtraOption->hide();
    mButtonSearchAgainstGroup->button(0)->setChecked(true);
    if (mTagFilterCombo->isVisible())
        mTagFilterCombo->setCurrentIndex(0);
    Q_EMIT clearButtonClicked();
}

void QuickSearchLine::slotSearchOptionChanged()
{
    Q_EMIT searchOptionChanged();
}

QuickSearchLine::SearchOptions QuickSearchLine::searchOptions() const
{
    QuickSearchLine::SearchOptions searchOptions;
#ifdef SEARCH_EVERYWHERE
    if (mSearchEveryWhere->isChecked()) {
        searchOptions |= SearchEveryWhere;
    }
#endif
    if (mSearchAgainstBody->isChecked()) {
        searchOptions |= SearchAgainstBody;
    }
    if (mSearchAgainstSubject->isChecked()) {
        searchOptions |= SearchAgainstSubject;
    }
    if (mSearchAgainstFrom->isChecked()) {
        searchOptions |= SearchAgainstFrom;
    }
    if (mSearchAgainstBcc->isChecked()) {
        searchOptions |= SearchAgainstBcc;
    }
    return searchOptions;
}

void QuickSearchLine::focusQuickSearch()
{
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

QToolButton *QuickSearchLine::openFullSearchButton() const
{
    return mOpenFullSearchButton;
}

QToolButton *QuickSearchLine::lockSearch() const
{
    return mLockSearch;
}

void QuickSearchLine::slotLockSearchClicked( bool locked )
{
    if ( locked ) {
        mLockSearch->setIcon( KIcon( QLatin1String( "object-locked" ) ) );
        mLockSearch->setToolTip( i18nc( "@info:tooltip", "Clear the quick search field when changing folders" ) );
    } else {
        mLockSearch->setIcon( KIcon( QLatin1String( "object-unlocked" ) ) );
        mLockSearch->setToolTip( i18nc( "@info:tooltip",
                                        "Prevent the quick search field from being cleared when changing folders" ) );
    }
}

void QuickSearchLine::resetFilter()
{
    Q_FOREACH(QToolButton *button, mListStatusButton) {
        button->setChecked(false);
    }
    if (mTagFilterCombo->isVisible())
        mTagFilterCombo->setCurrentIndex( 0 );
    mLockSearch->setChecked(false);
    mButtonSearchAgainstGroup->button(0)->setChecked(true);
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
    quickSearchButtonLayout->addWidget(button);
    mListStatusButton.append(button);
    mButtonStatusGroup->addButton(button);
}

void QuickSearchLine::initializeStatusSearchButton(QLayout *quickSearchButtonLayout)
{
    mButtonStatusGroup = new QButtonGroup(this);
    mButtonStatusGroup->setExclusive(false);
    connect(mButtonStatusGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(statusButtonsClicked()));

    createQuickSearchButton(SmallIcon(QLatin1String( "mail-unread" )), i18nc( "@action:inmenu Status of a message", "Unread" ), Akonadi::MessageStatus::statusUnread().toQInt32(),quickSearchButtonLayout );

    createQuickSearchButton( SmallIcon(QLatin1String( "mail-replied" )),
                                 i18nc( "@action:inmenu Status of a message", "Replied" ),
                                 Akonadi::MessageStatus::statusReplied().toQInt32(), quickSearchButtonLayout );

    createQuickSearchButton( SmallIcon(QLatin1String( "mail-forwarded" )),
                                 i18nc( "@action:inmenu Status of a message", "Forwarded" ),
                                 Akonadi::MessageStatus::statusForwarded().toQInt32(), quickSearchButtonLayout );

    createQuickSearchButton( SmallIcon(QLatin1String( "emblem-important" )),
                                 i18nc( "@action:inmenu Status of a message", "Important"),
                                 Akonadi::MessageStatus::statusImportant().toQInt32(), quickSearchButtonLayout );

    createQuickSearchButton( SmallIcon(QLatin1String( "mail-task" )),
                                 i18nc( "@action:inmenu Status of a message", "Action Item" ),
                                 Akonadi::MessageStatus::statusToAct().toQInt32(), quickSearchButtonLayout );

    createQuickSearchButton( QIcon( KStandardDirs::locate( "data", QLatin1String( "messagelist/pics/mail-thread-watch.png" ) ) ),
                                 i18nc( "@action:inmenu Status of a message", "Watched" ),
                                 Akonadi::MessageStatus::statusWatched().toQInt32(), quickSearchButtonLayout );

    createQuickSearchButton( QIcon( KStandardDirs::locate( "data", QLatin1String( "messagelist/pics/mail-thread-ignored.png" ) ) ),
                                 i18nc( "@action:inmenu Status of a message", "Ignored" ),
                                 Akonadi::MessageStatus::statusIgnored().toQInt32(), quickSearchButtonLayout );

    createQuickSearchButton( SmallIcon(QLatin1String( "mail-attachment" )),
                                 i18nc( "@action:inmenu Status of a message", "Has Attachment" ),
                                 Akonadi::MessageStatus::statusHasAttachment().toQInt32(), quickSearchButtonLayout );

    createQuickSearchButton( SmallIcon(QLatin1String( "mail-invitation" )),
                                 i18nc( "@action:inmenu Status of a message", "Has Invitation" ),
                                 Akonadi::MessageStatus::statusHasInvitation().toQInt32(), quickSearchButtonLayout );

    createQuickSearchButton( SmallIcon(QLatin1String( "mail-mark-junk" )),
                                 i18nc( "@action:inmenu Status of a message", "Spam" ),
                                 Akonadi::MessageStatus::statusSpam().toQInt32(), quickSearchButtonLayout );

    createQuickSearchButton( SmallIcon(QLatin1String( "mail-mark-notjunk" )),
                                 i18nc( "@action:inmenu Status of a message", "Ham" ),
                                 Akonadi::MessageStatus::statusHam().toQInt32(), quickSearchButtonLayout );
}

QList<Akonadi::MessageStatus> QuickSearchLine::status() const
{
    QList<Akonadi::MessageStatus> lstStatus;

    Q_FOREACH(QToolButton *button, mListStatusButton) {
        if (button->isChecked()) {
            Akonadi::MessageStatus status;
            status.fromQInt32( static_cast< qint32 >( button->property("statusvalue").toInt() ));
            lstStatus.append(status);
        }
    }
    return lstStatus;
}

void QuickSearchLine::updateComboboxVisibility()
{
    mTagFilterCombo->setVisible(mTagFilterCombo->count());
}
