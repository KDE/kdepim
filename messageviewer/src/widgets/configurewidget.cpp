/*
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "configurewidget.h"
#include "customheadersettingdialog.h"
#include "messageviewer_debug.h"
#include "ui_settings.h"
#include "messageviewer/messageviewerutil.h"
#include "settings/messageviewersettings.h"
#include "messageviewer/nodehelper.h"

#include "MessageCore/MessageCoreSettings"

#include <KConfigDialogManager>
#include <KLocalizedString>

using namespace MessageViewer;

class MessageViewer::ConfigureWidgetPrivate
{
public:
    ConfigureWidgetPrivate()
        : mSettingsUi(Q_NULLPTR)
    {

    }

    ~ConfigureWidgetPrivate()
    {
        delete mSettingsUi;
        mSettingsUi = Q_NULLPTR;
    }

    Ui_Settings *mSettingsUi;
};

ConfigureWidget::ConfigureWidget(QWidget *parent)
    : QWidget(parent),
      d(new MessageViewer::ConfigureWidgetPrivate)
{
    d->mSettingsUi = new Ui_Settings;
    d->mSettingsUi->setupUi(this);
    layout()->setContentsMargins(0, 0, 0, 0);

    QStringList encodings = NodeHelper::supportedEncodings(false);
    encodings.prepend(i18n("Auto"));
    d->mSettingsUi->overrideCharacterEncoding->addItems(encodings);
    d->mSettingsUi->overrideCharacterEncoding->setCurrentIndex(0);

    d->mSettingsUi->overrideCharacterEncoding->setWhatsThis(
        MessageCore::MessageCoreSettings::self()->overrideCharacterEncodingItem()->whatsThis());
    d->mSettingsUi->kcfg_ShowEmoticons->setWhatsThis(
        MessageViewer::MessageViewerSettings::self()->showEmoticonsItem()->whatsThis());
    d->mSettingsUi->kcfg_ShrinkQuotes->setWhatsThis(
        MessageViewer::MessageViewerSettings::self()->shrinkQuotesItem()->whatsThis());
    d->mSettingsUi->kcfg_ShowExpandQuotesMark->setWhatsThis(
        MessageViewer::MessageViewerSettings::self()->showExpandQuotesMarkItem()->whatsThis());

    connect(d->mSettingsUi->overrideCharacterEncoding, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &ConfigureWidget::settingsChanged);

    connect(d->mSettingsUi->configureCustomHeadersButton, &QPushButton::clicked, this, &ConfigureWidget::showCustomHeadersDialog);
}

ConfigureWidget::~ConfigureWidget()
{
    delete d;
}

void ConfigureWidget::readConfig()
{
    readCurrentOverrideCodec();
    d->mSettingsUi->kcfg_CollapseQuoteLevelSpin->setEnabled(
        MessageViewer::MessageViewerSettings::self()->showExpandQuotesMark());
}

void ConfigureWidget::writeConfig()
{
    MessageCore::MessageCoreSettings::self()->setOverrideCharacterEncoding(
        d->mSettingsUi->overrideCharacterEncoding->currentIndex() == 0 ?
        QString() :
        NodeHelper::encodingForName(d->mSettingsUi->overrideCharacterEncoding->currentText()));
}

void ConfigureWidget::readCurrentOverrideCodec()
{
    const QString &currentOverrideEncoding = MessageCore::MessageCoreSettings::self()->overrideCharacterEncoding();
    if (currentOverrideEncoding.isEmpty()) {
        d->mSettingsUi->overrideCharacterEncoding->setCurrentIndex(0);
        return;
    }
    QStringList encodings = NodeHelper::supportedEncodings(false);
    encodings.prepend(i18n("Auto"));
    QStringList::ConstIterator it(encodings.constBegin());
    const QStringList::ConstIterator end(encodings.constEnd());
    int i = 0;
    for (; it != end; ++it) {
        if (NodeHelper::encodingForName(*it) == currentOverrideEncoding) {
            d->mSettingsUi->overrideCharacterEncoding->setCurrentIndex(i);
            break;
        }
        ++i;
    }
    if (i == encodings.size()) {
        // the current value of overrideCharacterEncoding is an unknown encoding => reset to Auto
        qCWarning(MESSAGEVIEWER_LOG) << "Unknown override character encoding" << currentOverrideEncoding
                                     << ". Resetting to Auto.";
        d->mSettingsUi->overrideCharacterEncoding->setCurrentIndex(0);
        MessageCore::MessageCoreSettings::self()->setOverrideCharacterEncoding(QString());
    }
}

void ConfigureWidget::showCustomHeadersDialog()
{
    QPointer<CustomHeaderSettingDialog> dlg = new CustomHeaderSettingDialog(this);
    if (dlg->exec()) {
        dlg->writeSettings();
        settingsChanged();
    }
    delete dlg;
}

