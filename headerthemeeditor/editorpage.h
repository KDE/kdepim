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

#include "grantleethemeeditor/editorpage.h"


class ThemeTemplateWidget;
class EditorWidget;
class KZip;
class QSplitter;
class PreviewWidget;
class ThemeEditorWidget;
namespace GrantleeThemeEditor {
class EditorWidget;
}

class EditorPage : public GrantleeThemeEditor::EditorPage
{
    Q_OBJECT
public:

    explicit EditorPage(GrantleeThemeEditor::EditorPage::PageType type, const QString &projectDirectory, QWidget *parent = 0);
    ~EditorPage();

    PreviewWidget *preview() const;

private:
    PreviewWidget *mPreview;
    ThemeTemplateWidget *mThemeTemplate;
    QSplitter *mMainSplitter;
    QSplitter *mWidgetSplitter;
};

#endif // EDITORPAGE_H
