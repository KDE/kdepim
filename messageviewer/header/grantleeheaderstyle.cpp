/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "grantleeheaderstyle.h"
#include "header/grantleeheaderformatter.h"
#include "header/headerstrategy.h"

#include <kmime/kmime_message.h>

using namespace MessageViewer;

namespace MessageViewer {

GrantleeHeaderStyle::GrantleeHeaderStyle()
    : HeaderStyle()
{
    mGrantleeFormatter = new GrantleeHeaderFormatter;
}

GrantleeHeaderStyle::~GrantleeHeaderStyle()
{
    delete mGrantleeFormatter;
}

QString GrantleeHeaderStyle::format( KMime::Message *message ) const {
    if ( !message )
        return QString();
    return mGrantleeFormatter->toHtml(theme(), isPrinting(), this, message);
}

}
