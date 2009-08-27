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

#ifndef __MESSAGELIST_CORE_CONFIGURETHEMESDIALOG_P_H__
#define __MESSAGELIST_CORE_CONFIGURETHEMESDIALOG_P_H__

#include "core/configurethemesdialog.h"

namespace MessageList
{

namespace Core
{

class ThemeEditor;
class Theme;
class ThemeListWidget;
class ThemeListWidgetItem;

class ConfigureThemesDialog::Private
{
public:
  Private( ConfigureThemesDialog *owner )
    : q( owner ) { }

  static void display( QWidget * parent, const QString &preselectThemeId = QString() );
  static void cleanup();

  void fillThemeList();
  QString uniqueNameForTheme( QString baseName, Theme * skipTheme = 0 );
  ThemeListWidgetItem * findThemeItemByName( const QString &name, Theme * skipTheme = 0 );
  ThemeListWidgetItem * findThemeItemByTheme( Theme * set );
  ThemeListWidgetItem * findThemeItemById( const QString &themeId );
  void selectThemeById( const QString &themeId );
  void commitEditor();

  void themeListCurrentItemChanged( QListWidgetItem * cur, QListWidgetItem * prev );
  void newThemeButtonClicked();
  void cloneThemeButtonClicked();
  void deleteThemeButtonClicked();
  void editedThemeNameChanged();
  void okButtonClicked();

  ConfigureThemesDialog * const q;

  static ConfigureThemesDialog * mInstance;
  ThemeListWidget *mThemeList;
  ThemeEditor *mEditor;
  QPushButton *mNewThemeButton;
  QPushButton *mCloneThemeButton;
  QPushButton *mDeleteThemeButton;
};

} // namespace Core

} // namespace MessageList

#endif //!__MESSAGELIST_CORE_CONFIGURESKINSDIALOG_P_H__
