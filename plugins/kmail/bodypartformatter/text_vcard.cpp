/*  -*- mode: C++; c-file-style: "gnu" -*-
    text_vcard.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2004 Till Adam <adam@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#include <qurl.h>

#include <kglobal.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kglobalsettings.h>
#include <kiconloader.h>

#include "interfaces/bodypartformatter.h"
#include "interfaces/bodypart.h"
#include "khtmlparthtmlwriter.h"

#include <kabc/vcardconverter.h>
#include <kabc/addressee.h>
using KABC::VCardConverter;
using KABC::Addressee;

#include "addresseeview.h"
using KPIM::AddresseeView;

namespace {

  class Formatter : public KMail::Interface::BodyPartFormatter {
  public:
    Result format( KMail::Interface::BodyPart *bodyPart, KMail::HtmlWriter *writer ) const { 

       VCardConverter vcc;
       const QString vCard = bodyPart->asText();
       if ( vCard.isEmpty() ) return AsIcon;
       Addressee::List al = vcc.parseVCards(  vCard );
       if ( al.empty() ) return AsIcon;
       QValueListIterator<KABC::Addressee> it = al.begin();

       for ( ; it != al.end(); ++it ) {
          KABC::Addressee a = (*it);
          if ( a.isEmpty() ) return AsIcon;

          QString contact = AddresseeView::vCardAsHTML( a );
          writer->queue( contact );
       }

       return Ok;
    }
  };

  class Plugin : public KMail::Interface::BodyPartFormatterPlugin {
  public:
    const KMail::Interface::BodyPartFormatter * bodyPartFormatter( int idx ) const {
      return idx == 0 ? new Formatter() : 0 ;
    }
    const char * type( int idx ) const {
      return idx == 0 ? "text" : 0 ;
    }
    const char * subtype( int idx ) const {
      return idx == 0 ? "x-vcard" : 0 ;
    }

    const KMail::Interface::BodyPartURLHandler * urlHandler( int ) const { 
       return 0; 
    }
  };

}

extern "C"
KMail::Interface::BodyPartFormatterPlugin *
libkmail_bodypartformatter_text_vcard_create_bodypart_formatter_plugin() {
  return new Plugin();
}

