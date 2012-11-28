/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "findreplacebar.h"

#include <KIcon>
#include <KLocale>

#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <KLineEdit>
#include <QPushButton>
#include <QAction>
#include <QMenu>

namespace ComposerEditorNG {

class FindReplaceBarPrivate
{
public:
    FindReplaceBarPrivate(FindReplaceBar *qq)
        :q(qq)
    {

    }
    FindReplaceBar *q;
    QString mPositiveBackground;
    QString mNegativeBackground;
    QString mLastSearchStr;

    KLineEdit * search;
    QAction * caseSensitiveAct;

    QPushButton *findPreviousButton;
    QPushButton *findNextButton;
    QMenu *optionsMenu;

};


FindReplaceBar::FindReplaceBar(QWidget *parent)
    : QWidget(parent), d(new FindReplaceBarPrivate(this))
{
    QHBoxLayout * lay = new QHBoxLayout( this );
    lay->setMargin( 2 );

    QToolButton * closeBtn = new QToolButton( this );
    closeBtn->setIcon( KIcon( QLatin1String("dialog-close") ) );
    closeBtn->setIconSize( QSize( 24, 24 ) );
    closeBtn->setToolTip( i18n( "Close" ) );

#ifndef QT_NO_ACCESSIBILITY
    closeBtn->setAccessibleName( i18n( "Close" ) );
#endif

    closeBtn->setAutoRaise( true );
    lay->addWidget( closeBtn );

    QLabel * label = new QLabel( i18nc( "Find text", "F&ind:" ), this );
    lay->addWidget( label );

    d->search = new KLineEdit( this );
    d->search->setToolTip( i18n( "Text to search for" ) );
    d->search->setClearButtonShown( true );
    label->setBuddy( d->search );
    lay->addWidget( d->search );

    d->findNextButton = new QPushButton( KIcon( QLatin1String("go-down-search") ), i18nc( "Find and go to the next search match", "Next" ), this );
    d->findNextButton->setToolTip( i18n( "Jump to next match" ) );
    lay->addWidget( d->findNextButton );
    d->findNextButton->setEnabled( false );

    d->findPreviousButton = new QPushButton( KIcon( QLatin1String("go-up-search") ), i18nc( "Find and go to the previous search match", "Previous" ), this );
    d->findPreviousButton->setToolTip( i18n( "Jump to previous match" ) );
    lay->addWidget( d->findPreviousButton );
    d->findPreviousButton->setEnabled( false );

    QPushButton * optionsBtn = new QPushButton( this );
    optionsBtn->setText( i18n( "Options" ) );
    optionsBtn->setToolTip( i18n( "Modify search behavior" ) );
    d->optionsMenu = new QMenu( optionsBtn );
    d->caseSensitiveAct = d->optionsMenu->addAction( i18n( "Case sensitive" ) );
    d->caseSensitiveAct->setCheckable( true );

    optionsBtn->setMenu( d->optionsMenu );
    lay->addWidget( optionsBtn );

    /*
    connect( closeBtn, SIGNAL(clicked()), this, SLOT(closeBar()) );
    connect( d->findNextButton, SIGNAL(clicked()), this, SLOT(findNext()) );
    connect( d->findPreviousButton, SIGNAL(clicked()), this, SLOT(findPrev()) );
    connect( d->caseSensitiveAct, SIGNAL(toggled(bool)), this, SLOT(caseSensitivityChanged(bool)) );
    connect( d->search, SIGNAL(textChanged(QString)), this, SLOT(autoSearch(QString)) );
    connect( d->search, SIGNAL(clearButtonClicked()), this, SLOT(slotClearSearch()) );
    */
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    hide();

}

FindReplaceBar::~FindReplaceBar()
{
    delete d;
}


}
