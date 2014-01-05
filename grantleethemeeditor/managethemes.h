/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef MANAGETHEMES_H
#define MANAGETHEMES_H

#include "grantleethemeeditor_export.h"
#include <KDialog>

class QListWidget;
class KPushButton;
namespace GrantleeThemeEditor {

class GRANTLEETHEMEEDITOR_EXPORT ManageThemes : public KDialog
{
    Q_OBJECT
public:
    explicit ManageThemes(const QString &relativeThemePath, QWidget *parent = 0);
    ~ManageThemes();

private Q_SLOTS:
    void slotItemSelectionChanged();
    void slotDeleteTheme();

private:
    void readConfig();
    void writeConfig();
    void initialize();
    QString mLocalDirectory;
    QListWidget *mListThemes;
    KPushButton *mDeleteTheme;
};
}
#endif // MANAGETHEMES_H
