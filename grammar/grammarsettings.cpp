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

#include "grammarsettings.h"

#include <KConfig>
#include <KConfigGroup>

namespace Grammar {

class GrammarSettingsPrivate
{
public:
    GrammarSettingsPrivate()
    {
    }
    QString defaultLanguage;
};

GrammarSettings::GrammarSettings()
    : d(new GrammarSettingsPrivate)
{
}

GrammarSettings::~GrammarSettings()
{
    delete d;
}

void GrammarSettings::setDefaultLanguage(const QString &lang)
{
    d->defaultLanguage = lang;
}

QString GrammarSettings::defaultLanguage() const
{
    return d->defaultLanguage;
}

void GrammarSettings::saveSettings(KConfigGroup &group)
{
    group.writeEntry("DefaultLanguage", d->defaultLanguage);
}

void GrammarSettings::readSettings(const KConfigGroup &group)
{
    d->defaultLanguage = group.readEntry("DefaultLanguage");
}

}
