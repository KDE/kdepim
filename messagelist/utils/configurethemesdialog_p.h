/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#ifndef __MESSAGELIST_UTILS_CONFIGURETHEMESDIALOG_P_H__
#define __MESSAGELIST_UTILS_CONFIGURETHEMESDIALOG_P_H__

#include "utils/configurethemesdialog.h"

namespace MessageList
{

namespace Core
{

class Theme;

}

namespace Utils
{

class ThemeEditor;
class ThemeListWidget;
class ThemeListWidgetItem;

class ConfigureThemesDialog::Private
{
public:
    Private(ConfigureThemesDialog *owner)
        : q(owner) { }

    void fillThemeList();
    QString uniqueNameForTheme(const QString &baseName, Core::Theme *skipTheme = 0);
    ThemeListWidgetItem *findThemeItemByName(const QString &name, Core::Theme *skipTheme = 0);
    ThemeListWidgetItem *findThemeItemByTheme(Core::Theme *set);
    ThemeListWidgetItem *findThemeItemById(const QString &themeId);
    void commitEditor();

    void themeListItemClicked(QListWidgetItem *);
    void newThemeButtonClicked();
    void cloneThemeButtonClicked();
    void deleteThemeButtonClicked();
    void editedThemeNameChanged();
    void okButtonClicked();
    void exportThemeButtonClicked();
    void importThemeButtonClicked();

    ConfigureThemesDialog *const q;

    ThemeListWidget *mThemeList;
    ThemeEditor *mEditor;
    QPushButton *mNewThemeButton;
    QPushButton *mCloneThemeButton;
    QPushButton *mDeleteThemeButton;
    QPushButton *mExportThemeButton;
    QPushButton *mImportThemeButton;

};

} // namespace Utils

} // namespace MessageList

#endif //!__MESSAGELIST_UTILS_CONFIGURESKINSDIALOG_P_H__
