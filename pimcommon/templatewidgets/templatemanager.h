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

#ifndef TEMPLATEMANAGER_H
#define TEMPLATEMANAGER_H

#include "pimcommon_export.h"
#include <QObject>
#include <QStringList>
#include <QDebug>


class KDirWatch;
namespace PimCommon {
class TemplateListWidget;
struct TemplateInfo {
    QString name;
    QString script;
    bool isValid() const {
        return (!name.isEmpty() && !script.isEmpty());
    }
    void debug() const {
        qDebug()<<" name :"<<name<<" script :"<<script;
    }
};

class PIMCOMMON_EXPORT TemplateManager : public QObject
{
    Q_OBJECT
public:
    explicit TemplateManager(const QString &relativeTemplateDir, PimCommon::TemplateListWidget *sieveTemplateWidget);
    ~TemplateManager();

private Q_SLOTS:
    void slotDirectoryChanged();

private:
    void loadTemplates();
    void initTemplatesDirectories(const QString &templatesRelativePath);
    TemplateInfo loadTemplate(const QString &themePath, const QString &defaultDesktopFileName);

    QStringList mTemplatesDirectories;

    PimCommon::TemplateListWidget *mTemplateListWidget;
    KDirWatch *mDirWatch;
};
}

#endif // TEMPLATEMANAGER_H
