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

#include "sieveactionkeep.h"
#include <KLocale>

using namespace KSieveUi;
SieveActionKeep::SieveActionKeep(QObject *parent)
    : SieveAction(QLatin1String("keep"), i18n("Keep"), parent)
{
}

SieveAction* SieveActionKeep::newAction()
{
    return new SieveActionKeep;
}

QString SieveActionKeep::code(QWidget *) const
{
    return QLatin1String("keep;");
}

QString SieveActionKeep::help() const
{
    return i18n("The \"keep\" action is whatever action is taken in lieu of all other actions, if no filtering happens at all; generally, this simply means to file the message into the user's main mailbox.");
}

#include "sieveactionkeep.moc"
