/* Copyright (C) 2012, 2013 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "plaintexteditfindbar.h"
#include "textfindreplacewidget.h"

#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <klineedit.h>
#include <KColorScheme>

#include <QPlainTextEdit>
#include <QTimer>
#include <QLayout>
#include <QToolButton>
#include <QEvent>
#include <QKeyEvent>

using namespace PimCommon;

PlainTextEditFindBar::PlainTextEditFindBar( QPlainTextEdit * view, QWidget * parent )
    : QWidget( parent ),
      mView( view )
{
    QVBoxLayout *topLayout = new QVBoxLayout;
    topLayout->setMargin(0);
    QHBoxLayout * lay = new QHBoxLayout;
    lay->setMargin( 2 );

    topLayout->addLayout(lay);

    QToolButton * closeBtn = new QToolButton( this );
    closeBtn->setIcon( KIcon( QLatin1String("dialog-close") ) );
    closeBtn->setIconSize( QSize( 16, 16 ) );
    closeBtn->setToolTip( i18n( "Close" ) );

#ifndef QT_NO_ACCESSIBILITY
    closeBtn->setAccessibleName( i18n( "Close" ) );
#endif

    closeBtn->setAutoRaise( true );
    lay->addWidget( closeBtn );

    mFindWidget = new PlainTextFindWidget;
    lay->addWidget( mFindWidget );

    mReplaceWidget = new PlainTextReplaceWidget;
    topLayout->addWidget(mReplaceWidget);
    mReplaceWidget->hide();


    connect( closeBtn, SIGNAL(clicked()), this, SLOT(closeBar()) );
    connect( mFindWidget, SIGNAL(findNext()), this, SLOT(findNext()) );
    connect( mFindWidget, SIGNAL(findPrev()), this, SLOT(findPrev()) );
    connect( mFindWidget, SIGNAL(updateSearchOptions()), this, SLOT(slotUpdateSearchOptions()) );
    connect( mFindWidget, SIGNAL(updateSearchOptions()), this, SLOT(slotUpdateSearchOptions()) );
    connect( mFindWidget, SIGNAL(autoSearch(QString)), this, SLOT(autoSearch(QString)) );
    connect( mFindWidget, SIGNAL(clearSearch()), this, SLOT(slotClearSearch()) );
    connect( mFindWidget, SIGNAL(searchStringEmpty(bool)), mReplaceWidget, SLOT(slotSearchStringEmpty(bool)));
    connect( mReplaceWidget, SIGNAL(replaceText()), this, SLOT(slotReplaceText()));
    connect( mReplaceWidget, SIGNAL(replaceAllText()), this, SLOT(slotReplaceAllText()));
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    hide();
    setLayout(topLayout);
}

PlainTextEditFindBar::~PlainTextEditFindBar()
{
}

void PlainTextEditFindBar::showFind()
{
    if (mView->toPlainText().isEmpty())
        return;
    mReplaceWidget->slotSearchStringEmpty(mFindWidget->search()->text().isEmpty());
    show();
    if (mReplaceWidget->isVisible()) {
        mReplaceWidget->hide();
        updateGeometry();
    }
}

void PlainTextEditFindBar::showReplace()
{
    if (mView->isReadOnly())
        return;
    if (mView->toPlainText().isEmpty())
        return;
    mReplaceWidget->slotSearchStringEmpty(mFindWidget->search()->text().isEmpty());
    show();
    if (!mReplaceWidget->isVisible()) {
        mReplaceWidget->show();
        updateGeometry();
    }
}

void PlainTextEditFindBar::setText( const QString&text )
{
    mFindWidget->search()->setText( text );
}

QString PlainTextEditFindBar::text() const
{
    return mFindWidget->search()->text();
}

void PlainTextEditFindBar::focusAndSetCursor()
{
    setFocus();
    mFindWidget->search()->selectAll();
    mFindWidget->search()->setFocus();
}

void PlainTextEditFindBar::slotClearSearch()
{
    clearSelections();
}

void PlainTextEditFindBar::autoSearch( const QString& str )
{
    const bool isNotEmpty = ( !str.isEmpty() );
    if ( isNotEmpty ) {
        mView->moveCursor(QTextCursor::Start);
        QTimer::singleShot( 0, this, SLOT(slotSearchText()) );
    }
    else
        clearSelections();
}

void PlainTextEditFindBar::slotSearchText( bool backward, bool isAutoSearch )
{
    searchText( backward, isAutoSearch );
}

void PlainTextEditFindBar::messageInfo( bool backward, bool isAutoSearch, bool found )
{
    if ( !found && !isAutoSearch ) {
        if ( backward ) {
            KMessageBox::information( this, i18n( "Beginning of message reached.\nPhrase '%1' could not be found." ,mLastSearchStr ) );
        } else {
            KMessageBox::information( this, i18n( "End of message reached.\nPhrase '%1' could not be found.", mLastSearchStr ) );
        }
    }
}

bool PlainTextEditFindBar::searchText( bool backward, bool isAutoSearch )
{
    QTextDocument::FindFlags searchOptions = mFindWidget->searchOptions();
    if ( backward )
        searchOptions |= QTextDocument::FindBackward;

    if ( isAutoSearch ) {
        QTextCursor cursor = mView->textCursor();
        cursor.setPosition( cursor.selectionStart() );
        mView->setTextCursor( cursor );
    } else if ( !mLastSearchStr.contains( mFindWidget->search()->text(), Qt::CaseSensitive )) {
        clearSelections();
    }

    mLastSearchStr = mFindWidget->search()->text();
    const bool found = mView->find( mLastSearchStr, searchOptions );

    mFindWidget->setFoundMatch( found );
    messageInfo( backward, isAutoSearch, found );
    return found;
}

void PlainTextEditFindBar::findNext()
{
    searchText( false, false );
}

void PlainTextEditFindBar::findPrev()
{
    searchText( true, false );
}

void PlainTextEditFindBar::slotUpdateSearchOptions()
{
    const QTextDocument::FindFlags searchOptions = mFindWidget->searchOptions();
    mLastSearchStr = mFindWidget->search()->text();
    const bool found = mView->find( mLastSearchStr, searchOptions );
    mFindWidget->setFoundMatch( found );
}

void PlainTextEditFindBar::clearSelections()
{
    mFindWidget->setFoundMatch( false );
}

void PlainTextEditFindBar::closeBar()
{
    // Make sure that all old searches are cleared
    mFindWidget->search()->setText( QString() );
    mReplaceWidget->replace()->setText( QString() );
    clearSelections();
    hide();
}

bool PlainTextEditFindBar::event(QEvent* e)
{
    // Close the bar when pressing Escape.
    // Not using a QShortcut for this because it could conflict with
    // window-global actions (e.g. Emil Sedgh binds Esc to "close tab").
    // With a shortcut override we can catch this before it gets to kactions.
    const bool shortCutOverride = (e->type() == QEvent::ShortcutOverride);
    if (shortCutOverride || e->type() == QEvent::KeyPress) {
        QKeyEvent* kev = static_cast<QKeyEvent* >(e);
        if (kev->key() == Qt::Key_Escape) {
            if ( shortCutOverride ) {
                e->accept();
                return true;
            }
            e->accept();
            closeBar();
            return true;
        }
        else if ( kev->key() == Qt::Key_Enter ||
                  kev->key() == Qt::Key_Return ) {
            e->accept();
            if ( shortCutOverride ) {
                return true;
            }
            if ( kev->modifiers() & Qt::ShiftModifier )
                findPrev();
            else if ( kev->modifiers() == Qt::NoModifier )
                findNext();
            return true;
        }
    }
    return QWidget::event(e);
}

void PlainTextEditFindBar::slotReplaceText()
{
    if (mView->textCursor().hasSelection()) {
        if (mView->textCursor().selectedText() == mFindWidget->search()->text()) {
            mView->textCursor().insertText(mReplaceWidget->replace()->text());
            //search next after replace text.
            searchText(false, false);
        }
    } else {
        searchText( false, false );
    }
}

void PlainTextEditFindBar::slotReplaceAllText()
{
    mView->setPlainText(mView->toPlainText().replace(mFindWidget->findRegExp(), mReplaceWidget->replace()->text()));
}

#include "plaintexteditfindbar.moc"
