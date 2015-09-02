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

#include "translatorplugin.h"
#include "translator/translatorwidget.h"
#include <KLocalizedString>
#include <KActionCollection>
#include <KToggleAction>

using namespace PimCommon;

TranslatorPlugin::TranslatorPlugin(QObject *parent)
    : PimCommon::CustomToolsPlugin(parent),
      mAction(Q_NULLPTR),
      mTranslatorWidget(Q_NULLPTR)
{

}

TranslatorPlugin::~TranslatorPlugin()
{

}

void TranslatorPlugin::createAction()
{
    mAction = new KToggleAction(i18n("&Translator"), this);
    connect(mAction, &KToggleAction::toggled, this, &TranslatorPlugin::slotActivateTranslator);
    mAction->setChecked(false);
}

KToggleAction *TranslatorPlugin::action() const
{
    return mAction;
}

QWidget *TranslatorPlugin::createView(QWidget *parent)
{
    mTranslatorWidget = new PimCommon::TranslatorWidget(parent);
    connect(mTranslatorWidget, &PimCommon::TranslatorWidget::toolsWasClosed, this, &TranslatorPlugin::toolsWasClosed);
    return mTranslatorWidget;
}

QString TranslatorPlugin::customToolName() const
{
    return i18n("Translator");
}

void TranslatorPlugin::setShortcut(KActionCollection *ac)
{
    if (ac)
        ac->setDefaultShortcut(mAction, QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_T));
}

void TranslatorPlugin::setText(const QString &text)
{
    mTranslatorWidget->setTextToTranslate(text);
}

void TranslatorPlugin::slotActivateTranslator(bool b)
{
    Q_EMIT activateTool(b ? mTranslatorWidget : Q_NULLPTR);
}
