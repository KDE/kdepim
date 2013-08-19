/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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

#ifndef ClawsMailSettings_H
#define ClawsMailSettings_H

#include "sylpheed/sylpheedsettings.h"

class ImportWizard;
class KConfigGroup;

class ClawsMailSettings : public SylpheedSettings
{
public:
    explicit ClawsMailSettings(ImportWizard *parent);
    ~ClawsMailSettings();

    void importSettings(const QString& filename, const QString& path);

protected:
    //Reimplement from sylpheed
    void readSettingsColor(const KConfigGroup& group);
    void readTemplateFormat(const KConfigGroup& group);
    void readGlobalSettings(const KConfigGroup& group);
    void readTagColor(const KConfigGroup& group);

private:
    QString writeColor(const QColor& col);
};

#endif // ClawsMailSettings_H
