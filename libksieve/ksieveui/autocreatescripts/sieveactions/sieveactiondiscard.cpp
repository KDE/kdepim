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

#include "sieveactiondiscard.h"
#include "editor/sieveeditorutil.h"
#include <KLocalizedString>

using namespace KSieveUi;
SieveActionDiscard::SieveActionDiscard(QObject *parent)
    : SieveAction(QLatin1String("discard"), i18n("Discard"), parent)
{
}

SieveAction *SieveActionDiscard::newAction()
{
    return new SieveActionDiscard;
}

QString SieveActionDiscard::code(QWidget *) const
{
    return QLatin1String("discard;");
}

QString SieveActionDiscard::help() const
{
    return i18n("Discard is used to silently throw away the message.");
}

QString SieveActionDiscard::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}
