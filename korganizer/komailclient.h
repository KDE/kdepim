/*
    This file is part of KOrganizer.
    Copyright (c) 1998 Barry D Benowitz
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOMAILCLIENT_H
#define KOMAILCLIENT_H

#include <tqstring.h>

class KURL;
namespace KCal {
class IncidenceBase;
}
using namespace KCal;

class KOMailClient
{
  public:
    KOMailClient();
    virtual ~KOMailClient();

    bool mailAttendees(IncidenceBase *,const TQString &attachment=TQString::null);
    bool mailOrganizer(IncidenceBase *,const TQString &attachment=TQString::null, const TQString &sub = TQString::null);
    bool mailTo(IncidenceBase *,const TQString &recipients,const TQString &attachment=TQString::null);

  protected:
    /** Send mail with specified from, to and subject field and body as text. If
     * bcc is set, send a blind carbon copy to the sender from */
    bool send(const TQString &from,const TQString &to,const TQString &cc,
              const TQString &subject,const TQString &body,bool bcc=false,
              const TQString &attachment=TQString::null);

    int kMailOpenComposer(const TQString& to, const TQString& cc,
                          const TQString& bcc, const TQString& subject,
                          const TQString& body, int hidden,
                          const TQString& attachName, const TQCString& attachCte,
                          const TQCString& attachData,
                          const TQCString& attachType,
                          const TQCString& attachSubType,
                          const TQCString& attachParamAttr,
                          const TQString& attachParamValue,
                          const TQCString& attachContDisp,
                          const TQCString& attachCharset,
                          uint identity);
    int kMailOpenComposer(const TQString& arg0,const TQString& arg1,
                          const TQString& arg2,const TQString& arg3,
                          const TQString& arg4,int arg5,const KURL& arg6);
};

#endif
