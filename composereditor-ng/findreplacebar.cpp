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
#include <KColorScheme>
#include <KMessageBox>

#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <KLineEdit>
#include <QPushButton>
#include <QAction>
#include <QMenu>
#include <QEvent>
#include <QKeyEvent>
#include <QTimer>

namespace ComposerEditorNG {

class FindReplaceBarPrivate
{
public:
    FindReplaceBarPrivate(FindReplaceBar *qq, KWebView *view)
        :q(qq), webView(view)
    {

    }
    void _k_closeBar();
    void _k_slotHighlightAllChanged(bool highLight);
    void _k_slotCaseSensitivityChanged(bool sensitivity);
    void _k_slotClearSearch();
    void _k_slotAutoSearch(const QString&);
    void _k_slotSearchText(bool backward = false, bool isAutoSearch = true);
    void _k_slotFindNext();
    void _k_slotFindPrev();


    void clearSelections();
    void setFoundMatch( bool match );
    void searchText( bool backward, bool isAutoSearch );
    void messageInfo( bool backward, bool isAutoSearch, bool found );

    FindReplaceBar *q;
    QString mPositiveBackground;
    QString mNegativeBackground;
    QString mLastSearchStr;

    KLineEdit *search;
    QAction *caseSensitiveAct;
    QAction *highlightAll;

    QPushButton *findPreviousButton;
    QPushButton *findNextButton;
    QMenu *optionsMenu;
    KWebView *webView;
};

void FindReplaceBarPrivate::_k_slotHighlightAllChanged(bool highLight)
{
    bool found = false;
    if ( highLight ) {
        QWebPage::FindFlags searchOptions = QWebPage::FindWrapsAroundDocument;
        if ( caseSensitiveAct->isChecked() )
            searchOptions |= QWebPage::FindCaseSensitively;
        searchOptions |= QWebPage::HighlightAllOccurrences;
        found = webView->findText(mLastSearchStr, searchOptions);
    }
    else
        found = webView->findText(QString(), QWebPage::HighlightAllOccurrences);
    setFoundMatch( found );

}

void FindReplaceBarPrivate::_k_closeBar()
{
    // Make sure that all old searches are cleared
    search->setText( QString() );
    clearSelections();
    q->hide();
}

void FindReplaceBarPrivate::_k_slotClearSearch()
{
    clearSelections();
}

void FindReplaceBarPrivate::clearSelections()
{
    setFoundMatch( false );
    webView->findText(QString(), QWebPage::HighlightAllOccurrences);
}

void FindReplaceBarPrivate::setFoundMatch( bool match )
{
#ifndef QT_NO_STYLE_STYLESHEET
    QString styleSheet;

    if (!search->text().isEmpty()) {
        if(mNegativeBackground.isEmpty()) {
            KStatefulBrush bgBrush(KColorScheme::View, KColorScheme::PositiveBackground);
            mPositiveBackground = QString::fromLatin1("QLineEdit{ background-color:%1 }").arg(bgBrush.brush(search).color().name());
            bgBrush = KStatefulBrush(KColorScheme::View, KColorScheme::NegativeBackground);
            mNegativeBackground = QString::fromLatin1("QLineEdit{ background-color:%1 }").arg(bgBrush.brush(search).color().name());
        }
        if (match)
            styleSheet = mPositiveBackground;
        else
            styleSheet = mNegativeBackground;
    }
    search->setStyleSheet(styleSheet);
#endif
}

void FindReplaceBarPrivate::_k_slotCaseSensitivityChanged(bool sensitivity)
{
    QWebPage::FindFlags searchOptions = QWebPage::FindWrapsAroundDocument;
    if ( sensitivity ) {
        searchOptions |= QWebPage::FindCaseSensitively;
        webView->findText(QString(), QWebPage::HighlightAllOccurrences); //Clear an existing highligh
    }
    if ( highlightAll->isChecked() )
        searchOptions |= QWebPage::HighlightAllOccurrences;
    const bool found = webView->findText(mLastSearchStr, searchOptions);
    setFoundMatch( found );
}

void FindReplaceBarPrivate::_k_slotAutoSearch(const QString& str)
{
    const bool isNotEmpty = ( !str.isEmpty() );
    findPreviousButton->setEnabled( isNotEmpty );
    findNextButton->setEnabled( isNotEmpty );
    if ( isNotEmpty )
        QTimer::singleShot( 0, q, SLOT(_k_slotSearchText()) );
    else
        clearSelections();
}

void FindReplaceBarPrivate::_k_slotSearchText(bool backward, bool isAutoSearch)
{
    searchText( backward, isAutoSearch );
}

void FindReplaceBarPrivate::searchText( bool backward, bool isAutoSearch )
{
    QWebPage::FindFlags searchOptions = QWebPage::FindWrapsAroundDocument;

    if ( backward )
        searchOptions |= QWebPage::FindBackward;
    if ( caseSensitiveAct->isChecked() )
        searchOptions |= QWebPage::FindCaseSensitively;
    if ( highlightAll->isChecked() )
        searchOptions |= QWebPage::HighlightAllOccurrences;

    const QString searchWord( search->text() );
    if( !isAutoSearch && !mLastSearchStr.contains( searchWord, Qt::CaseSensitive ) )
    {
        clearSelections();
    }
    webView->findText(QString(), QWebPage::HighlightAllOccurrences); //Clear an existing highligh

    mLastSearchStr = searchWord;
    const bool found = webView->findText( mLastSearchStr, searchOptions );

    setFoundMatch( found );
    messageInfo( backward, isAutoSearch, found );
}

void FindReplaceBarPrivate::messageInfo( bool backward, bool isAutoSearch, bool found )
{
    if ( !found && !isAutoSearch ) {
        if ( backward ) {
            KMessageBox::information( q, i18n( "Beginning of message reached.\nPhrase '%1' could not be found." ,mLastSearchStr ) );
        } else {
            KMessageBox::information( q, i18n( "End of message reached.\nPhrase '%1' could not be found.", mLastSearchStr ) );
        }
    }
}

void FindReplaceBarPrivate::_k_slotFindNext()
{
    searchText( false, false );
}

void FindReplaceBarPrivate::_k_slotFindPrev()
{
    searchText( true, false );
}


FindReplaceBar::FindReplaceBar(KWebView *parent)
    : QWidget(parent), d(new FindReplaceBarPrivate(this, parent))
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

    d->highlightAll = d->optionsMenu->addAction( i18n( "Highlight all matches" ) );
    d->highlightAll->setCheckable( true );
    connect( d->highlightAll, SIGNAL(toggled(bool)), this, SLOT(_k_slotHighlightAllChanged(bool)) );

    optionsBtn->setMenu( d->optionsMenu );
    lay->addWidget( optionsBtn );

    connect( closeBtn, SIGNAL(clicked()), this, SLOT(_k_closeBar()) );
    connect( d->caseSensitiveAct, SIGNAL(toggled(bool)), this, SLOT(_k_slotCaseSensitivityChanged(bool)) );
    connect( d->search, SIGNAL(clearButtonClicked()), this, SLOT(_k_slotClearSearch()) );
    connect( d->search, SIGNAL(textChanged(QString)), this, SLOT(_k_slotAutoSearch(QString)) );
    connect( d->findNextButton, SIGNAL(clicked()), this, SLOT(_k_slotFindNext()) );
    connect( d->findPreviousButton, SIGNAL(clicked()), this, SLOT(_k_slotFindPrev()) );
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    hide();

}

FindReplaceBar::~FindReplaceBar()
{
    delete d;
}

bool FindReplaceBar::event(QEvent* e)
{
    // Close the bar when pressing Escape.
    // Not using a QShortcut for this because it could conflict with
    // window-global actions (e.g. Emil Sedgh binds Esc to "close tab").
    // With a shortcut override we can catch this before it gets to kactions.
    const bool shortCutOverride = (e->type() == QEvent::ShortcutOverride);
    if ( shortCutOverride || e->type() == QEvent::KeyPress ) {
        QKeyEvent* kev = static_cast<QKeyEvent* >(e);
        if (kev->key() == Qt::Key_Escape) {
            if( shortCutOverride ) {
                e->accept();
                return true;
            }
            e->accept();
            d->_k_closeBar();
            return true;
        }
        else if ( kev->key() == Qt::Key_Enter ||
                  kev->key() == Qt::Key_Return ) {
            e->accept();
            if( shortCutOverride ) {
                return true;
            }
            if ( kev->modifiers() & Qt::ShiftModifier )
                d->_k_slotFindPrev();
            else if ( kev->modifiers() == Qt::NoModifier )
                d->_k_slotFindNext();
            return true;
        }
    }
    return QWidget::event(e);
}

}
#include "findreplacebar.moc"
