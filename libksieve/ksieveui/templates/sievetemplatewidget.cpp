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

#include "sievetemplatewidget.h"
#include "sievetemplateeditdialog.h"
#include "sievedefaulttemplate.h"
#include "sievetemplatemanager.h"

#include <KLocale>

#include <QListWidget>
#include <QPointer>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>

namespace KSieveUi {

SieveTemplateListWidget::SieveTemplateListWidget(const QString &configName, QWidget *parent)
    : PimCommon::TemplateListWidget(configName, parent)
{
    setKNewStuffConfigFile(QLatin1String("ksieve_script.knsrc"));
    loadTemplates();
}

SieveTemplateListWidget::~SieveTemplateListWidget()
{
}

QList<PimCommon::defaultTemplate> SieveTemplateListWidget::defaultTemplates()
{
    return KSieveUi::SieveDefaultTemplate::defaultTemplates();
}

bool SieveTemplateListWidget::addNewTemplate(QString &templateName, QString &templateScript)
{
    QPointer<SieveTemplateEditDialog> dlg = new SieveTemplateEditDialog(this);
    if (dlg->exec()) {
        templateName = dlg->templateName();
        templateScript = dlg->script();
        delete dlg;
        return true;
    }
    delete dlg;
    return false;
}

bool SieveTemplateListWidget::modifyTemplate(QString &templateName, QString &templateScript, bool defaultTemplate)
{
    QPointer<SieveTemplateEditDialog> dlg = new SieveTemplateEditDialog(this, defaultTemplate);
    dlg->setTemplateName(templateName);
    dlg->setScript(templateScript);
    if (dlg->exec()) {
        if (!defaultTemplate) {
            templateName = dlg->templateName();
            templateScript = dlg->script();
        }
        delete dlg;
        return true;
    }
    delete dlg;
    return false;
}


SieveTemplateWidget::SieveTemplateWidget(const QString &title, QWidget *parent)
    : QWidget(parent),
      mTemplateManager(new SieveTemplateManager(this))
{
    QVBoxLayout *lay = new QVBoxLayout;
    QLabel *lab = new QLabel(title);
    lay->addWidget(lab);
    mListTemplate = new SieveTemplateListWidget(QLatin1String("sievetemplaterc"));
    mListTemplate->setWhatsThis(i18n("You can drag and drop element on editor to import template"));
    connect(mListTemplate, SIGNAL(insertTemplate(QString)), SIGNAL(insertTemplate(QString)));
    lay->addWidget(mListTemplate);
    setLayout(lay);
}


SieveTemplateWidget::~SieveTemplateWidget()
{
}

void SieveTemplateWidget::addDefaultTemplate(const QString &templateName, const QString &templateScript)
{
    mListTemplate->addDefaultTemplate(templateName, templateScript);
}

}
