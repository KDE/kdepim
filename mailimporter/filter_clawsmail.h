/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#ifndef MAILIMPORTER_FILTER_CLAWSMAILS_HXX
#define MAILIMPORTER_FILTER_CLAWSMAILS_HXX

#include <QHash>

#include "filter_sylpheed.h"
/**
 * Imports Sylpheed mail folder with maildir format recursively, recreating the folder structure.
 * @author Danny Kukawka
 */
namespace MailImporter {
class MAILIMPORTER_EXPORT FilterClawsMail : public FilterSylpheed
{

public:
    explicit FilterClawsMail();
    ~FilterClawsMail();

    static QString defaultSettingsPath();

    /* return local mail dir from folderlist.xml*/
    virtual QString localMailDirPath();
    virtual bool excludeFile(const QString& file);
    virtual QString defaultInstallFolder() const;
    virtual QString markFile() const;
};
}

#endif
