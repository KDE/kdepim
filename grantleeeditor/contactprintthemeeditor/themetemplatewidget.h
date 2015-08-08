/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef THEMETEMPLATEWIDGET_H
#define THEMETEMPLATEWIDGET_H

#include <QWidget>
#include "pimcommon/templatewidgets/templatelistwidget.h"

class ThemeTemplateListWidget : public PimCommon::TemplateListWidget
{
    Q_OBJECT
public:
    explicit ThemeTemplateListWidget(const QString &configName, QWidget *parent = Q_NULLPTR);
    ~ThemeTemplateListWidget();

    QVector<PimCommon::defaultTemplate> defaultTemplates() Q_DECL_OVERRIDE;
};

class ThemeTemplateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ThemeTemplateWidget(const QString &title, QWidget *parent = Q_NULLPTR);
    ~ThemeTemplateWidget();

Q_SIGNALS:
    void insertTemplate(const QString &);

private:
    ThemeTemplateListWidget *mListTemplate;
};

#endif // THEMETEMPLATEWIDGET_H
