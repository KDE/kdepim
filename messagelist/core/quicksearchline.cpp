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
using namespace MessageList::Core;
QuickSearchLine::QuickSearchLine(QWidget *parent)
    : QWidget(parent),
      mFirstTagInComboIndex(-1)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    setLayout(hbox);
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


    mSearchEdit = new KLineEdit( this );
    mSearchEdit->setClickMessage( i18nc( "Search for messages.", "Search" ) );
    mSearchEdit->setObjectName( QLatin1String( "quicksearch" ) );
    mSearchEdit->setClearButtonShown( true );
    mSearchEdit->setVisible( Settings::self()->showQuickSearch() );

    connect( mSearchEdit, SIGNAL(textEdited(QString)), this, SIGNAL(searchEditTextEdited(QString)));

    connect( mSearchEdit, SIGNAL(clearButtonClicked()), this, SIGNAL(clearButtonClicked()));

    hbox->addWidget( mSearchEdit );

    // The status filter button. Will be populated later, as populateStatusFilterCombo() is virtual
    mStatusFilterCombo = new KComboBox( this ) ;
    mStatusFilterCombo->setVisible( Settings::self()->showQuickSearch() );
    mStatusFilterCombo->setMaximumWidth(300);
    defaultFilterStatus();
    hbox->addWidget( mStatusFilterCombo );

    // The "Open Full Search" button
    mOpenFullSearchButton = new QToolButton( this );
    mOpenFullSearchButton->setIcon( KIcon( QLatin1String( "edit-find-mail" ) ) );
    mOpenFullSearchButton->setText( i18n( "Open Full Search" ) );
    mOpenFullSearchButton->setToolTip( mOpenFullSearchButton->text() );
    mOpenFullSearchButton->setVisible( Settings::self()->showQuickSearch() );
    hbox->addWidget( mOpenFullSearchButton );

    connect( mOpenFullSearchButton, SIGNAL(clicked()), this, SIGNAL(fullSearchRequest()) );
    mSearchEdit->setEnabled( false );
    mStatusFilterCombo->setEnabled( false );
}

QuickSearchLine::~QuickSearchLine()
{

}

void QuickSearchLine::focusQuickSearch()
{
    mSearchEdit->setFocus();
}

KComboBox *QuickSearchLine::statusFilterComboBox() const
{
    return mStatusFilterCombo;
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

int QuickSearchLine::firstTagInComboIndex() const
{
    return mFirstTagInComboIndex;
}

void QuickSearchLine::defaultFilterStatus()
{
    mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "system-run" )), i18n( "Any Status" ), 0 );

    mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-unread" )),
                                 i18nc( "@action:inmenu Status of a message", "Unread" ),
                                 Akonadi::MessageStatus::statusUnread().toQInt32() );

    mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-replied" )),
                                 i18nc( "@action:inmenu Status of a message", "Replied" ),
                                 Akonadi::MessageStatus::statusReplied().toQInt32() );

    mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-forwarded" )),
                                 i18nc( "@action:inmenu Status of a message", "Forwarded" ),
                                 Akonadi::MessageStatus::statusForwarded().toQInt32() );

    mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "emblem-important" )),
                                 i18nc( "@action:inmenu Status of a message", "Important"),
                                 Akonadi::MessageStatus::statusImportant().toQInt32() );

    mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-task" )),
                                 i18nc( "@action:inmenu Status of a message", "Action Item" ),
                                 Akonadi::MessageStatus::statusToAct().toQInt32() );

    mStatusFilterCombo->addItem( QIcon( KStandardDirs::locate( "data", QLatin1String( "messagelist/pics/mail-thread-watch.png" ) ) ),
                                 i18nc( "@action:inmenu Status of a message", "Watched" ),
                                 Akonadi::MessageStatus::statusWatched().toQInt32() );

    mStatusFilterCombo->addItem( QIcon( KStandardDirs::locate( "data", QLatin1String( "messagelist/pics/mail-thread-ignored.png" ) ) ),
                                 i18nc( "@action:inmenu Status of a message", "Ignored" ),
                                 Akonadi::MessageStatus::statusIgnored().toQInt32() );

    mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-attachment" )),
                                 i18nc( "@action:inmenu Status of a message", "Has Attachment" ),
                                 Akonadi::MessageStatus::statusHasAttachment().toQInt32() );

    mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-invitation" )),
                                 i18nc( "@action:inmenu Status of a message", "Has Invitation" ),
                                 Akonadi::MessageStatus::statusHasInvitation().toQInt32() );

    mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-mark-junk" )),
                                 i18nc( "@action:inmenu Status of a message", "Spam" ),
                                 Akonadi::MessageStatus::statusSpam().toQInt32() );

    mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-mark-notjunk" )),
                                 i18nc( "@action:inmenu Status of a message", "Ham" ),
                                 Akonadi::MessageStatus::statusHam().toQInt32() );
    mFirstTagInComboIndex = mStatusFilterCombo->count();
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

