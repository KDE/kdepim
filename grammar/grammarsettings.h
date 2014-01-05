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

#ifndef GRAMMARSETTINGS_H
#define GRAMMARSETTINGS_H

#include <QString>
class KConfigGroup;

namespace Grammar {
class GrammarSettingsPrivate;
class GrammarSettings
{
public:
    GrammarSettings();
    ~GrammarSettings();

    void setDefaultLanguage(const QString &lang);
    QString defaultLanguage() const;

    void saveSettings(KConfigGroup &group);
    void readSettings(const KConfigGroup &group);

private:
    friend class GrammarSettingsPrivate;
    GrammarSettingsPrivate * const d;
};
}

#endif // GRAMMARSETTINGS_H
