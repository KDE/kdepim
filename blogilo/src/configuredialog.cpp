/*
  Copyright (c) 2014-2016 Montel Laurent <montel@kde.org>

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

#include "configuredialog.h"
#include "blogilo_debug.h"
#include "ui_advancedsettingsbase.h"
#include "ui_settingsbase.h"
#include "ui_editorsettingsbase.h"

#include "settings.h"

#include "blogsettings.h"

#include <KLocalizedString>

ConfigureDialog::ConfigureDialog(QWidget *parent, const QString &name, KConfigSkeleton *config)
    : KConfigDialog(parent, name, config),
      mHasChanged(false)
{
    QWidget *generalSettingsDlg = new QWidget;
    generalSettingsDlg->setAttribute(Qt::WA_DeleteOnClose);
    Ui::SettingsBase ui_prefs_base;
    Ui::EditorSettingsBase ui_editorsettings_base;
    ui_prefs_base.setupUi(generalSettingsDlg);

    BlogSettings *blogSettingsDlg = new BlogSettings;
    blogSettingsDlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(blogSettingsDlg, &BlogSettings::blogAdded, this, &ConfigureDialog::blogAdded);
    connect(blogSettingsDlg, &BlogSettings::blogEdited, this, &ConfigureDialog::blogEdited);
    connect(blogSettingsDlg, &BlogSettings::blogRemoved, this, &ConfigureDialog::blogRemoved);

    QWidget *editorSettingsDlg = new QWidget;
    editorSettingsDlg->setAttribute(Qt::WA_DeleteOnClose);
    ui_editorsettings_base.setupUi(editorSettingsDlg);
    QWidget *advancedSettingsDlg = new QWidget;
    advancedSettingsDlg->setAttribute(Qt::WA_DeleteOnClose);
    Ui::AdvancedSettingsBase ui_advancedsettings_base;
    ui_advancedsettings_base.setupUi(advancedSettingsDlg);

    addPage(generalSettingsDlg, i18nc("Configure Page", "General"), QStringLiteral("configure"));
    addPage(blogSettingsDlg, i18nc("Configure Page", "Blogs"), QStringLiteral("document-properties"));
    addPage(editorSettingsDlg, i18nc("Configure Page", "Editor"), QStringLiteral("accessories-text-editor"));
    addPage(advancedSettingsDlg, i18nc("Configure Page", "Advanced"), QStringLiteral("applications-utilities"));

    connect(this, &KConfigDialog::settingsChanged, this, &ConfigureDialog::configurationChanged);
    connect(this, &ConfigureDialog::destroyed, this, &ConfigureDialog::dialogDestroyed);
    connect(button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &ConfigureDialog::slotApplySettingsClicked);
    connect(button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &ConfigureDialog::slotApplySettingsClicked);
    setAttribute(Qt::WA_DeleteOnClose);
    resize(Settings::configWindowSize());
    show();
}

ConfigureDialog::~ConfigureDialog()
{

}

bool ConfigureDialog::hasChanged()
{
    return (KConfigDialog::hasChanged() || mHasChanged);
}

void ConfigureDialog::slotApplySettingsClicked()
{
    mHasChanged = false;
    updateButtons();
    Q_EMIT configurationChanged();
}
