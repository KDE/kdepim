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

#include <KLocalizedString>
#include <KLineEdit>
#include <KComboBox>
#include <KIcon>

#include <QToolButton>
#include <QHBoxLayout>
using namespace MessageList::Core;
QuickSearchLine::QuickSearchLine(QWidget *parent)
    : QWidget(parent)
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

    mLockSearch->setVisible( Settings::self()->showQuickSearch() );
    connect( mLockSearch, SIGNAL(toggled(bool)), this, SIGNAL(lockSearchClicked(bool)));
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
    //TODO defaultFilterStatus();
    hbox->addWidget( mStatusFilterCombo );

    // The "Open Full Search" button
    mOpenFullSearchButton = new QToolButton( this );
    mOpenFullSearchButton->setIcon( KIcon( QLatin1String( "edit-find-mail" ) ) );
    mOpenFullSearchButton->setText( i18n( "Open Full Search" ) );
    mOpenFullSearchButton->setToolTip( mOpenFullSearchButton->text() );
    mOpenFullSearchButton->setVisible( Settings::self()->showQuickSearch() );
    hbox->addWidget( mOpenFullSearchButton );

    connect( mOpenFullSearchButton, SIGNAL(clicked()), this, SIGNAL(fullSearchRequest()) );

}

QuickSearchLine::~QuickSearchLine()
{

}

