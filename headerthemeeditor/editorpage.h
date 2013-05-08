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


#ifndef EDITORPAGE_H
#define EDITORPAGE_H

#include <QWidget>

class ThemeTemplateWidget;
class EditorWidget;
class KZip;
class QSplitter;
class PreviewWidget;

class EditorPage : public QWidget
{
    Q_OBJECT
public:
    explicit EditorPage(QWidget *parent = 0);
    ~EditorPage();
    void saveTheme(const QString &path);
    void loadTheme(const QString &path);

    void setPageFileName(const QString &filename);
    QString pageFileName() const;

    void createZip(const QString &themeName, KZip *zip);
    void saveAsFilename(const QString &filename);
    void installTheme(const QString &themePath);

    bool wasChanged() const;

private Q_SLOTS:
    void slotChanged();

private:
    QString mPageFileName;
    EditorWidget *mEditor;
    PreviewWidget *mPreview;
    ThemeTemplateWidget *mThemeTemplate;
    QSplitter *mMainSplitter;
    QSplitter *mWidgetSplitter;
    bool mChanged;
};

#endif // EDITORPAGE_H
