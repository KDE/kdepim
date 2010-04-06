/*  -*- c++ -*-
    headerstyle.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

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

#ifndef __MESSAGEVIEWER_HEADERSTYLE_H__
#define __MESSAGEVIEWER_HEADERSTYLE_H__

#include "messageviewer_export.h"

#include <KMime/Message>

class QByteArray;
class QString;
class KDateTime;

namespace MessageViewer {

class HeaderStrategy;
class NodeHelper;

/** This class encapsulates the visual appearance of message
    headers. Together with HeaderStrategy, which determines
    which of the headers present in the message be shown, it is
    responsible for the formatting of message headers.

    @short Encapsulates visual appearance of message headers.
    @author Marc Mutz <mutz@kde.org>
    @see HeaderStrategy
**/
class MESSAGEVIEWER_EXPORT HeaderStyle {
protected:
  HeaderStyle();
  virtual ~HeaderStyle();

public:
  //
  // Factory methods:
  //
  enum Type {
    Brief,
    Plain,
    Fancy,
    Enterprise
#ifdef KDEPIM_MOBILE_UI
    , Mobile
#endif
  };

  static const HeaderStyle * create( Type type );
  static const HeaderStyle * create( const QString & type );

  static const HeaderStyle * brief();
  static const HeaderStyle * plain();
  static const HeaderStyle * fancy();
  static const HeaderStyle * enterprise();
#ifdef KDEPIM_MOBILE_UI
  static const HeaderStyle * mobile();
#endif

  //
  // Methods for handling the styles:
  //
  virtual const char * name() const = 0;
  virtual const HeaderStyle * next() const = 0;
  virtual const HeaderStyle * prev() const = 0;

  //
  // HeaderStyle interface:
  //
  virtual QString format( KMime::Message::Ptr message,
        const HeaderStrategy * strategy,
        const QString & vCardName,
        bool printing = false, bool topLevel = false ) const = 0;

  static QString dateStr(const KDateTime &dateTime);
  static QByteArray dateShortStr(const KDateTime &dateTime);

};
}

#endif // __MESSAGEVIEWER_HEADERSTYLE_H__
