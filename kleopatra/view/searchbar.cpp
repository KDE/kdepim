/* -*- mode: c++; c-basic-offset:4 -*-
    view/searchbar.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "searchbar.h"

#include <kleo/keyfilter.h>
#include <kleo/keyfiltermanager.h>

#include <KLocalizedString>
#include <KLineEdit>

#include <QComboBox>
#include <QHBoxLayout>

#include <cassert>

using namespace Kleo;
using namespace boost;

class SearchBar::Private {
    friend class ::Kleo::SearchBar;
    SearchBar * const q;
public:
    explicit Private( SearchBar * qq );
    ~Private();

private:
    void slotKeyFilterChanged( int idx ) {
        emit q->keyFilterChanged( keyFilter( idx ) );
    }

    shared_ptr<KeyFilter> keyFilter( int idx ) const {
        const QModelIndex mi = KeyFilterManager::instance()->model()->index( idx, 0 );
        return KeyFilterManager::instance()->fromModelIndex( mi );
    }

    shared_ptr<KeyFilter> currentKeyFilter() const {
        return keyFilter( combo->currentIndex() );
    }

    QString currentKeyFilterID() const {
        if ( const shared_ptr<KeyFilter> f = currentKeyFilter() )
            return f->id();
        else
            return QString();
    }

private:
    KLineEdit * lineEdit;
    QComboBox * combo;
};

SearchBar::Private::Private( SearchBar * qq )
  : q( qq )
{
    QHBoxLayout * layout = new QHBoxLayout( q );
    layout->setMargin( 0 );
    lineEdit = new KLineEdit( q );
    lineEdit->setClearButtonShown( true );
    lineEdit->setPlaceholderText(i18n("Search..."));
    layout->addWidget( lineEdit, /*stretch=*/1 );
    combo = new QComboBox( q );
    layout->addWidget( combo );

    combo->setModel( KeyFilterManager::instance()->model() );

    KDAB_SET_OBJECT_NAME( layout );
    KDAB_SET_OBJECT_NAME( lineEdit );
    KDAB_SET_OBJECT_NAME( combo );

    connect( lineEdit, SIGNAL(textChanged(QString)),
             q, SIGNAL(stringFilterChanged(QString)) );
    connect( combo, SIGNAL(currentIndexChanged(int)),
             q, SLOT(slotKeyFilterChanged(int)) );
}

SearchBar::Private::~Private() {}

SearchBar::SearchBar( QWidget * parent, Qt::WindowFlags f )
    : QWidget( parent, f ), d( new Private( this ) )
{
    
}

SearchBar::~SearchBar() {}


void SearchBar::updateClickMessage(const QString &shortcutStr)
{
    d->lineEdit->setPlaceholderText(i18n("Search...<%1>", shortcutStr));
}

// slot
void SearchBar::setStringFilter( const QString & filter ) {
    d->lineEdit->setText( filter );
}

// slot
void SearchBar::setKeyFilter( const shared_ptr<KeyFilter> & kf ) {
    const QModelIndex idx = KeyFilterManager::instance()->toModelIndex( kf );
    if ( idx.isValid() )
        d->combo->setCurrentIndex( idx.row() );
    else
        d->combo->setCurrentIndex( 0 );
}

// slot
void SearchBar::setChangeStringFilterEnabled( bool on ) {
    d->lineEdit->setEnabled( on );
}

// slot
void SearchBar::setChangeKeyFilterEnabled( bool on ) {
    d->combo->setEnabled( on );
}

KLineEdit * SearchBar::lineEdit() const {
    return d->lineEdit;
}

#include "moc_searchbar.cpp"

