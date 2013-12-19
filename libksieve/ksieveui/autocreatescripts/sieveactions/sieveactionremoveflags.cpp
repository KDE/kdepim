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

#include "sieveactionremoveflags.h"
#include "editor/sieveeditorutil.h"
#include "pimcommon/widgets/minimumcombobox.h"

#include <KLocalizedString>

using namespace KSieveUi;
SieveActionRemoveFlags::SieveActionRemoveFlags(QObject *parent)
    : SieveActionAbstractFlags(QLatin1String("removeflag"), i18n("Remove Flags"), parent)
{
}

SieveAction* SieveActionRemoveFlags::newAction()
{
    return new SieveActionRemoveFlags;
}

QString SieveActionRemoveFlags::flagsCode() const
{
    return QString::fromLatin1("removeflag");
}

QString SieveActionRemoveFlags::help() const
{
    return i18n("Removeflag is used to remove flags from a list of [IMAP] flags. Removeflag clears flags previously set by \"set\"/\"addflag\". Calling removeflag with a flag that wasn't set before is not an error and is ignored.");
}

QString SieveActionRemoveFlags::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

