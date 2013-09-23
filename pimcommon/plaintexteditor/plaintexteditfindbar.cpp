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

#include <kicon.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <KColorScheme>

#include <QPlainTextEdit>
#include <QtCore/QTimer>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QToolButton>
#include <QEvent>
#include <QKeyEvent>

using namespace PimCommon;

PlainTextReplaceWidget::PlainTextReplaceWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    QLabel *label = new QLabel( i18nc( "Replace text", "Replace:" ), this );
    lay->addWidget( label );

    mReplace = new KLineEdit;
    mReplace->setClearButtonShown(true);
    lay->addWidget(mReplace);

    mReplaceBtn = new QPushButton( i18n( "Replace" ), this );
    connect( mReplaceBtn, SIGNAL(clicked()), this, SIGNAL(replaceText()) );
    lay->addWidget( mReplaceBtn );

    mReplaceAllBtn = new QPushButton( i18n( "Replace All" ), this );
    connect( mReplaceAllBtn, SIGNAL(clicked()), this, SIGNAL(replaceAllText()) );
    lay->addWidget( mReplaceAllBtn );

    setLayout(lay);
}

PlainTextReplaceWidget::~PlainTextReplaceWidget()
{

}

KLineEdit *PlainTextReplaceWidget::replace() const
{
    return mReplace;
}

PlainTextFindWidget::PlainTextFindWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    QLabel *label = new QLabel( i18nc( "Find text", "F&ind:" ), this );
    lay->addWidget( label );

    mSearch = new KLineEdit( this );
    mSearch->setToolTip( i18n( "Text to search for" ) );
    mSearch->setClearButtonShown( true );
    label->setBuddy( mSearch );
    lay->addWidget( mSearch );

    mFindNextBtn = new QPushButton( KIcon( QLatin1String("go-down-search") ), i18nc( "Find and go to the next search match", "Next" ), this );
    mFindNextBtn->setToolTip( i18n( "Jump to next match" ) );
    lay->addWidget( mFindNextBtn );
    mFindNextBtn->setEnabled( false );

    mFindPrevBtn = new QPushButton( KIcon( QLatin1String("go-up-search") ), i18nc( "Find and go to the previous search match", "Previous" ), this );
    mFindPrevBtn->setToolTip( i18n( "Jump to previous match" ) );
    lay->addWidget( mFindPrevBtn );
    mFindPrevBtn->setEnabled( false );

    QPushButton * optionsBtn = new QPushButton( this );
    optionsBtn->setText( i18n( "Options" ) );
    optionsBtn->setToolTip( i18n( "Modify search behavior" ) );
    QMenu *optionsMenu = new QMenu( optionsBtn );
    mCaseSensitiveAct = optionsMenu->addAction( i18n( "Case sensitive" ) );
    mCaseSensitiveAct->setCheckable( true );

    mWholeWordAct = optionsMenu->addAction( i18n( "Whole word" ) );
    mWholeWordAct->setCheckable( true );

    optionsBtn->setMenu( optionsMenu );
    lay->addWidget( optionsBtn );

    connect( mFindNextBtn, SIGNAL(clicked()), this, SIGNAL(findNext()) );
    connect( mFindPrevBtn, SIGNAL(clicked()), this, SIGNAL(findPrev()) );
    connect( mCaseSensitiveAct, SIGNAL(toggled(bool)), this, SIGNAL(updateSearchOptions()) );
    connect( mWholeWordAct, SIGNAL(toggled(bool)), this, SIGNAL(updateSearchOptions()) );
    connect( mSearch, SIGNAL(textChanged(QString)), this, SLOT(slotAutoSearch(QString)) );
    connect( mSearch, SIGNAL(clearButtonClicked()), this, SIGNAL(clearSearch()) );
    setLayout(lay);
}

PlainTextFindWidget::~PlainTextFindWidget()
{

}

void PlainTextFindWidget::setFoundMatch( bool match )
{
#ifndef QT_NO_STYLE_STYLESHEET
    QString styleSheet;

    if (! mSearch->text().isEmpty()) {
        KColorScheme::BackgroundRole bgColorScheme;

        if (match)
            bgColorScheme = KColorScheme::PositiveBackground;
        else
            bgColorScheme = KColorScheme::NegativeBackground;

        KStatefulBrush bgBrush(KColorScheme::View, bgColorScheme);

        styleSheet = QString::fromLatin1("QLineEdit{ background-color:%1 }")
                .arg(bgBrush.brush(mSearch).color().name());
    }

     mSearch->setStyleSheet(styleSheet);
#endif
}

void PlainTextFindWidget::slotAutoSearch(const QString &str)
{
    const bool isNotEmpty = ( !str.isEmpty() );
    mFindPrevBtn->setEnabled( isNotEmpty );
    mFindNextBtn->setEnabled( isNotEmpty );
    Q_EMIT autoSearch(str);
}

KLineEdit *PlainTextFindWidget::search() const
{
    return mSearch;
}

QTextDocument::FindFlags PlainTextFindWidget::searchOptions() const
{
    QTextDocument::FindFlags opt=0;
    if ( mCaseSensitiveAct->isChecked() )
        opt |= QTextDocument::FindCaseSensitively;
    if ( mWholeWordAct->isChecked() )
        opt |= QTextDocument::FindWholeWords;
    return opt;
}

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
    //TODO
}

#include "plaintexteditfindbar.moc"
