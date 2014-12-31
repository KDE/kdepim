/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

namespace GrantleeThemeEditor {
class ThemeEditorTabWidget;
class ThemeSession;
class DesktopFilePage;
}
class EditorPage;
class KZip;

class ThemeEditorPage : public QWidget
{
    Q_OBJECT
public:
    explicit ThemeEditorPage(const QString &projectDir, const QString &themeName, QWidget *parent = 0);
    ~ThemeEditorPage();

    void saveThemeAs(const QString &directory);
    bool saveTheme(bool withConfirmation = true);
    void loadTheme(const QString &filename);

    void addExtraPage();

    void uploadTheme();
    void installTheme(const QString &themePath);

    bool themeWasChanged() const;

    void reloadConfig();

    void insertFile();

    void setPrinting(bool print);

    void updatePreview();

Q_SIGNALS:
    void changed(bool);
    void canInsertFile(bool);

private Q_SLOTS:
    void slotUpdateViewer();
    void slotCloseTab(int);
    void slotChanged();
    void slotExtraHeaderDisplayChanged(const QStringList &extraHeaders);
    void slotCurrentWidgetChanged(QWidget*);

private:
    void setChanged(bool b);
    void storeTheme(const QString &directory = QString());
    QString projectDirectory() const;
    void createZip(const QString &themeName, KZip *zip);
    EditorPage *createExtraPage(const QString &filename);
    QList<EditorPage*> mExtraPage;
    GrantleeThemeEditor::ThemeEditorTabWidget *mTabWidget;
    EditorPage *mEditorPage;
    GrantleeThemeEditor::DesktopFilePage *mDesktopPage;
    GrantleeThemeEditor::ThemeSession *mThemeSession;
    bool mChanged;
};

#endif // THEMEEDITORPAGE_H
