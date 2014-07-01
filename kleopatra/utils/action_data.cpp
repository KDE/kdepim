/* -*- mode: c++; c-basic-offset:4 -*-
    utils/action_data.cpp

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

#include "action_data.h"

#include <KToggleAction>
#include <KActionCollection>
#include <QAction>
#include <QIcon>
#include <QKeySequence>
QAction * Kleo::make_action_from_data( const action_data & ad, QObject * parent ) {

    QAction * const a = ad.toggle ? new KToggleAction( parent ) : new QAction( parent ) ;
    a->setObjectName( QLatin1String(ad.name) );
    a->setText( ad.text );
    if ( !ad.tooltip.isEmpty() )
        a->setToolTip( ad.tooltip );
    if ( ad.icon )
        a->setIcon( QIcon::fromTheme( QLatin1String(ad.icon) ) );
    if ( ad.receiver && ad.slot ) {
        if ( ad.toggle )
            QObject::connect( a, SIGNAL(toggled(bool)), ad.receiver, ad.slot );
        else
            QObject::connect( a, SIGNAL(triggered()), ad.receiver, ad.slot );
    }
    if ( !ad.shortcut.isEmpty() )
        a->setShortcut( QKeySequence( ad.shortcut ) );
    a->setEnabled( ad.enabled );
    return a;
}

void Kleo::make_actions_from_data( const action_data * ads, unsigned int size, QObject * parent ) {
    for ( unsigned int i = 0 ; i < size ; ++i )
        make_action_from_data( ads[i], parent );
}

void Kleo::make_actions_from_data( const action_data * ads, unsigned int size, KActionCollection * coll ) {
    for ( unsigned int i = 0 ; i < size ; ++i )
        coll->addAction( QLatin1String(ads[i].name), make_action_from_data( ads[i], coll ) );
}
