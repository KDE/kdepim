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
#ifndef __MESSAGELIST_UTILS_THEMECONFIGBUTTON_H__
#define __MESSAGELIST_UTILS_THEMECONFIGBUTTON_H__

#include <messagelist/messagelist_export.h>
#include <KPushButton>

namespace MessageList
{

namespace Utils
{

class ThemeComboBox;
class ThemeConfigButtonPrivate;

/**
 * A specialized KPushButton that displays the theme
 * configure dialog when pressed.
 */
class MESSAGELIST_EXPORT ThemeConfigButton : public KPushButton
{
    Q_OBJECT

public:
    /** Constructor.
   * @param parent The parent widget for the button.
   * @param themeComboBox Optional ThemeComboBox to be kept in sync
   * with changes made by the configure dialog.
   */
    explicit ThemeConfigButton( QWidget * parent, const ThemeComboBox * themeComboBox = 0 );
    ~ThemeConfigButton();

signals:
    /**
   * A signal emitted when configure dialog has been successfully completed.
   */
    void configureDialogCompleted();

private:
    Q_PRIVATE_SLOT(d, void slotConfigureThemes())

    ThemeConfigButtonPrivate * const d;
};

} // namespace Utils

} // namespace MessageList

#endif //!__MESSAGELIST_UTILS_THEMECONFIGBUTTON_H__
