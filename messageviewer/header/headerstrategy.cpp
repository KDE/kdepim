/*  -*- c++ -*-
    headerstrategy.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

    Copyright (c) 2013 Laurent Montel <montel@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
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




#include "headerstrategy.h"
#include "header/headerstrategy_p.h"

#include <qdebug.h>

//
// HeaderStrategy abstract base:
//
namespace MessageViewer {
HeaderStrategy::HeaderStrategy() {

}

HeaderStrategy::~HeaderStrategy() {

}

QStringList HeaderStrategy::headersToDisplay() const {
    return QStringList();
}

QStringList HeaderStrategy::headersToHide() const {
    return QStringList();
}

bool HeaderStrategy::showHeader( const QString & header ) const {
    if ( headersToDisplay().contains( header.toLower() ) ) return true;
    if ( headersToHide().contains( header.toLower() ) ) return false;
    return defaultPolicy() == Display;
}

HeaderStrategy * HeaderStrategy::create( Type type ) {
    switch ( type ) {
    case All:  return all();
    case Rich:   return rich();
    case Standard: return standard();
    case Brief:  return brief();
    case Custom:  return custom();
    case Grantlee:  return grantlee();
    }
    qCritical() << "Unknown header strategy ( type ==" << (int)type << ") requested!";
    return 0; // make compiler happy
}

HeaderStrategy * HeaderStrategy::create( const QString & type ) {
    const QString lowerType = type.toLower();
    if ( lowerType == QLatin1String( "all" ) )
        return all();
    else if ( lowerType == QLatin1String( "rich" ) )
        return HeaderStrategy::rich();
    //if ( lowerType == "standard" ) return standard(); // not needed, see below
    else if ( lowerType == QLatin1String( "brief" ) )
        return brief();
    else if ( lowerType == QLatin1String( "custom" ) )
        return custom();
    else if ( lowerType == QLatin1String( "grantlee" ) )
        return grantlee();
    // don't kFatal here, b/c the strings are user-provided
    // (KConfig), so fail gracefully to the default:
    return standard();
}

static HeaderStrategy * allStrategy = 0;
static HeaderStrategy * richStrategy = 0;
static HeaderStrategy * standardStrategy = 0;
static HeaderStrategy * briefStrategy = 0;
static HeaderStrategy * customStrategy = 0;
static HeaderStrategy * grantleeStrategy = 0;

HeaderStrategy * HeaderStrategy::all() {
    if ( !allStrategy )
        allStrategy = new MessageViewer::AllHeaderStrategy();
    return allStrategy;
}

HeaderStrategy * HeaderStrategy::rich() {
    if ( !richStrategy )
        richStrategy = new MessageViewer::RichHeaderStrategy();
    return richStrategy;
}

HeaderStrategy * HeaderStrategy::standard() {
    if ( !standardStrategy )
        standardStrategy = new MessageViewer::StandardHeaderStrategy();
    return standardStrategy;
}

HeaderStrategy * HeaderStrategy::brief() {
    if ( !briefStrategy )
        briefStrategy = new MessageViewer::BriefHeaderStrategy();
    return briefStrategy;
}

HeaderStrategy * HeaderStrategy::custom() {
    if ( !customStrategy )
        customStrategy = new MessageViewer::CustomHeaderStrategy();
    return customStrategy;
}

void HeaderStrategy::readConfig() {
    if(customStrategy) {
        static_cast<MessageViewer::CustomHeaderStrategy*>(customStrategy)->loadConfig();
    }
}

HeaderStrategy * HeaderStrategy::grantlee() {
    if ( !grantleeStrategy )
        grantleeStrategy = new MessageViewer::GrantleeHeaderStrategy();
    return grantleeStrategy;
}

}
