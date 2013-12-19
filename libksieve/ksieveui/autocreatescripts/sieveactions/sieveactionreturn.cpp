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

#include "sieveactionreturn.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>


using namespace KSieveUi;
SieveActionReturn::SieveActionReturn(QObject *parent)
    : SieveAction(QLatin1String("return"), i18n("Return"), parent)
{
}

SieveAction* SieveActionReturn::newAction()
{
    return new SieveActionReturn;
}

QString SieveActionReturn::code(QWidget *) const
{
    return QLatin1String("return;");
}

QString SieveActionReturn::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}

QString SieveActionReturn::help() const
{
    return i18n("The \"return\" command stops processing of the immediately included script only and returns processing control to the script that includes it.");
}

QStringList SieveActionReturn::needRequires(QWidget */*parent*/) const
{
    return QStringList() << QLatin1String("include");
}

bool SieveActionReturn::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionReturn::serverNeedsCapability() const
{
    return QLatin1String("include");
}


