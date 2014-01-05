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

#include "sieveactionstop.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>

using namespace KSieveUi;

SieveActionStop::SieveActionStop(QObject *parent)
    : SieveAction(QLatin1String("stop"), i18n("Stop"), parent)
{
}

SieveAction* SieveActionStop::newAction()
{
    return new SieveActionStop;
}

QString SieveActionStop::code(QWidget *) const
{
    return QLatin1String("stop;");
}

QString SieveActionStop::help() const
{
    return i18n("The \"stop\" action ends all processing.  If the implicit keep has not been cancelled, then it is taken.");
}

QString SieveActionStop::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}
