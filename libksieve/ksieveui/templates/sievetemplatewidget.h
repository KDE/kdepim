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

#ifndef SIEVETEMPLATEWIDGET_H
#define SIEVETEMPLATEWIDGET_H

#include "pimcommon/templatewidgets/templatelistwidget.h"

namespace KSieveUi {
class SieveTemplateListWidget : public PimCommon::TemplateListWidget
{
    Q_OBJECT
public:
    explicit SieveTemplateListWidget(const QString &configName, QWidget *parent = 0);
    ~SieveTemplateListWidget();

    QList<PimCommon::defaultTemplate> defaultTemplates();
    bool addNewTemplate(QString &templateName, QString &templateScript);
    bool modifyTemplate(QString &templateName, QString &templateScript, bool defaultTemplate);
};

class SieveTemplateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveTemplateWidget(const QString &title, QWidget *parent = 0);
    ~SieveTemplateWidget();

Q_SIGNALS:
    void insertTemplate(const QString &);

private:
    SieveTemplateListWidget *mListTemplate;
};
}

#endif // SIEVETEMPLATEWIDGET_H
