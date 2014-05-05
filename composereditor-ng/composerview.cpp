/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "composerview.h"
#include "private/composerview_p.h"

#include "table/composertableactionmenu.h"
#include "globalsettings_base.h"

#include <kpimtextedit/emoticontexteditaction.h>


#include <KLocalizedString>
#include <QAction>
#include <KToggleAction>
#include <KFontAction>
#include <KSelectAction>
#include <KActionCollection>
#include <KMessageBox>
#include <KStandardDirs>
#include <QDebug>
#include <KMenu>
#include <KToolBar>
#include <KIcon>

#include <QAction>
#include <QFileInfo>
#include <QWebElement>
#include <QContextMenuEvent>
#include <QDebug>

namespace ComposerEditorNG {

ComposerView::ComposerView(QWidget *parent)
    : KWebView(parent),
      d(new ComposerViewPrivate(this))
{
    QFile file ( initialHtml() );
    qDebug() <<file.fileName();

    if ( !file.open ( QIODevice::ReadOnly ) )
        KMessageBox::error(this, i18n ( "Cannot open template file %1.", QFileInfo(file).absoluteFilePath() ), i18n ( "composer editor" ));
    else
        setContent ( file.readAll());//, "application/xhtml+xml" );

    page()->setContentEditable(true);
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(this, SIGNAL(linkClicked(QUrl)), SIGNAL(openLink(QUrl)));
    connect(page(), SIGNAL(selectionChanged()), this, SLOT(_k_slotAdjustActions()) );

    setWindowModified(false);
}

ComposerView::~ComposerView()
{
    delete d;
}

QString ComposerView::initialHtml()
{
    return KStandardDirs::locate ( "data", QLatin1String("composereditor/composereditorinitialhtml") );
}

void ComposerView::createActions(const QList<ComposerViewAction>& lstActions)
{
    Q_FOREACH (ComposerViewAction action, lstActions)
        d->createAction( action );

    d->connectActionGroup();
}

void ComposerView::createAllActions()
{
    for (uint i=0;i<LastType;++i)
        d->createAction( (ComposerViewAction)i );
    d->connectActionGroup();
}


void ComposerView::addCreatedActionsToActionCollection(KActionCollection *actionCollection)
{
    if (actionCollection) {
        if (d->action_text_bold)
            actionCollection->addAction(QLatin1String("htmleditor_format_text_bold"), d->action_text_bold);
        if (d->action_text_italic)
            actionCollection->addAction(QLatin1String("htmleditor_format_text_italic"), d->action_text_italic);
        if (d->action_text_underline)
            actionCollection->addAction(QLatin1String("htmleditor_format_text_underline"), d->action_text_underline);
        if (d->action_text_strikeout)
            actionCollection->addAction(QLatin1String("htmleditor_format_text_strikeout"), d->action_text_strikeout);
        if (d->action_align_left)
            actionCollection->addAction(QLatin1String("htmleditor_format_align_left"), d->action_align_left);
        if (d->action_align_center)
            actionCollection->addAction(QLatin1String("htmleditor_format_align_center"), d->action_align_center);
        if (d->action_align_right)
            actionCollection->addAction(QLatin1String("htmleditor_format_align_right"), d->action_align_right);
        if (d->action_align_justify)
            actionCollection->addAction(QLatin1String("htmleditor_format_align_justify"), d->action_align_justify);
        if (d->action_direction_ltr)
            actionCollection->addAction(QLatin1String("htmleditor_direction_ltr"), d->action_direction_ltr);
        if (d->action_direction_rtl)
            actionCollection->addAction(QLatin1String("htmleditor_direction_rtl"), d->action_direction_rtl);
        if (d->action_text_subscript)
            actionCollection->addAction(QLatin1String("htmleditor_format_text_subscript"), d->action_text_subscript);
        if (d->action_text_superscript)
            actionCollection->addAction(QLatin1String("htmleditor_format_text_superscript"), d->action_text_superscript);
        if (d->action_page_color)
            actionCollection->addAction( QLatin1String( "htmleditor_page_color_and_background" ), d->action_page_color );
        if (d->action_insert_table)
            actionCollection->addAction( QLatin1String( "htmleditor_insert_new_table" ), d->action_insert_table );
        if (d->action_insert_link)
            actionCollection->addAction(QLatin1String("htmleditor_insert_link"), d->action_insert_link);
        if (d->action_insert_horizontal_rule)
            actionCollection->addAction(QLatin1String("htmleditor_insert_horizontal_rule"), d->action_insert_horizontal_rule);
        if (d->action_text_foreground_color)
            actionCollection->addAction(QLatin1String("htmleditor_format_text_foreground_color"), d->action_text_foreground_color);
        if (d->action_text_background_color)
            actionCollection->addAction(QLatin1String("htmleditor_format_text_background_color"), d->action_text_background_color);
        if (d->action_add_emoticon)
            actionCollection->addAction(QLatin1String("htmleditor_add_emoticon"), d->action_add_emoticon);
        if (d->action_insert_html)
            actionCollection->addAction( QLatin1String( "htmleditor_insert_html" ), d->action_insert_html);
        if (d->action_insert_image)
            actionCollection->addAction( QLatin1String( "htmleditor_add_image" ), d->action_insert_image);
        if (d->action_spell_check)
            actionCollection->addAction(QLatin1String("htmleditor_spell_check"), d->action_spell_check);
        if (d->action_format_reset)
            actionCollection->addAction( QLatin1String("htmleditor_format_reset"), d->action_format_reset);
        if (d->action_font_family)
            actionCollection->addAction(QLatin1String("htmleditor_format_font_family"), d->action_font_family);
        if (d->action_font_size)
            actionCollection->addAction(QLatin1String("htmleditor_format_font_size"), d->action_font_size);
        if (d->action_format_type)
            actionCollection->addAction(QLatin1String("htmleditor_format_type"), d->action_format_type);
        if (d->action_block_quote)
            actionCollection->addAction(QLatin1String("htmleditor_block_quote"), d->action_block_quote);
        if (d->action_ordered_list)
            actionCollection->addAction(QLatin1String("htmleditor_format_list_ordered"), d->action_ordered_list);
        if (d->action_unordered_list)
            actionCollection->addAction(QLatin1String("htmleditor_format_list_unordered"), d->action_unordered_list);
        if (d->action_list_indent)
            actionCollection->addAction(QLatin1String("htmleditor_format_list_indent_more"), d->action_list_indent);
        if (d->action_list_dedent)
            actionCollection->addAction(QLatin1String("htmleditor_format_list_indent_less"), d->action_list_dedent);
        if (d->action_find)
            actionCollection->addAction(QLatin1String("htmleditor_find"), d->action_find);
        if (d->action_replace)
            actionCollection->addAction(QLatin1String("htmleditor_replace"), d->action_replace);
        if (d->action_save_as)
            actionCollection->addAction(QLatin1String("htmleditor_save_as"), d->action_save_as);
        if (d->action_print)
            actionCollection->addAction(QLatin1String("htmleditor_print"), d->action_print);
        if (d->action_print_preview)
            actionCollection->addAction(QLatin1String("htmleditor_print_preview"), d->action_print_preview);
        if (d->action_paste_withoutformatting)
            actionCollection->addAction(QLatin1String("htmleditor_paste_without_formatting"), d->action_paste_withoutformatting);
        if (d->action_insert_specialchar)
            actionCollection->addAction(QLatin1String("htmleditor_insert_specialchar"), d->action_insert_specialchar);
        if (d->action_insert_anchor)
            actionCollection->addAction(QLatin1String("htmleditor_insert_anchor"), d->action_insert_anchor);
    }
}

void ComposerView::contextMenuEvent(QContextMenuEvent *event)
{
    d->hideImageResizeWidget();
    d->contextMenuResult = page()->mainFrame()->hitTestContent(event->pos());

    const bool linkSelected = !d->contextMenuResult.linkElement().isNull();
    const bool imageSelected = !d->contextMenuResult.imageUrl().isEmpty();

    const QWebElement elm = d->contextMenuResult.element();
    const bool tableCellSelected = (elm.tagName().toLower() == QLatin1String("td"));
    const bool tableSelected = (elm.tagName().toLower() == QLatin1String("table") ||
                                tableCellSelected );

    const bool listSelected = (elm.tagName().toLower() == QLatin1String("ol") ||
                               elm.tagName().toLower() == QLatin1String("ul") ||
                               elm.tagName().toLower() == QLatin1String("li") );

    const bool anchorSelected = (elm.tagName().toLower() == QLatin1String("a"));

    qDebug()<<" elm.tagName().toLower() "<<elm.tagName().toLower();

    KMenu *menu = new KMenu;
    const QString selectedText = page()->mainFrame()->toPlainText().simplified();
    const bool emptyDocument = selectedText.isEmpty();

    menu->addAction(page()->action(QWebPage::Undo));
    menu->addAction(page()->action(QWebPage::Redo));
    menu->addSeparator();
    menu->addAction(page()->action(QWebPage::Cut));
    menu->addAction(page()->action(QWebPage::Copy));
    menu->addAction(page()->action(QWebPage::Paste));
    if (d->action_paste_withoutformatting)
        menu->addAction(d->action_paste_withoutformatting);

    menu->addSeparator();
    if (!emptyDocument) {
        menu->addAction(page()->action(QWebPage::SelectAll));
        menu->addSeparator();
    }
    if (!emptyDocument && d->action_find) {
        menu->addAction(d->action_find);
        menu->addSeparator();
    }
    if (imageSelected) {
        QAction *editImageAction = menu->addAction(i18n("Edit Image..."));
        connect( editImageAction, SIGNAL(triggered(bool)), this, SLOT(_k_slotEditImage()) );
    } else if (linkSelected) {
        QAction *editLinkAction = menu->addAction(i18n("Edit Link..."));
        connect( editLinkAction, SIGNAL(triggered(bool)), this, SLOT(_k_slotEditLink()) );
        QAction *openLinkAction = menu->addAction(i18n("Open Link"));
        connect( openLinkAction, SIGNAL(triggered(bool)), this, SLOT(_k_slotOpenLink()) );
    } else if (tableSelected) {
        ComposerTableActionMenu * tableActionMenu = new ComposerTableActionMenu(elm,menu,this);
        connect(tableActionMenu, SIGNAL(insertNewTable()), this, SLOT(_k_slotInsertTable()));
        menu->addAction(tableActionMenu);
    } else if (listSelected) {
        QAction *editListAction = menu->addAction(i18n("Edit List..."));
        connect( editListAction, SIGNAL(triggered(bool)), this, SLOT(_k_slotEditList()) );
    }
    menu->addSeparator();
    if (!emptyDocument && d->action_spell_check) {
        menu->addAction(d->action_spell_check);
        menu->addSeparator();
    }
#if QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 3, 0)
    //Don't use "Auto Spell Check" it will confict with search menu entry in spellchecklineedit.
    QAction *autoSpellCheckingAction = menu->addAction(i18n("Enable Spell Checking"));
    autoSpellCheckingAction->setCheckable( true );
    autoSpellCheckingAction->setChecked( d->checkSpellingEnabled() );
    connect( autoSpellCheckingAction, SIGNAL(triggered(bool)), this, SLOT(_k_changeAutoSpellChecking(bool)) );
#endif
    QAction *speakAction = menu->addAction(i18n("Speak Text"));
    speakAction->setIcon(KIcon(QLatin1String("preferences-desktop-text-to-speech")));
    speakAction->setEnabled(!emptyDocument );
    connect( speakAction, SIGNAL(triggered(bool)), this, SLOT(_k_slotSpeakText()) );
    addExtraAction(menu);
    menu->exec(event->globalPos());
    delete menu;
}

void ComposerView::addExtraAction(QMenu *menu)
{
    Q_UNUSED(menu);
    //Redefine if necessary.
}

void ComposerView::setActionsEnabled(bool enabled)
{
    Q_FOREACH (QAction *action, d->htmlEditorActionList)
        action->setEnabled(enabled);
}

void ComposerView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        d->contextMenuResult = page()->mainFrame()->hitTestContent(event->pos());
        const bool imageSelected = !d->contextMenuResult.imageUrl().isEmpty();
        if (imageSelected) {
            d->showImageResizeWidget();
        }
    } else {
        d->hideImageResizeWidget();
    }
    KWebView::mousePressEvent(event);
}

void ComposerView::keyPressEvent(QKeyEvent *event)
{
    d->hideImageResizeWidget();
    KWebView::keyPressEvent(event);
}

void ComposerView::wheelEvent(QWheelEvent *event)
{
    d->hideImageResizeWidget();
    KWebView::wheelEvent(event);
}

void ComposerView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        d->contextMenuResult = page()->mainFrame()->hitTestContent(event->pos());
        const bool imageSelected = !d->contextMenuResult.imageUrl().isEmpty();
        if (imageSelected) {
            d->showImageResizeWidget();
            d->_k_slotEditImage();
        }
    } else {
        d->hideImageResizeWidget();
    }
    KWebView::mouseDoubleClickEvent(event);
}

void ComposerView::setHtmlContent( const QString &html )
{
    setHtml(html);
}

void ComposerView::evaluateJavascript( const QString &javascript)
{
    d->evaluateJavascript(javascript);
}

void ComposerView::createToolBar(const QList<ComposerViewAction> &lstAction, KToolBar *toolbar)
{
    const int numberOfAction(lstAction.count());
    for (int i=0;i<numberOfAction;++i) {
        switch(lstAction.at(i)) {
        case Bold:
            toolbar->addAction(d->action_text_bold);
            break;
        case Italic:
            toolbar->addAction(d->action_text_italic);
            break;
        case Underline:
            toolbar->addAction(d->action_text_underline);
            break;
        case StrikeOut:
            toolbar->addAction(d->action_text_strikeout);
            break;
        case AlignLeft:
            toolbar->addAction(d->action_align_left);
            break;
        case AlignCenter:
            toolbar->addAction(d->action_align_center);
            break;
        case AlignRight:
            toolbar->addAction(d->action_align_right);
            break;
        case AlignJustify:
            toolbar->addAction(d->action_align_justify);
            break;
        case DirectionLtr:
            toolbar->addAction(d->action_direction_ltr);
            break;
        case DirectionRtl:
            toolbar->addAction(d->action_direction_rtl);
            break;
        case SubScript:
            toolbar->addAction(d->action_text_subscript);
            break;
        case SuperScript:
            toolbar->addAction(d->action_text_superscript);
            break;
        case HorizontalRule:
            toolbar->addAction(d->action_insert_horizontal_rule);
            break;
        case ListIndent:
            toolbar->addAction(d->action_list_indent);
            break;
        case ListDedent:
            toolbar->addAction(d->action_list_dedent);
            break;
        case OrderedList:
            toolbar->addAction(d->action_ordered_list);
            break;
        case UnorderedList:
            toolbar->addAction(d->action_unordered_list);
            break;
        case FormatType:
            toolbar->addAction(d->action_format_type);
            break;
        case FontSize:
            toolbar->addAction(d->action_font_size);
            break;
        case FontFamily:
            toolbar->addAction(d->action_font_family);
            break;
        case Emoticon:
            toolbar->addAction(d->action_add_emoticon);
            break;
        case InsertHtml:
            toolbar->addAction(d->action_insert_html);
            break;
        case InsertImage:
            toolbar->addAction(d->action_insert_image);
            break;
        case InsertTable:
            toolbar->addAction(d->action_insert_table);
            break;
        case InsertLink:
            toolbar->addAction(d->action_insert_link);
            break;
        case InsertSpecialChar:
            toolbar->addAction(d->action_insert_specialchar);
            break;
        case TextForegroundColor:
            toolbar->addAction(d->action_text_foreground_color);
            break;
        case TextBackgroundColor:
            toolbar->addAction(d->action_text_background_color);
            break;
        case FormatReset:
            toolbar->addAction(d->action_format_reset);
            break;
        case SpellCheck:
            toolbar->addAction(d->action_spell_check);
            break;
        case Find:
            toolbar->addAction(d->action_find);
            break;
        case Replace:
            toolbar->addAction(d->action_replace);
            break;
        case PageColor:
            toolbar->addAction(d->action_page_color);
            break;
        case BlockQuote:
            toolbar->addAction(d->action_block_quote);
            break;
        case SaveAs:
            toolbar->addAction(d->action_save_as);
            break;
        case Print:
            toolbar->addAction(d->action_print);
            break;
        case PrintPreview:
            toolbar->addAction(d->action_print_preview);
            break;
        case Separator: {
            QAction *act = new QAction(this);
            act->setSeparator(true);
            toolbar->addAction(act);
            break;
        }
        case PasteWithoutFormatting: {
            toolbar->addAction(d->action_paste_withoutformatting);
            break;
        }
        case InsertAnchor: {
            toolbar->addAction(d->action_insert_anchor);
            break;
        }
        case LastType: {
            //nothing
            break;
        }
        }
    }
}

QAction *ComposerView::action(ComposerViewAction actionType) const
{
    switch(actionType) {
    case Bold:
        return d->action_text_bold;
    case Italic:
        return d->action_text_italic;
    case Underline:
        return d->action_text_underline;
    case StrikeOut:
        return d->action_text_strikeout;
    case AlignLeft:
        return d->action_align_left;
    case AlignCenter:
        return d->action_align_center;
    case AlignRight:
        return d->action_align_right;
    case AlignJustify:
        return d->action_align_justify;
    case DirectionLtr:
        return d->action_direction_ltr;
    case DirectionRtl:
        return d->action_direction_rtl;
    case SubScript:
        return d->action_text_subscript;
    case SuperScript:
        return d->action_text_superscript;
    case HorizontalRule:
        return d->action_insert_horizontal_rule;
    case ListIndent:
        return d->action_list_indent;
    case ListDedent:
        return d->action_list_dedent;
    case OrderedList:
        return d->action_ordered_list;
    case UnorderedList:
        return d->action_unordered_list;
    case FormatType:
        return d->action_format_type;
    case FontSize:
        return d->action_font_size;
    case FontFamily:
        return d->action_font_family;
    case Emoticon:
        return d->action_add_emoticon;
    case InsertHtml:
        return d->action_insert_html;
    case InsertImage:
        return d->action_insert_image;
    case InsertTable:
        return d->action_insert_table;
    case InsertLink:
        return d->action_insert_link;
    case InsertSpecialChar:
        return d->action_insert_specialchar;
    case TextForegroundColor:
        return d->action_text_foreground_color;
    case TextBackgroundColor:
        return d->action_text_background_color;
    case FormatReset:
        return d->action_format_reset;
    case SpellCheck:
        return d->action_spell_check;
    case Find:
        return d->action_find;
    case Replace:
        return d->action_replace;
    case PageColor:
        return d->action_page_color;
    case BlockQuote:
        return d->action_block_quote;
    case SaveAs:
        return d->action_save_as;
    case Print:
        return d->action_print;
    case PrintPreview:
        return d->action_print_preview;
    case PasteWithoutFormatting:
        return d->action_paste_withoutformatting;
    case InsertAnchor:
        return d->action_insert_anchor;
    case Separator:
    case LastType:
        break;
    }
    return 0;
}

QMap<QString, QString> ComposerView::localImages() const
{
    return d->localImages();
}


}

#include "moc_composerview.cpp"
