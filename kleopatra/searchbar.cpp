/* -*- mode: c++; c-basic-offset:4 -*-
    searchbar.cpp

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

#include "searchbar.h"

#include <QLabel>

#include <QComboBox>
#include <QLineEdit>
#include <KLocale>

#include <QHBoxLayout>

using boost::shared_ptr;

class SearchBar::State::Private {
public:
    QString searchString;
    //TODO add combo state
};

class SearchBar::Private {
    friend class ::SearchBar;
    SearchBar * const q;
public:
    explicit Private( SearchBar * qq );
    ~Private();

    QLineEdit * lineEdit;
    QComboBox * combo;
    mutable shared_ptr<SearchBar::State> state;
};

SearchBar::Private::Private( SearchBar * qq )
  : q( qq )
{
    QHBoxLayout * layout = new QHBoxLayout( q );
    QLabel * label = new QLabel;
    label->setText( i18n( "Find: " ) );
    layout->addWidget( label );
    lineEdit = new QLineEdit;
    connect( lineEdit, SIGNAL( textChanged( QString ) ),
             q, SIGNAL( textChanged( QString ) ) );
    label->setBuddy( lineEdit );
    layout->addWidget( lineEdit, /*stretch=*/1 );
    combo = new QComboBox;
    layout->addWidget( combo );
}

SearchBar::Private::~Private() {}

SearchBar::State::State() : d( new Private )
{}

SearchBar::State::~State() {}

SearchBar::SearchBar( QWidget * parent, Qt::WFlags f )
    : QWidget( parent, f ), d( new Private( this ) )
{
    
}

boost::shared_ptr<SearchBar::State> SearchBar::state() const
{
    if ( !d->state )
        d->state.reset( new State );
    d->state->d->searchString = d->lineEdit->text();
    return d->state;
}

void SearchBar::setState( shared_ptr<SearchBar::State> state )
{
    d->state = state;
    if ( !d->state )
        return;
    d->lineEdit->setText( d->state->d->searchString ); 
}

SearchBar::~SearchBar() {}

void SearchBar::setText( const QString& text )
{
    d->lineEdit->setText( text );
}
 
