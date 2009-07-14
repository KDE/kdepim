/*******************************************************************************
**
** Filename   : util
** Created on : 03 April, 2005
** Copyright  : (c) 2005 Till Adam
** Email      : <adam@kde.org>
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   It is distributed in the hope that it will be useful, but
**   WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**   General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
**   In addition, as a special exception, the copyright holders give
**   permission to link the code of this program with any edition of
**   the Qt library by Trolltech AS, Norway (or with modified versions
**   of Qt that use the same license as Qt), and distribute linked
**   combinations including the two.  You must obey the GNU General
**   Public License in all respects for all of the code used other than
**   Qt.  If you modify this file, you may extend this exception to
**   your version of the file, but you are not obligated to do so.  If
**   you do not wish to do so, delete this exception statement from
**   your version.
**
*******************************************************************************/
#ifndef MAILVIEWERUTIL_H
#define MAILVIEWERUTIL_H

class KUrl;
class QWidget;
class QStringList;
class QString;

namespace MailViewer
{
    /**
     * The Util namespace contains a collection of helper functions use in
     * various places.
     */
namespace Util {

    // return true if we should proceed, false if we should abort
    bool checkOverwrite( const KUrl &url, QWidget *w );

    /**
     * Delegates opening a URL to the Max OSX mechanisms for that.
     * Returns false if it did nothing (such as on other platforms.
     */
    bool handleUrlOnMac( const KUrl& url );

    /**
    * Return a list of the supported encodings
    * @param usAscii if true, US-Ascii encoding will be prepended to the list.
    */
    QStringList supportedEncodings( bool usAscii );

    /**
    * Fixes an encoding received by a KDE function and returns the proper,
    * MIME-compilant encoding name instead.
    * @see encodingForName
    */
    QString fixEncoding( const QString &encoding );

    /**
    * Drop-in replacement for KCharsets::encodingForName(). The problem with
    * the KCharsets function is that it returns "human-readable" encoding names
    * like "ISO 8859-15" instead of valid encoding names like "ISO-8859-15".
    * This function fixes this by replacing whitespace with a hyphen.
    */
    QString encodingForName( const QString &descriptiveName );
}
}

#endif
