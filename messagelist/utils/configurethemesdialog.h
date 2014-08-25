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

#ifndef __MESSAGELIST_UTILS_CONFIGURETHEMESDIALOG_H__
#define __MESSAGELIST_UTILS_CONFIGURETHEMESDIALOG_H__

#include <QDialog>

#include <QListWidget>

#include <messagelist/messagelist_export.h>

class QPushButton;

namespace MessageList
{

namespace Core
{

class Manager;

} // namespace Core

namespace Utils
{

class MESSAGELIST_EXPORT ConfigureThemesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureThemesDialog( QWidget *parent = 0 );
    ~ConfigureThemesDialog();

    void selectTheme( const QString &themeId );

Q_SIGNALS:
    void okClicked();

private:
    Q_PRIVATE_SLOT(d, void themeListItemClicked(QListWidgetItem*))
    Q_PRIVATE_SLOT(d, void newThemeButtonClicked())
    Q_PRIVATE_SLOT(d, void cloneThemeButtonClicked())
    Q_PRIVATE_SLOT(d, void deleteThemeButtonClicked())
    Q_PRIVATE_SLOT(d, void editedThemeNameChanged())
    Q_PRIVATE_SLOT(d, void okButtonClicked())
    Q_PRIVATE_SLOT(d, void importThemeButtonClicked())
    Q_PRIVATE_SLOT(d, void exportThemeButtonClicked())


    class Private;
    Private * const d;
};

} // namespace Utils

} // namespace MessageList

#endif //!__MESSAGELIST_UTILS_CONFIGURESKINSDIALOG_H__
