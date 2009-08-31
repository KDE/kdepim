/* Copyright 2009 James Bendig <james@imptalk.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __MESSAGELIST_CORE_THEMECOMBOBOX_P_H__
#define __MESSAGELIST_CORE_THEMECOMBOBOX_P_H__

namespace MessageList
{

namespace Core
{

class Theme;
class ThemeComboBox;

class ThemeComboBox::Private
{
public:
  Private( ThemeComboBox *owner )
    : q( owner ) { }

  ThemeComboBox * const q;

  /**
   * Refresh list of themes in the combobox.
   */
  void slotLoadThemes();

  void setCurrentTheme( const Theme *theme );
  const Theme *currentTheme() const;
};

} // namespace Core

} // namespace MessageList

#endif //!__MESSAGELIST_CORE_THEMECOMBOBOX_P_H__
