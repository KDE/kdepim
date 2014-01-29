/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "ui_advancedsettingsbase.h"
#include "ui_settingsbase.h"
#include "ui_editorsettingsbase.h"

#include "settings.h"

#include "blogsettings.h"


#include <KLocalizedString>

ConfigureDialog::ConfigureDialog(QWidget *parent, const QString& name, KConfigSkeleton *config)
    : KConfigDialog(parent, name, config)
{
    QWidget *generalSettingsDlg = new QWidget;
    generalSettingsDlg->setAttribute( Qt::WA_DeleteOnClose );
    Ui::SettingsBase ui_prefs_base;
    Ui::EditorSettingsBase ui_editorsettings_base;
    ui_prefs_base.setupUi( generalSettingsDlg );

    BlogSettings *blogSettingsDlg = new BlogSettings;
    blogSettingsDlg->setAttribute( Qt::WA_DeleteOnClose );
    connect( blogSettingsDlg, SIGNAL(blogAdded(BilboBlog)),
             this, SIGNAL(blogAdded(BilboBlog)) );
    connect( blogSettingsDlg, SIGNAL(blogEdited(BilboBlog)),
             this, SIGNAL(blogEdited(BilboBlog)) );
    connect( blogSettingsDlg, SIGNAL(blogRemoved(int)), this, SIGNAL(blogRemoved(int)) );

    QWidget *editorSettingsDlg = new QWidget;
    editorSettingsDlg->setAttribute( Qt::WA_DeleteOnClose );
    ui_editorsettings_base.setupUi( editorSettingsDlg );
    QWidget *advancedSettingsDlg = new QWidget;
    advancedSettingsDlg->setAttribute( Qt::WA_DeleteOnClose );
    Ui::AdvancedSettingsBase ui_advancedsettings_base;
    ui_advancedsettings_base.setupUi( advancedSettingsDlg );

    addPage( generalSettingsDlg, i18nc( "Configure Page", "General" ), QLatin1String("configure") );
    addPage( blogSettingsDlg, i18nc( "Configure Page", "Blogs" ), QLatin1String("document-properties"));
    addPage( editorSettingsDlg, i18nc( "Configure Page", "Editor" ), QLatin1String("accessories-text-editor"));
    addPage( advancedSettingsDlg, i18nc( "Configure Page", "Advanced" ), QLatin1String("applications-utilities"));
    connect( this, SIGNAL(settingsChanged(QString)), this, SIGNAL(settingsChanged()) );
    connect( this, SIGNAL(destroyed(QObject*)), this, SIGNAL(dialogDestroyed(QObject*)));
    setAttribute( Qt::WA_DeleteOnClose );
    resize( Settings::configWindowSize() );
    show();
}

ConfigureDialog::~ConfigureDialog()
{

}
