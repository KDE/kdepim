/*
  Copyright (c) 2013-2016 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "blogilocomposerwebengineeditor.h"

#include "bilbomedia.h"
#include "global.h"

#include "blogilo_debug.h"

#include <QAction>
#include <KLocalizedString>
#include <QIcon>

#include <QMimeDatabase>
#include <QMimeType>

BlogiloComposerWebEngineEditor::BlogiloComposerWebEngineEditor(BlogiloComposerWebEngineView *view, QWidget *parent)
    : ComposerEditorWebEngine::ComposerWebEngineWidget(view, parent),
      readOnly(false)
{
    QList<ComposerEditorWebEngine::ComposerWebEngine::ComposerWebEngineAction> lstActions;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::Separator;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::Bold;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::Italic;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::Underline;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::StrikeOut;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::Separator;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::FormatType;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::FontSize;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::TextForegroundColor;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::FormatReset;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::BlockQuote;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::Separator;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::InsertLink;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::InsertImage;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::Separator;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::AlignLeft;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::AlignCenter;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::AlignRight;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::AlignJustify;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::Separator;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::DirectionRtl;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::OrderedList;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::UnorderedList;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::Separator;
    lstActions << ComposerEditorWebEngine::ComposerWebEngine::InsertTable;

    QList<ComposerEditorWebEngine::ComposerWebEngine::ComposerWebEngineAction> toolBarActions;
    toolBarActions << lstActions;

    QList<ComposerEditorWebEngine::ComposerWebEngine::ComposerWebEngineAction> lstActionsFormat;
    lstActionsFormat << ComposerEditorWebEngine::ComposerWebEngine::SubScript;
    lstActionsFormat << ComposerEditorWebEngine::ComposerWebEngine::SuperScript;
    lstActionsFormat << ComposerEditorWebEngine::ComposerWebEngine::Separator;
    lstActionsFormat << ComposerEditorWebEngine::ComposerWebEngine::ListIndent;
    lstActionsFormat << ComposerEditorWebEngine::ComposerWebEngine::ListDedent;
    lstActionsFormat << ComposerEditorWebEngine::ComposerWebEngine::Separator;
    lstActionsFormat << ComposerEditorWebEngine::ComposerWebEngine::TextBackgroundColor;

    //Create all actions first (before to add to toolbar)
    createActions(lstActions << ComposerEditorWebEngine::ComposerWebEngine::PasteWithoutFormatting << lstActionsFormat);

    KToolBar *mainToolBar = createToolBar(toolBarActions);

    mActSplitPost = new QAction(QIcon::fromTheme(QStringLiteral("insert-more-mark")), i18n("Split text"), this);
    connect(mActSplitPost, &QAction::triggered, this, &BlogiloComposerWebEngineEditor::slotAddPostSplitter);
    addActionInToolBar(mActSplitPost, mainToolBar);

    mActCode = new QAction(QIcon::fromTheme(QStringLiteral("format-text-code")), i18nc("Sets text font to code style",
                           "Code"), this);
    connect(mActCode, &QAction::triggered, this, &BlogiloComposerWebEngineEditor::slotToggleCode);
    addActionInToolBar(mActCode, mainToolBar);

    createToolBar(lstActionsFormat);
}

BlogiloComposerWebEngineEditor::~BlogiloComposerWebEngineEditor()
{

}

void BlogiloComposerWebEngineEditor::setReadOnly(bool _readOnly)
{
    qDebug()<<" void BlogiloComposerWebEngineEditor::setReadOnly(bool _readOnly)"<<_readOnly;
    if (readOnly != _readOnly) {
        readOnly = _readOnly;
        view()->evaluateJavascript(QStringLiteral("setReadOnly(%1)").arg(readOnly ? QStringLiteral("true") : QStringLiteral("false")));
    }
}

QList< BilboMedia * > BlogiloComposerWebEngineEditor::getLocalImages()
{

    qCDebug(BLOGILO_LOG);
    QList< BilboMedia * > list;
#if 0
    QWebElementCollection images = view()->page()->mainFrame()->findAllElements(QStringLiteral("img"));
    Q_FOREACH (const QWebElement &elm, images) {
        if (elm.attribute(QStringLiteral("src")).startsWith(QStringLiteral("file://"))) {
            //             qCDebug(BLOGILO_LOG)<<elm.toOuterXml();
            BilboMedia *media = new BilboMedia(this);
            QUrl mediaUrl(elm.attribute(QStringLiteral("src")));
            media->setLocalUrl(mediaUrl);
            QMimeDatabase db;
            media->setMimeType(db.mimeTypeForFile(mediaUrl.path(), QMimeDatabase::MatchExtension).name());
            media->setName(mediaUrl.fileName());
            media->setBlogId(__currentBlogId);
            list.append(media);
        }
    }
#endif
    return list;
}

void BlogiloComposerWebEngineEditor::replaceImageSrc(const QString &src, const QString &dest)
{
    const QString cmd = QStringLiteral("replaceImageSrc('%1','%2')").arg(src).arg(dest);
    view()->evaluateJavascript(cmd);
}

void BlogiloComposerWebEngineEditor::slotAddPostSplitter()
{
    execCommand(QStringLiteral("insertHTML"), QStringLiteral("<hr/><!--split-->"));
}

void BlogiloComposerWebEngineEditor::slotToggleCode(bool)
{
    const QString selection = view()->selectedText();
    if (selection.isEmpty()) {
        return;
    }
    const QString html = QStringLiteral("<code>%1</code>").arg(selection);
    execCommand(QStringLiteral("insertHtml"), html);
}

void BlogiloComposerWebEngineEditor::startEditing()
{
    static_cast<BlogiloComposerWebEngineView *>(view())->startEditing();
}

void BlogiloComposerWebEngineEditor::execCommand(const QString &cmd)
{
    const QString js = QStringLiteral("document.execCommand(\"%1\", false, null)").arg(cmd);
    view()->page()->runJavaScript(js);
}

void BlogiloComposerWebEngineEditor::execCommand(const QString &cmd, const QString &arg)
{
    const QString js = QStringLiteral("document.execCommand(\"%1\", false, \"%2\")").arg(cmd).arg(arg);
    view()->page()->runJavaScript(js);
}

void BlogiloComposerWebEngineEditor::insertShortUrl(const QString &url)
{
    QString html = QStringLiteral("<a href=\'%1\'>%1</a>").arg(url);
    execCommand(QStringLiteral("insertHtml"), html);
}

