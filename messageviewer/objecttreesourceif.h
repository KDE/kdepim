/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef MAILVIEWER_OBJECTTREESOURCE_IF_H
#define MAILVIEWER_OBJECTTREESOURCE_IF_H

#include "messageviewer_export.h"
#include "util.h"

namespace KMime {
  class Message;
}

class QObject;
class QTextCodec;

namespace MessageViewer {
  class HtmlWriter;
  class CSSHelper;
  class AttachmentStrategy;
}

namespace MessageViewer {
/** @author Andras Mantia <amantia@kdab.net> */

/** Interface for object tree sources.*/
class MESSAGEVIEWER_EXPORT ObjectTreeSourceIf  {

public:

    ObjectTreeSourceIf() {};
    virtual ~ObjectTreeSourceIf() {};

    /**
      * Sets the type of mail that is currently displayed. Applications can display this
      * information to the user, for example KMail displays a HTML status bar.
      * Note: This is not called when the mode is "Normal".
      */
    virtual void setHtmlMode( Util::HtmlMode mode ) = 0;

    /** Return true if the mail should be parsed as a html mail */
    virtual bool htmlMail() = 0;

    /** Return true if an encrypted mail should be decrypted */
    
    virtual bool decryptMessage() = 0;

    /** Return true if external sources should be loaded in a html mail */
    virtual bool htmlLoadExternal() = 0;

    /** Return true to include the signature detailes in the generated html */
    virtual bool showSignatureDetails() = 0;

    virtual int levelQuote() = 0;

    /** The override codec that should be used for the mail */
    virtual const QTextCodec * overrideCodec() = 0;
    
    virtual QString createMessageHeader( KMime::Message* message) = 0;

    /** Disable drag and drop in the sourceObject */
    virtual void emitNoDrag() = 0;

    /** Return the wanted attachment startegy */
    virtual const AttachmentStrategy * attachmentStrategy() = 0;

    /** Return the html write object */
    virtual HtmlWriter * htmlWriter() = 0;

    /** Return the css helper object */
    virtual CSSHelper* cssHelper() = 0;

    /** The source object behind the interface. */    
    virtual QObject *sourceObject() = 0;
};
}

#endif
