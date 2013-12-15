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

#include "contacttemplatewidget.h"
#include "contactdefaulttemplate.h"

#include <KLocalizedString>

#include <QVBoxLayout>
#include <QLabel>

ContactTemplateListWidget::ContactTemplateListWidget(const QString &configName, QWidget *parent)
    : PimCommon::TemplateListWidget(configName, parent)
{
    loadTemplates();
}

ContactTemplateListWidget::~ContactTemplateListWidget()
{
}

QList<PimCommon::defaultTemplate> ContactTemplateListWidget::defaultTemplates()
{
    return ContactDefaultTemplate::contactTemplates();
}

ContactTemplateWidget::ContactTemplateWidget(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    QLabel *lab = new QLabel(title);
    lay->addWidget(lab);
    mListTemplate = new ContactTemplateListWidget(QLatin1String("contactthemeeditorrc"));
    mListTemplate->setWhatsThis(i18n("You can drag and drop element on editor to import template"));
    connect(mListTemplate, SIGNAL(insertTemplate(QString)), SIGNAL(insertTemplate(QString)));
    lay->addWidget(mListTemplate);
    setLayout(lay);
}


ContactTemplateWidget::~ContactTemplateWidget()
{
}

