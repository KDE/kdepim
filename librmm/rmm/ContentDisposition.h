/* This file is part of the KDE project

   Copyright (C) 1999, 2000 Rik Hemsley <rik@kde.org>
             (C) 1999, 2000 Wilco Greven <j.w.greven@student.utwente.nl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef RMM_CONTENT_DISPOSITION_H
#define RMM_CONTENT_DISPOSITION_H

#include <qcstring.h>

#include <rmm/Enum.h>
#include <rmm/HeaderBody.h>
#include <rmm/Defines.h>
#include <rmm/Parameter.h>
#include <rmm/ParameterList.h>

namespace RMM {

/**
 * Two common ways of presenting multipart electronic messages are as a
 * main document with a list of separate attachments, and as a single
 * document with the various parts expanded (displayed) inline. The
 * display of an attachment is generally construed to require positive
 * action on the part of the recipient, while inline message components
 * are displayed automatically when the message is viewed. A mechanism
 * is needed to allow the sender to transmit this sort of presentational
 * information to the recipient; the Content-Disposition header provides
 * this mechanism, allowing each component of a message to be tagged
 * with an indication of its desired presentation semantics.
 * -- RFC 2183
 */
class ContentDisposition : public HeaderBody {

#include "rmm/ContentDisposition_generated.h"

    public:
        
        QCString            filename();
        void                setFilename(const QCString &);
        void                addParameter(Parameter & p);
        ParameterList &     parameterList();
        RMM::DispType       type();

    private:

        ParameterList       parameterList_;
        RMM::DispType       dispType_;
        QCString            filename_;
};

}

#endif
// vim:ts=4:sw=4:tw=78
