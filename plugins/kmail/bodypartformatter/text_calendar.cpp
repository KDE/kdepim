/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#include <kdebug.h>

#include "interfaces/bodypartformatter.h"
#include "interfaces/bodypart.h"
#include "khtmlparthtmlwriter.h"

namespace {

class Formatter : public KMail::Interface::BodyPartFormatter
{
  public:
    Result format( KMail::Interface::BodyPart *bodyPart,
                   KMail::HtmlWriter *writer ) const
    {
      const QString iCalendar = bodyPart->asText();
      if ( iCalendar.isEmpty() ) return AsIcon;

      writer->begin( "" );

      QString html = "This plugin is supposed to handle iCalendar attachments."
                     "It needs to actually be implemented, though.";
      
      writer->write( html );

      writer->end();
    
      return Ok;
    }
};

class Plugin : public KMail::Interface::BodyPartFormatterPlugin
{
  public:
    const KMail::Interface::BodyPartFormatter *bodyPartFormatter( int idx ) const
    {
      return idx == 0 ? new Formatter() : 0 ;
    }

    const char *type( int idx ) const
    {
      return idx == 0 ? "text" : 0 ;
    }

    const char *subtype( int idx ) const
    {
      return idx == 0 ? "calendar" : 0 ;
    }

    const KMail::Interface::BodyPartURLHandler * urlHandler( int ) const { 
       return 0; 
    }
};

}

extern "C"
KMail::Interface::BodyPartFormatterPlugin *
libkmail_bodypartformatter_text_calendar_create_bodypart_formatter_plugin() {
  return new Plugin();
}
