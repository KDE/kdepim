/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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


#include "sieveactionaddflags.h"

#include <KLocalizedString>

using namespace KSieveUi;
SieveActionAddFlags::SieveActionAddFlags(QObject *parent)
    : SieveActionAbstractFlags(QLatin1String("addflag"), i18n("Add Flags"), parent)
{
}

SieveAction* SieveActionAddFlags::newAction()
{
    return new SieveActionAddFlags;
}

QString SieveActionAddFlags::flagsCode() const
{
    return QString::fromLatin1("addflag");
}

QString SieveActionAddFlags::help() const
{
    return i18n("Addflag is used to add flags to a list of [IMAP] flags.  It doesn't replace any previously set flags.  This means that multiple occurrences of addflag are treated additively.");
}

