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

#include "themetemplatewidget.h"
#include "themedefaulttemplate.h"

#include <KLocalizedString>

#include <QVBoxLayout>
#include <QLabel>

ThemeTemplateListWidget::ThemeTemplateListWidget(const QString &configName, QWidget *parent)
    : PimCommon::TemplateListWidget(configName, parent)
{
    loadTemplates();
}

ThemeTemplateListWidget::~ThemeTemplateListWidget()
{
}

QList<PimCommon::defaultTemplate> ThemeTemplateListWidget::defaultTemplates()
{
    return ThemeDefaultTemplate::defaultTemplates();
}

ThemeTemplateWidget::ThemeTemplateWidget(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    QLabel *lab = new QLabel(title);
    lay->addWidget(lab);
    mListTemplate = new ThemeTemplateListWidget(QStringLiteral("contactprintthemeeditorrc"));
    mListTemplate->setWhatsThis(i18n("You can drag and drop element on editor to import template"));
    connect(mListTemplate, &ThemeTemplateListWidget::insertTemplate, this, &ThemeTemplateWidget::insertTemplate);
    lay->addWidget(mListTemplate);
    setLayout(lay);
}

ThemeTemplateWidget::~ThemeTemplateWidget()
{
}

