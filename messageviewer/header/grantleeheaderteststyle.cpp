/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "grantleeheaderteststyle.h"

#include "header/grantleeheaderformatter.h"
#include "header/headerstrategy.h"

#include <kmime/kmime_message.h>

#include <QDebug>

using namespace MessageViewer;

namespace MessageViewer {

GrantleeHeaderTestStyle::GrantleeHeaderTestStyle()
    : HeaderStyle()
{
    mGrantleeFormatter = new GrantleeHeaderFormatter;
}

GrantleeHeaderTestStyle::~GrantleeHeaderTestStyle()
{
    delete mGrantleeFormatter;
}

QString GrantleeHeaderTestStyle::format( KMime::Message *message ) const
{
    if ( !message )
        return QString();
    return mGrantleeFormatter->toHtml(mExtraDisplay, mAbsolutePath, mMainFilename, this, message, isPrinting());
}

void GrantleeHeaderTestStyle::setAbsolutePath(const QString &path)
{
    mAbsolutePath = path;
}

void GrantleeHeaderTestStyle::setMainFilename(const QString &filename)
{
    mMainFilename = filename;
}

void GrantleeHeaderTestStyle::setExtraDisplayHeaders(const QStringList &extraDisplay)
{
    mExtraDisplay = extraDisplay;
}


}
