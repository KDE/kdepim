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


#include "editorpage.h"
#include "editorwidget.h"
#include "previewwidget.h"
#include "themeeditorwidget.h"
#include "themetemplatewidget.h"
#include "pimcommon/texteditor/plaintexteditor/plaintexteditor.h"
#include "pimcommon/widgets/splittercollapser.h"

#include <KLocale>
#include <KGlobal>
#include <KConfigGroup>

#include <QSplitter>
#include <QVBoxLayout>

EditorPage::EditorPage(GrantleeThemeEditor::EditorPage::PageType type, const QString &projectDirectory, QWidget *parent)
    : GrantleeThemeEditor::EditorPage(type, parent),
      mWidgetSplitter(0)
{
    QVBoxLayout *lay = new QVBoxLayout;

    mMainSplitter = new QSplitter;
    if (mType == MainPage) {
        mWidgetSplitter = new QSplitter;
        mWidgetSplitter->setOrientation(Qt::Vertical);
        mWidgetSplitter->setChildrenCollapsible(false);
        lay->addWidget(mWidgetSplitter);

        mWidgetSplitter->addWidget(mMainSplitter);

        mPreview = new PreviewWidget(projectDirectory);
        mWidgetSplitter->addWidget(mPreview);
        connect(mPreview, SIGNAL(needUpdateViewer()), this, SIGNAL(needUpdateViewer()));
    } else {
        lay->addWidget(mMainSplitter);
    }

    mEditor = new ThemeEditorWidget;

    if (mType == MainPage) {
        KConfigGroup group( KGlobal::config(), QLatin1String("Global"));
        mEditor->setPlainText(group.readEntry("defaultTemplate",QString()));
    }
    mMainSplitter->addWidget(mEditor);
    mThemeTemplate = new ThemeTemplateWidget(i18n("Theme Templates:"));
    connect(mThemeTemplate, SIGNAL(insertTemplate(QString)), mEditor->editor(), SLOT(insertPlainText(QString)));
    mMainSplitter->addWidget(mThemeTemplate);
    mMainSplitter->setCollapsible(0, false);
    new PimCommon::SplitterCollapser(mMainSplitter, mThemeTemplate, this);

    connect(mEditor->editor(), SIGNAL(textChanged()), this, SIGNAL(changed()));


    if (mType == MainPage) {
        KConfigGroup group( KGlobal::config(), "EditorPage" );
        QList<int> size;
        size << 400 << 100;
        mMainSplitter->setSizes(group.readEntry( "mainSplitter", size));
        mWidgetSplitter->setSizes(group.readEntry( "widgetSplitter", size));
    }
    setLayout(lay);
}

EditorPage::~EditorPage()
{
    if (mType == MainPage) {
        KConfigGroup group( KGlobal::config(), "EditorPage" );
        group.writeEntry( "mainSplitter", mMainSplitter->sizes());
        group.writeEntry("widgetSplitter", mWidgetSplitter->sizes());
    }
}

GrantleeThemeEditor::PreviewWidget *EditorPage::preview() const
{
    return mPreview;
}

