/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "invalidfilterinfo.h"
using namespace MailCommon;

InvalidFilterInfo::InvalidFilterInfo()
{

}

InvalidFilterInfo::InvalidFilterInfo(const QString &name, const QString &information)
    : mName(name),
      mInformation(information)
{

}

QString InvalidFilterInfo::information() const
{
    return mInformation;
}

QString InvalidFilterInfo::name() const
{
    return mName;
}

void InvalidFilterInfo::setName(const QString &name)
{
    mName = name;
}

void InvalidFilterInfo::setInformation(const QString &information)
{
    mInformation = information;
}

bool InvalidFilterInfo::operator ==(const InvalidFilterInfo &other) const
{
    return (mName == other.name()) && (mInformation == other.information());
}

