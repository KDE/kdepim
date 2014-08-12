/*  -*- c++ -*-
    headerstyle.cpp

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

#include "headerstyle.h"
#include "header/briefheaderstyle.h"
#include "header/grantleeheaderstyle.h"
#include "header/customheaderstyle.h"
#include "header/plainheaderstyle.h"
#include "header/mobileheaderstyle.h"
#include "header/entrepriseheaderstyle.h"
#include "header/fancyheaderstyle.h"

#include <QDebug>

#include <KLocalizedString>

#include "messagecore/settings/globalsettings.h"

using namespace MessageCore;


namespace MessageViewer {
// #####################

//
// HeaderStyle abstract base:
//

HeaderStyle::HeaderStyle()
    : mStrategy( 0 ),
      mPrinting( false ),
      mTopLevel( true ),
      mNodeHelper( 0 ),
      mAllowAsync( false ),
      mSourceObject( 0 )
{
}

HeaderStyle::~HeaderStyle() {

}

bool HeaderStyle::hasAttachmentQuickList() const
{
    return false;
}

HeaderStyle * HeaderStyle::create( Type type ) {
    switch ( type ) {
    case Brief:  return brief();
    case Plain:  return plain();
    case Fancy:   return fancy();
    case Enterprise: return enterprise();
    case Mobile: return mobile();
    case MobileExtended: return mobileExtended();
    case Custom: return custom();
    case Grantlee: return grantlee();
    }
    qCritical() << "Unknown header style ( type ==" << (int)type << ") requested!";
    return 0; // make compiler happy
}

HeaderStyle * HeaderStyle::create( const QString & type ) {
    const QString lowerType = type.toLower();
    if ( lowerType == QLatin1String("brief") ) return brief();
    else if ( lowerType == QLatin1String("plain") )  return plain();
    else if ( lowerType == QLatin1String("enterprise") )  return enterprise();
    else if ( lowerType == QLatin1String("mobile") )  return mobile();
    else if ( lowerType == QLatin1String("mobileExtended") )  return mobileExtended();
    else if ( lowerType == QLatin1String("custom") )  return custom();
    else if ( lowerType == QLatin1String("grantlee")) return grantlee();
    //if ( lowerType == "fancy" ) return fancy(); // not needed, see below
    // don't kFatal here, b/c the strings are user-provided
    // (KConfig), so fail gracefully to the default:
    return fancy();
}

HeaderStyle * briefStyle = 0;
HeaderStyle * plainStyle = 0;
HeaderStyle * fancyStyle = 0;
HeaderStyle * enterpriseStyle = 0;
HeaderStyle * mobileStyle = 0;
HeaderStyle * mobileExtendedStyle = 0;
HeaderStyle * customStyle = 0;
HeaderStyle * grantleeStyle = 0;

HeaderStyle * HeaderStyle::brief() {
    if ( !briefStyle )
        briefStyle = new BriefHeaderStyle();
    return briefStyle;
}

HeaderStyle * HeaderStyle::plain() {
    if ( !plainStyle )
        plainStyle = new MessageViewer::PlainHeaderStyle();
    return plainStyle;
}

HeaderStyle * HeaderStyle::fancy() {
    if ( !fancyStyle )
        fancyStyle = new MessageViewer::FancyHeaderStyle();
    return fancyStyle;
}

HeaderStyle * HeaderStyle::enterprise() {
    if ( !enterpriseStyle )
        enterpriseStyle = new MessageViewer::EnterpriseHeaderStyle();
    return enterpriseStyle;
}

HeaderStyle * HeaderStyle::mobile() {
    if ( !mobileStyle )
        mobileStyle = new MessageViewer::MobileHeaderStyle();
    return mobileStyle;
}

HeaderStyle * HeaderStyle::mobileExtended() {
    if ( !mobileExtendedStyle )
        mobileExtendedStyle = new MessageViewer::MobileExtendedHeaderStyle;
    return mobileExtendedStyle;
}

HeaderStyle * HeaderStyle::custom() {
    if ( !customStyle )
        customStyle = new MessageViewer::CustomHeaderStyle;
    return customStyle;
}

HeaderStyle * HeaderStyle::grantlee() {
    if ( !grantleeStyle )
        grantleeStyle = new MessageViewer::GrantleeHeaderStyle;
    return grantleeStyle;
}

}
