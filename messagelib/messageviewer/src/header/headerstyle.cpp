/*  -*- c++ -*-
    headerstyle.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

    Copyright (C) 2013-2015 Laurent Montel <montel@kde.org>

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

#include "headerstyle.h"
#include "messageviewer_debug.h"

#include <KLocalizedString>

using namespace MessageViewer;
// #####################

//
// HeaderStyle abstract base:
//

HeaderStyle::HeaderStyle()
    : mStrategy(0),
      mNodeHelper(0),
      mSourceObject(0),
      mPrinting(false),
      mTopLevel(true),
      mAllowAsync(false)
{
}

HeaderStyle::~HeaderStyle()
{

}

bool HeaderStyle::hasAttachmentQuickList() const
{
    return false;
}

void HeaderStyle::setMessagePath(const QString &path)
{
    mMessagePath = path;
}

QString HeaderStyle::messagePath() const
{
    return mMessagePath;
}

void HeaderStyle::setHeaderStrategy(const HeaderStrategy *strategy)
{
    mStrategy = strategy;
}

const HeaderStrategy *HeaderStyle::headerStrategy() const
{
    return mStrategy;
}

void HeaderStyle::setVCardName(const QString &vCardName)
{
    mVCardName = vCardName;
}

QString HeaderStyle::vCardName() const
{
    return mVCardName;
}

void HeaderStyle::setPrinting(bool printing)
{
    mPrinting = printing;
}

bool HeaderStyle::isPrinting() const
{
    return mPrinting;
}

void HeaderStyle::setTopLevel(bool topLevel)
{
    mTopLevel = topLevel;
}

bool HeaderStyle::isTopLevel() const
{
    return mTopLevel;
}

void HeaderStyle::setNodeHelper(NodeHelper *nodeHelper)
{
    mNodeHelper = nodeHelper;
}

NodeHelper *HeaderStyle::nodeHelper() const
{
    return mNodeHelper;
}

void HeaderStyle::setAllowAsync(bool allowAsync)
{
    mAllowAsync = allowAsync;
}

bool HeaderStyle::allowAsync() const
{
    return mAllowAsync;
}

void HeaderStyle::setSourceObject(QObject *sourceObject)
{
    mSourceObject = sourceObject;
}

QObject *HeaderStyle::sourceObject() const
{
    return mSourceObject;
}

void HeaderStyle::setMessageStatus(Akonadi::MessageStatus status)
{
    mMessageStatus = status;
}

Akonadi::MessageStatus HeaderStyle::messageStatus() const
{
    return mMessageStatus;
}

void HeaderStyle::setTheme(const GrantleeTheme::Theme &theme)
{
    mTheme = theme;
}

GrantleeTheme::Theme HeaderStyle::theme() const
{
    return mTheme;
}
