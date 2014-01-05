/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include "blogilocomposereditor.h"

#include "bilbomedia.h"
#include "global.h"

#include <KDebug>
#include <KMimeType>
#include <KAction>
#include <KLocale>

#include <QWebElementCollection>
#include <QWebFrame>

BlogiloComposerEditor::BlogiloComposerEditor(BlogiloComposerView *view, QWidget *parent)
    : ComposerEditorNG::ComposerEditor(view,parent),
      readOnly(false)
{
    QList<ComposerEditorNG::ComposerView::ComposerViewAction> lstActions;
    lstActions<<ComposerEditorNG::ComposerView::Separator;
    lstActions<<ComposerEditorNG::ComposerView::Bold;
    lstActions<<ComposerEditorNG::ComposerView::Italic;
    lstActions<<ComposerEditorNG::ComposerView::Underline;
    lstActions<<ComposerEditorNG::ComposerView::StrikeOut;
    lstActions<<ComposerEditorNG::ComposerView::Separator;
    lstActions<<ComposerEditorNG::ComposerView::FormatType;
    lstActions<<ComposerEditorNG::ComposerView::FontSize;
    lstActions<<ComposerEditorNG::ComposerView::TextForegroundColor;
    lstActions<<ComposerEditorNG::ComposerView::FormatReset;
    lstActions<<ComposerEditorNG::ComposerView::BlockQuote;
    lstActions<<ComposerEditorNG::ComposerView::Separator;
    lstActions<<ComposerEditorNG::ComposerView::InsertLink;
    lstActions<<ComposerEditorNG::ComposerView::InsertImage;
    lstActions<<ComposerEditorNG::ComposerView::Separator;
    lstActions<<ComposerEditorNG::ComposerView::AlignLeft;
    lstActions<<ComposerEditorNG::ComposerView::AlignCenter;
    lstActions<<ComposerEditorNG::ComposerView::AlignRight;
    lstActions<<ComposerEditorNG::ComposerView::AlignJustify;
    lstActions<<ComposerEditorNG::ComposerView::Separator;
    lstActions<<ComposerEditorNG::ComposerView::DirectionRtl;
    lstActions<<ComposerEditorNG::ComposerView::OrderedList;
    lstActions<<ComposerEditorNG::ComposerView::UnorderedList;
    lstActions<<ComposerEditorNG::ComposerView::Separator;
    lstActions<<ComposerEditorNG::ComposerView::InsertTable;


    QList<ComposerEditorNG::ComposerView::ComposerViewAction> toolBarActions;
    toolBarActions<<lstActions;

    QList<ComposerEditorNG::ComposerView::ComposerViewAction> lstActionsFormat;
    lstActionsFormat<<ComposerEditorNG::ComposerView::SubScript;
    lstActionsFormat<<ComposerEditorNG::ComposerView::SuperScript;
    lstActionsFormat<<ComposerEditorNG::ComposerView::Separator;
    lstActionsFormat<<ComposerEditorNG::ComposerView::ListIndent;
    lstActionsFormat<<ComposerEditorNG::ComposerView::ListDedent;
    lstActionsFormat<<ComposerEditorNG::ComposerView::Separator;
    lstActionsFormat<<ComposerEditorNG::ComposerView::TextBackgroundColor;

    //Create all actions first (before to add to toolbar)
    createActions(lstActions<<ComposerEditorNG::ComposerView::PasteWithoutFormatting<<lstActionsFormat);

    KToolBar *mainToolBar = createToolBar(toolBarActions);


    mActSplitPost = new KAction( KIcon( QLatin1String("insert-more-mark") ), i18n( "Split text" ), this );
    connect( mActSplitPost, SIGNAL(triggered(bool)), this, SLOT(slotAddPostSplitter()) );
    addActionInToolBar(mActSplitPost, mainToolBar);

    mActCode = new KAction( KIcon( QLatin1String("format-text-code") ), i18nc( "Sets text font to code style",
                           "Code" ), this );
    connect( mActCode, SIGNAL(triggered(bool)), this, SLOT(slotToggleCode(bool)) );
    addActionInToolBar(mActCode, mainToolBar);

    createToolBar(lstActionsFormat);
}

BlogiloComposerEditor::~BlogiloComposerEditor()
{

}

void BlogiloComposerEditor::setReadOnly ( bool _readOnly )
{
    if (readOnly != _readOnly) {
        readOnly = _readOnly;
        view()->evaluateJavascript( QString::fromLatin1( "setReadOnly(%1)" ).arg ( readOnly ? QLatin1String("true") : QLatin1String("false") ) );
    }
}

QList< BilboMedia* > BlogiloComposerEditor::getLocalImages()
{
    kDebug();
    QList< BilboMedia* > list;
    QWebElementCollection images = view()->page()->mainFrame()->findAllElements(QLatin1String("img"));
    Q_FOREACH (const QWebElement& elm, images){
        if (elm.attribute(QLatin1String("src")).startsWith(QLatin1String("file://"))){
            //             kDebug()<<elm.toOuterXml();
            BilboMedia* media = new BilboMedia(this);
            KUrl mediaUrl (elm.attribute(QLatin1String("src")));
            media->setLocalUrl( mediaUrl );
            media->setMimeType( KMimeType::findByUrl( mediaUrl, 0, true )->name() );
            media->setName(mediaUrl.fileName());
            media->setBlogId(__currentBlogId);
            list.append(media);
        }
    }
    return list;
}

void BlogiloComposerEditor::replaceImageSrc(const QString& src, const QString& dest)
{
    const QString cmd = QString::fromLatin1("replaceImageSrc('%1','%2')").arg(src).arg(dest);
    view()->evaluateJavascript(cmd);
}

void BlogiloComposerEditor::slotAddPostSplitter()
{
    execCommand(QLatin1String("insertHTML"), QLatin1String("<hr/><!--split-->"));
}

void BlogiloComposerEditor::slotToggleCode(bool )
{
    const QString selection = view()->selectedText();
    if(selection.isEmpty())
        return;
    const QString html = QString::fromLatin1( "<code>%1</code>" ).arg ( selection );
    execCommand(QLatin1String("insertHtml"), html);
}

#if 0
QString TextEditor::getHtml() const
{
    QString html = const_cast<TextEditor*>( this )->evaluateJavaScript( "getHtml()", false ).toString();
    html.remove(" xmlns=\"http://www.w3.org/1999/xhtml\"", Qt::CaseInsensitive);
    return html;
}
#endif

void BlogiloComposerEditor::startEditing()
{
    static_cast<BlogiloComposerView*>(view())->startEditing();
}

void BlogiloComposerEditor::execCommand ( const QString &cmd )
{
    QWebFrame *frame = view()->page()->mainFrame();
    const QString js = QString::fromLatin1( "document.execCommand(\"%1\", false, null)" ).arg ( cmd );
    frame->evaluateJavaScript ( js );
}

void BlogiloComposerEditor::execCommand( const QString &cmd, const QString &arg )
{
    QWebFrame *frame = view()->page()->mainFrame();
    const QString js = QString::fromLatin1( "document.execCommand(\"%1\", false, \"%2\")" ).arg ( cmd ).arg ( arg );
    frame->evaluateJavaScript ( js );
}

void BlogiloComposerEditor::insertShortUrl(const QString &url)
{
    QString html = QString::fromLatin1( "<a href=\'%1\'>%1</a>").arg(url);
    execCommand(QLatin1String("insertHtml"), html);
}



