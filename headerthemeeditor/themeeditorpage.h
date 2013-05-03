/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef THEMEEDITORPAGE_H
#define THEMEEDITORPAGE_H

#include <QWidget>

class KTabWidget;
class EditorPage;
class DesktopFilePage;
class PreviewPage;
class ThemeSession;
class KZip;
class ThemeEditorPage : public QWidget
{
    Q_OBJECT
public:
    explicit ThemeEditorPage(const QString &themeName, QWidget *parent = 0);
    ~ThemeEditorPage();

    void saveTheme();
    void loadTheme(const QString &filename);

    void addExtraPage();

    QString projectDirectory() const;
    void setProjectDirectory(const QString &dir);

    void uploadTheme();

private:
    bool themeWasChanged() const;
    void createZip(const QString &themeName, KZip *zip);
    QList<EditorPage*> mExtraPage;
    KTabWidget *mTabWidget;
    EditorPage *mEditorPage;
    DesktopFilePage *mDesktopPage;
    PreviewPage *mPreviewPage;
    ThemeSession *mThemeSession;
};

#endif // THEMEEDITORPAGE_H
