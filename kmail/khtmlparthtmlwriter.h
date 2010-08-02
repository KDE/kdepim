/*  -*- c++ -*-
    khtmlparthtmlwriter.h

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

#ifndef __KMAIL_KHTMLPARTHTMLWRITER_H__
#define __KMAIL_KHTMLPARTHTMLWRITER_H__

#include "interfaces/htmlwriter.h"
#include <tqobject.h>

#include <tqstringlist.h>
#include <tqtimer.h>

class TQString;
class KHTMLPart;

namespace KMail {

  class KHtmlPartHtmlWriter : public TQObject, public HtmlWriter {
    Q_OBJECT
  public:
    // Key is Content-Id, value is URL
    typedef TQMap<TQString, TQString> EmbeddedPartMap;
    KHtmlPartHtmlWriter( KHTMLPart * part,
			 TQObject * parent=0, const char * name = 0 );
    virtual ~KHtmlPartHtmlWriter();

    void begin( const TQString & cssDefs );
    void end();
    void reset();
    void write( const TQString & str );
    void queue( const TQString & str );
    void flush();
    void embedPart( const TQCString & contentId, const TQString & url );

  private slots:
    void slotWriteNextHtmlChunk();

  private:
    void resolveCidUrls();

  private:
    KHTMLPart * mHtmlPart;
    TQStringList mHtmlQueue;
    TQTimer mHtmlTimer;
    enum State {
      Begun,
      Queued,
      Ended
    } mState;
    EmbeddedPartMap mEmbeddedPartMap;
  };

} // namespace KMail

#endif // __KMAIL_KHTMLPARTHTMLWRITER_H__
