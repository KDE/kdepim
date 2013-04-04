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

#include "editorpage.h"
#include "themetemplatewidget.h"

#include "kpimtextedit/htmlhighlighter.h"

#include <KTextEdit>
#include <KLocale>

#include <QSplitter>
#include <QVBoxLayout>

EditorPage::EditorPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    QSplitter *splitter = new QSplitter;
    lay->addWidget(splitter);
    mEditor = new KTextEdit;
    new KPIMTextEdit::HtmlHighlighter(mEditor->document());
    mEditor->setAcceptRichText(false);

    splitter->addWidget(mEditor);
    mThemeTemplate = new ThemeTemplateWidget(i18n("Theme Templates:"));
    splitter->addWidget(mThemeTemplate);

    setLayout(lay);
}

EditorPage::~EditorPage()
{
}

void EditorPage::saveTheme()
{

}



#include "editorpage.moc"
