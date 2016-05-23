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

#include "composerwebengine_p.h"
#include "table/composertabledialog.h"
#include "utils/composereditorutils_p.h"
#include "globalsetting_composereditorwebengine.h"
#if 0
#include "list/composerlistdialog.h"
#include "image/composerimageresizewidget.h"
#include "pagecolor/pagecolorbackgrounddialog.h"
#include "helper/listhelper_p.h"
#endif

#include <kpimtextedit/emoticontexteditaction.h>
#include <kpimtextedit/inserthtmldialog.h>
#include <kpimtextedit/selectspecialchardialog.h>
#include "kpimtextedit/texttospeech.h"

#include <Sonnet/Dialog>
#include <sonnet/backgroundchecker.h>

#include <KLocalizedString>
#include <KToggleAction>
#include <QAction>
#include <KSelectAction>
#include <QColorDialog>
#include <KMessageBox>
#include <QApplication>
#include "composereditorwebengine_debug.h"
#include <KFontAction>
#include <KRun>
#include <QUrl>
#include <QIcon>
#include <KStandardShortcut>

#include <PimCommon/KPimPrintPreviewDialog>
#include <QDBusConnectionInterface>
#include <image/composerimagedialog.h>
#include <link/composeranchordialog.h>
#include <link/composerlinkdialog.h>
#include <QPointer>
#include <QPrinter>
#include <QPrintDialog>
#include <QClipboard>
#include <QFileDialog>

namespace ComposerEditorWebEngine
{
ComposerEditorWebEnginePrivate::ComposerEditorWebEnginePrivate(ComposerEditorWebEngine::ComposerWebEngine *qq)
    : action_text_bold(Q_NULLPTR),
      action_text_italic(Q_NULLPTR),
      action_text_underline(Q_NULLPTR),
      action_text_strikeout(Q_NULLPTR),
      action_align_left(Q_NULLPTR),
      action_align_center(Q_NULLPTR),
      action_align_right(Q_NULLPTR),
      action_align_justify(Q_NULLPTR),
      action_direction_ltr(Q_NULLPTR),
      action_direction_rtl(Q_NULLPTR),
      action_text_subscript(Q_NULLPTR),
      action_text_superscript(Q_NULLPTR),
      action_insert_horizontal_rule(Q_NULLPTR),
      action_list_indent(Q_NULLPTR),
      action_list_dedent(Q_NULLPTR),
      action_ordered_list(Q_NULLPTR),
      action_unordered_list(Q_NULLPTR),
      action_format_type(Q_NULLPTR),
      action_font_size(Q_NULLPTR),
      action_font_family(Q_NULLPTR),
      action_add_emoticon(Q_NULLPTR),
      action_insert_html(Q_NULLPTR),
      action_insert_image(Q_NULLPTR),
      action_insert_table(Q_NULLPTR),
      action_text_foreground_color(Q_NULLPTR),
      action_text_background_color(Q_NULLPTR),
      action_format_reset(Q_NULLPTR),
      action_insert_link(Q_NULLPTR),
      action_spell_check(Q_NULLPTR),
      action_find(Q_NULLPTR),
      action_replace(Q_NULLPTR),
      action_page_color(Q_NULLPTR),
      action_block_quote(Q_NULLPTR),
      action_save_as(Q_NULLPTR),
      action_print(Q_NULLPTR),
      action_print_preview(Q_NULLPTR),
      action_paste_withoutformatting(Q_NULLPTR),
      action_insert_specialchar(Q_NULLPTR),
      action_insert_anchor(Q_NULLPTR),
      q(qq),
      imageResizeWidget(Q_NULLPTR),
      mPageEngine(Q_NULLPTR)
{
}
#define FOLLOW_CHECK(a1, a2) a1->setChecked(getAction(a2)->isChecked())

void ComposerEditorWebEnginePrivate::createAction(ComposerWebEngine::ComposerWebEngineAction type)
{
    switch (type) {
    case ComposerWebEngine::Bold: {
        if (!action_text_bold) {
            action_text_bold = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-bold")), i18nc("@action boldify selected text", "&Bold"), q);
            QFont bold;
            bold.setBold(true);
            action_text_bold->setFont(bold);
            action_text_bold->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
            htmlEditorActionList.append(action_text_bold);
            q->connect(action_text_bold, SIGNAL(triggered(bool)), SLOT(_k_slotBold(bool)));
        }
        break;
    }
    case ComposerWebEngine::Italic: {
        if (!action_text_italic) {
            action_text_italic = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-italic")), i18nc("@action italicize selected text", "&Italic"), q);
            QFont italic;
            italic.setItalic(true);
            action_text_italic->setFont(italic);
            action_text_italic->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
            q->connect(action_text_italic, SIGNAL(triggered(bool)), SLOT(_k_slotItalic(bool)));
            htmlEditorActionList.append(action_text_italic);
        }
        break;
    }
    case ComposerWebEngine::Underline: {
        if (!action_text_underline) {
            action_text_underline = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-underline")), i18nc("@action underline selected text", "&Underline"), q);
            QFont underline;
            underline.setUnderline(true);
            action_text_underline->setFont(underline);
            action_text_underline->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
            q->connect(action_text_underline, SIGNAL(triggered(bool)), SLOT(_k_slotUnderline(bool)));
            htmlEditorActionList.append(action_text_underline);
        }
        break;
    }
    case ComposerWebEngine::StrikeOut: {
        if (!action_text_strikeout) {
            action_text_strikeout = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-strikethrough")), i18nc("@action", "&Strike Out"), q);
            action_text_strikeout->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
            q->connect(action_text_strikeout, SIGNAL(triggered(bool)), SLOT(_k_slotStrikeout(bool)));
            htmlEditorActionList.append(action_text_strikeout);
        }
        break;
    }
    case ComposerWebEngine::AlignLeft: {
        if (!action_align_left) {
            action_align_left = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-left")), i18nc("@action", "Align &Left"), q);
            action_align_left->setIconText(i18nc("@label left justify", "Left"));
            htmlEditorActionList.append((action_align_left));
            q->connect(action_align_left, SIGNAL(triggered(bool)), SLOT(_k_slotJustifyLeft(bool)));
        }
        break;
    }
    case ComposerWebEngine::AlignCenter: {
        if (!action_align_center) {
            action_align_center = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-center")), i18nc("@action", "Align &Center"), q);
            action_align_center->setIconText(i18nc("@label center justify", "Center"));
            htmlEditorActionList.append((action_align_center));
            q->connect(action_align_center, SIGNAL(triggered(bool)), SLOT(_k_slotJustifyCenter(bool)));
        }
        break;
    }
    case ComposerWebEngine::AlignRight: {
        if (!action_align_right) {
            action_align_right = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-right")), i18nc("@action", "Align &Right"), q);
            action_align_right->setIconText(i18nc("@label right justify", "Right"));
            htmlEditorActionList.append((action_align_right));
            q->connect(action_align_right, SIGNAL(triggered(bool)), SLOT(_k_slotJustifyRight(bool)));
        }
        break;
    }
    case ComposerWebEngine::AlignJustify: {
        if (!action_align_justify) {
            action_align_justify = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-fill")), i18nc("@action", "&Justify"), q);
            action_align_justify->setIconText(i18nc("@label justify fill", "Justify"));
            htmlEditorActionList.append((action_align_justify));
            q->connect(action_align_justify, SIGNAL(triggered(bool)), SLOT(_k_slotJustifyFull(bool)));
        }
        break;
    }
    case ComposerWebEngine::DirectionLtr: {
        if (!action_direction_ltr) {
            action_direction_ltr = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-direction-ltr")), i18nc("@action", "Left-to-Right"), q);
            action_direction_ltr->setIconText(i18nc("@label left-to-right", "Left-to-Right"));
            htmlEditorActionList.append(action_direction_ltr);
            q->connect(action_direction_ltr, SIGNAL(triggered(bool)), SLOT(_k_slotDirectionLtr()));
        }
        break;
    }
    case ComposerWebEngine::DirectionRtl: {
        if (!action_direction_rtl) {
            action_direction_rtl = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-direction-rtl")), i18nc("@action", "Right-to-Left"), q);
            action_direction_rtl->setIconText(i18nc("@label right-to-left", "Right-to-Left"));
            htmlEditorActionList.append(action_direction_rtl);
            q->connect(action_direction_rtl, SIGNAL(triggered(bool)), SLOT(_k_slotDirectionRtl()));
        }
        break;
    }
    case ComposerWebEngine::SubScript: {
        if (!action_text_subscript) {
            action_text_subscript = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-subscript")), i18nc("@action", "Subscript"), q);
            htmlEditorActionList.append((action_text_subscript));
            q->connect(action_text_subscript, SIGNAL(triggered(bool)), SLOT(_k_slotSubscript(bool)));
        }
        break;
    }
    case ComposerWebEngine::SuperScript: {
        if (!action_text_superscript) {
            action_text_superscript = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-superscript")), i18nc("@action", "Superscript"), q);
            htmlEditorActionList.append((action_text_superscript));
            q->connect(action_text_superscript, SIGNAL(triggered(bool)), SLOT(_k_slotSuperscript(bool)));
        }
        break;
    }
    case ComposerWebEngine::HorizontalRule: {
        if (!action_insert_horizontal_rule) {
            action_insert_horizontal_rule = new QAction(QIcon::fromTheme(QStringLiteral("insert-horizontal-rule")), i18nc("@action", "Insert Rule Line"), q);
            htmlEditorActionList.append((action_insert_horizontal_rule));
            q->connect(action_insert_horizontal_rule, SIGNAL(triggered(bool)), SLOT(_k_slotInsertHorizontalRule()));
        }
        break;
    }
    case ComposerWebEngine::ListIndent: {
        if (!action_list_indent) {
            action_list_indent = new QAction(QIcon::fromTheme(QStringLiteral("format-indent-more")), i18nc("@action", "Increase Indent"), q);
            htmlEditorActionList.append((action_list_indent));
            q->connect(action_list_indent, SIGNAL(triggered(bool)), SLOT(_k_slotListIndent()));
        }
        break;
    }
    case ComposerWebEngine::ListDedent: {
        if (!action_list_dedent) {
            action_list_dedent = new QAction(QIcon::fromTheme(QStringLiteral("format-indent-less")), i18nc("@action", "Decrease Indent"), q);
            htmlEditorActionList.append(action_list_dedent);
            q->connect(action_list_dedent, SIGNAL(triggered(bool)), SLOT(_k_slotListDedent()));
        }
        break;
    }
    case ComposerWebEngine::OrderedList: {
        if (!action_ordered_list) {
            action_ordered_list = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-list-ordered")), i18n("Ordered Style"), q);
            htmlEditorActionList.append(action_ordered_list);
            q->connect(action_ordered_list, SIGNAL(triggered(bool)), SLOT(_k_slotOrderedList(bool)));
        }
        break;
    }
    case ComposerWebEngine::UnorderedList: {
        if (!action_unordered_list) {
            action_unordered_list = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-list-unordered")), i18n("Unordered List"), q);
            htmlEditorActionList.append(action_unordered_list);
            q->connect(action_unordered_list, SIGNAL(triggered(bool)), SLOT(_k_slotUnOrderedList(bool)));
        }
        break;
    }
    case ComposerWebEngine::FormatType: {
        if (!action_format_type) {
            action_format_type = new KSelectAction(QIcon::fromTheme(QStringLiteral("format-list-unordered")), i18nc("@title:menu", "List Style"), q);
            QAction *act = action_format_type->addAction(i18n("Paragraph"));
            act->setData(QVariant::fromValue(ComposerEditorWebEnginePrivate::Paragraph));
            act = action_format_type->addAction(i18n("Heading 1"));
            act->setData(QVariant::fromValue(ComposerEditorWebEnginePrivate::Header1));
            act = action_format_type->addAction(i18n("Heading 2"));
            act->setData(QVariant::fromValue(ComposerEditorWebEnginePrivate::Header2));
            act = action_format_type->addAction(i18n("Heading 3"));
            act->setData(QVariant::fromValue(ComposerEditorWebEnginePrivate::Header3));
            act = action_format_type->addAction(i18n("Heading 4"));
            act->setData(QVariant::fromValue(ComposerEditorWebEnginePrivate::Header4));
            act = action_format_type->addAction(i18n("Heading 5"));
            act->setData(QVariant::fromValue(ComposerEditorWebEnginePrivate::Header5));
            act = action_format_type->addAction(i18n("Heading 6"));
            act->setData(QVariant::fromValue(ComposerEditorWebEnginePrivate::Header6));
            act = action_format_type->addAction(i18n("Pre"));
            act->setData(QVariant::fromValue(ComposerEditorWebEnginePrivate::Pre));
            act = action_format_type->addAction(i18n("Address"));
            act->setData(QVariant::fromValue(ComposerEditorWebEnginePrivate::Address));
            action_format_type->setCurrentItem(0);
            htmlEditorActionList.append(action_format_type);

            q->connect(action_format_type, SIGNAL(triggered(QAction*)),
                       q, SLOT(_k_setFormatType(QAction*)));
        }
        break;
    }
    case ComposerWebEngine::FontSize: {
        if (!action_font_size) {
            action_font_size = new KSelectAction(i18nc("@action", "Font &Size"), q);
            htmlEditorActionList.append(action_font_size);
            QStringList sizes;
            sizes << QStringLiteral("xx-small");
            sizes << QStringLiteral("x-small");
            sizes << QStringLiteral("small");
            sizes << QStringLiteral("medium");
            sizes << QStringLiteral("large");
            sizes << QStringLiteral("x-large");
            sizes << QStringLiteral("xx-large");
            action_font_size->setItems(sizes);
            action_font_size->setCurrentItem(0);
            q->connect(action_font_size, SIGNAL(triggered(int)), q, SLOT(_k_setFontSize(int)));
        }
        break;
    }
    case ComposerWebEngine::FontFamily: {
        if (!action_font_family) {
            action_font_family = new KFontAction(i18nc("@action", "&Font"), q);
            htmlEditorActionList.append((action_font_family));
            q->connect(action_font_family, SIGNAL(triggered(QString)), q, SLOT(_k_setFontFamily(QString)));
        }
        break;
    }
    case ComposerWebEngine::Emoticon: {
        if (!action_add_emoticon) {
            action_add_emoticon = new KPIMTextEdit::EmoticonTextEditAction(q);
            q->connect(action_add_emoticon, SIGNAL(emoticonActivated(QString)),
                       q, SLOT(_k_slotAddEmoticon(QString)));
        }
        break;
    }
    case ComposerWebEngine::InsertImage: {
        if (!action_insert_image) {
            action_insert_image = new QAction(QIcon::fromTheme(QStringLiteral("insert-image")), i18n("Add Image"), q);
            q->connect(action_insert_image, SIGNAL(triggered(bool)), SLOT(_k_slotAddImage()));
        }
        break;
    }
    case ComposerWebEngine::InsertHtml: {
        if (!action_insert_html) {
            action_insert_html = new QAction(i18n("Insert HTML"), q);
            q->connect(action_insert_html, SIGNAL(triggered(bool)), SLOT(_k_slotInsertHtml()));
        }
        break;
    }
    case ComposerWebEngine::InsertTable: {
        if (!action_insert_table) {
            action_insert_table = new QAction(QIcon::fromTheme(QStringLiteral("insert-table")), i18n("Table..."), q);
            htmlEditorActionList.append(action_insert_table);
            q->connect(action_insert_table, SIGNAL(triggered(bool)), SLOT(_k_slotInsertTable()));
        }
        break;
    }
    case ComposerWebEngine::InsertLink: {
        if (!action_insert_link) {
            action_insert_link = new QAction(QIcon::fromTheme(QStringLiteral("insert-link")), i18nc("@action", "Link"), q);
            htmlEditorActionList.append(action_insert_link);
            q->connect(action_insert_link, SIGNAL(triggered(bool)), q, SLOT(_k_insertLink()));
        }
        break;
    }
    case ComposerWebEngine::TextForegroundColor: {
        if (!action_text_foreground_color) {
            action_text_foreground_color = new QAction(QIcon::fromTheme(QStringLiteral("format-stroke-color")), i18nc("@action", "Text &Color..."), q);
            action_text_foreground_color->setIconText(i18nc("@label stroke color", "Color"));
            htmlEditorActionList.append((action_text_foreground_color));
            q->connect(action_text_foreground_color, SIGNAL(triggered()), q, SLOT(_k_setTextForegroundColor()));
        }
        break;
    }
    case ComposerWebEngine::TextBackgroundColor: {
        if (!action_text_background_color) {
            action_text_background_color = new QAction(QIcon::fromTheme(QStringLiteral("format-fill-color")), i18nc("@action", "Text &Highlight..."), q);
            htmlEditorActionList.append((action_text_background_color));
            q->connect(action_text_background_color, SIGNAL(triggered()), q, SLOT(_k_setTextBackgroundColor()));
        }
        break;
    }
    case ComposerWebEngine::FormatReset: {
        if (!action_format_reset) {
            action_format_reset = new QAction(QIcon::fromTheme(QStringLiteral("draw-eraser")), i18n("Reset Font Settings"), q);
            q->connect(action_format_reset, SIGNAL(triggered()), q, SLOT(_k_slotResetFormat()));
        }
        break;
    }
    case ComposerWebEngine::SpellCheck: {
        if (!action_spell_check) {
            action_spell_check = new QAction(QIcon::fromTheme(QStringLiteral("tools-check-spelling")), i18n("Check Spelling..."), q);
            htmlEditorActionList.append(action_spell_check);
            q->connect(action_spell_check, SIGNAL(triggered(bool)), q, SLOT(_k_slotSpellCheck()));
        }
        break;
    }
    case ComposerWebEngine::PageColor: {
        if (!action_page_color) {
            action_page_color = new QAction(i18n("Page Color and Background..."), q);
            htmlEditorActionList.append(action_page_color);
            q->connect(action_page_color, SIGNAL(triggered(bool)), SLOT(_k_slotChangePageColorAndBackground()));
        }
        break;
    }
    case ComposerWebEngine::BlockQuote: {
        if (!action_block_quote) {
            action_block_quote = new QAction(QIcon::fromTheme(QStringLiteral("format-text-blockquote")), i18n("Blockquote"), q);
            htmlEditorActionList.append(action_block_quote);
            q->connect(action_block_quote, SIGNAL(triggered()), q, SLOT(_k_slotToggleBlockQuote()));
        }
        break;
    }
    case ComposerWebEngine::Find: {
        if (!action_find) {
            action_find = new QAction(QIcon::fromTheme(QStringLiteral("edit-find")), i18n("&Find..."), q);
            action_find->setShortcut(KStandardShortcut::find().first());
            htmlEditorActionList.append(action_find);
            q->connect(action_find, SIGNAL(triggered()), q, SLOT(_k_slotFind()));
        }
        break;
    }
    case ComposerWebEngine::Replace: {
        if (!action_replace) {
            action_replace = new QAction(QIcon::fromTheme(QStringLiteral("edit-replace")), i18n("&Replace..."), q);
            htmlEditorActionList.append(action_replace);
            action_replace->setShortcut(KStandardShortcut::replace().first());
            q->connect(action_replace, SIGNAL(triggered()), q, SLOT(_k_slotReplace()));
        }
        break;
    }
    case ComposerWebEngine::SaveAs: {
        if (!action_save_as) {
            action_save_as = new QAction(QIcon::fromTheme(QStringLiteral("file_save_as")), i18n("Save &As..."), q);
            htmlEditorActionList.append(action_save_as);
            action_replace->setShortcut(KStandardShortcut::save().first());
            q->connect(action_save_as, SIGNAL(triggered()), q, SLOT(_k_slotSaveAs()));
        }
        break;
    }
    case ComposerWebEngine::Print: {
        if (!action_print) {
            action_print = new QAction(QIcon::fromTheme(QStringLiteral("file_print")), i18n("&Print..."), q);
            htmlEditorActionList.append(action_print);
            action_replace->setShortcut(KStandardShortcut::print().first());
            q->connect(action_print, SIGNAL(triggered()), q, SLOT(_k_slotPrint()));
        }
        break;
    }
    case ComposerWebEngine::PrintPreview: {
        if (!action_print_preview) {
            action_print_preview = new QAction(QIcon::fromTheme(QStringLiteral("file_print_preview")), i18n("Print Previe&w"), q);
            htmlEditorActionList.append(action_print_preview);
            q->connect(action_print_preview, SIGNAL(triggered()), q, SLOT(_k_slotPrintPreview()));
        }
        break;
    }
    case ComposerWebEngine::PasteWithoutFormatting: {
        if (!action_paste_withoutformatting) {
            action_paste_withoutformatting = new QAction(i18n("Paste Without Formatting"), q);
            htmlEditorActionList.append(action_paste_withoutformatting);
            q->connect(action_paste_withoutformatting, SIGNAL(triggered()), q, SLOT(_k_slotPasteWithoutFormatting()));
        }
        break;
    }
    case ComposerWebEngine::InsertSpecialChar: {
        if (!action_insert_specialchar) {
            action_insert_specialchar = new QAction(i18n("Insert Special Char..."), q);
            htmlEditorActionList.append(action_insert_specialchar);
            q->connect(action_insert_specialchar, SIGNAL(triggered()), q, SLOT(_k_slotInsertSpecialChar()));
        }
        break;
    }
    case ComposerWebEngine::InsertAnchor: {
        if (!action_insert_anchor) {
            action_insert_anchor = new QAction(i18n("Insert Anchor..."), q);
            htmlEditorActionList.append(action_insert_anchor);
            q->connect(action_insert_anchor, SIGNAL(triggered()), q, SLOT(_k_slotInsertAnchor()));
        }
        break;
    }
    case ComposerWebEngine::Separator:
        //nothing
        break;
    case ComposerWebEngine::LastType:
        //nothing
        break;
    }

}

void ComposerEditorWebEnginePrivate::connectActionGroup()
{
    if (action_align_left || action_align_center || action_align_right || action_align_justify) {
        QActionGroup *alignmentGroup = new QActionGroup(q);
        if (action_align_left) {
            alignmentGroup->addAction(action_align_left);
        }
        if (action_align_center) {
            alignmentGroup->addAction(action_align_center);
        }
        if (action_align_right) {
            alignmentGroup->addAction(action_align_right);
        }
        if (action_align_justify) {
            alignmentGroup->addAction(action_align_justify);
        }
    }

    if (action_direction_ltr && action_direction_rtl) {
        QActionGroup *directionGroup = new QActionGroup(q);
        directionGroup->addAction(action_direction_ltr);
        directionGroup->addAction(action_direction_rtl);
    }
}

bool ComposerEditorWebEnginePrivate::checkSpellingEnabled()
{
    return ComposerEditorWebEngine::GlobalSettingsBase::autoSpellChecking();
}

void ComposerEditorWebEnginePrivate::_k_changeAutoSpellChecking(bool checked)
{
    ComposerEditorWebEngine::GlobalSettingsBase::setAutoSpellChecking(checked);
}

QAction *ComposerEditorWebEnginePrivate::getAction(QWebEnginePage::WebAction action) const
{
    if (action >= 0 && action <= 66) {
        return q->page()->action(static_cast<QWebEnginePage::WebAction>(action));
    } else {
        return 0;
    }
}

void ComposerEditorWebEnginePrivate::hideImageResizeWidget()
{
    delete imageResizeWidget;
    imageResizeWidget = 0;
}

void ComposerEditorWebEnginePrivate::showImageResizeWidget()
{
#if 0
    if (!imageResizeWidget) {
        imageResizeWidget = new ComposerImageResizeWidget(contextMenuResult.element(), q);
        imageResizeWidget->move(contextMenuResult.element().geometry().topLeft());
        imageResizeWidget->show();
    }
#endif
}

void ComposerEditorWebEnginePrivate::_k_setFormatType(QAction *act)
{
    if (!act) {
        return;
    }
    QString command;
    switch (act->data().value<ComposerEditorWebEngine::ComposerEditorWebEnginePrivate::FormatType>()) {
    case Paragraph:
        command = QStringLiteral("p");
        break;
    case Header1:
        command = QStringLiteral("h1");
        break;
    case Header2:
        command = QStringLiteral("h2");
        break;
    case Header3:
        command = QStringLiteral("h3");
        break;
    case Header4:
        command = QStringLiteral("h4");
        break;
    case Header5:
        command = QStringLiteral("h5");
        break;
    case Header6:
        command = QStringLiteral("h6");
        break;
    case Pre:
        command = QStringLiteral("pre");
        break;
    case Address:
        command = QStringLiteral("address");
        break;
    }
    execCommand(QStringLiteral("formatBlock"), command);
}

void ComposerEditorWebEnginePrivate::_k_slotToggleBlockQuote()
{
    execCommand(QStringLiteral("formatBlock"), QStringLiteral("BLOCKQUOTE"));
}

void ComposerEditorWebEnginePrivate::_k_slotAddEmoticon(const QString &emoticon)
{
    execCommand(QStringLiteral("insertHTML"), emoticon);
}

void ComposerEditorWebEnginePrivate::_k_slotInsertHtml()
{
    QPointer<KPIMTextEdit::InsertHtmlDialog> dialog = new KPIMTextEdit::InsertHtmlDialog(q);
    if (dialog->exec()) {
        const QString str = dialog->html().remove(QStringLiteral("\n"));
        if (!str.isEmpty()) {
            execCommand(QStringLiteral("insertHTML"), str);
        }
    }
    delete dialog;
}

void ComposerEditorWebEnginePrivate::_k_setTextBackgroundColor()
{
    QColor newColor = ComposerEditorWebEngine::Utils::convertRgbToQColor(evaluateJavascript(QStringLiteral("getTextBackgroundColor()")).toString());
    newColor = QColorDialog::getColor(newColor, q);
    if (newColor.isValid()) {
        execCommand(QStringLiteral("hiliteColor"), newColor.name());
    }
}

QVariant ComposerEditorWebEnginePrivate::evaluateJavascript(const QString &command)
{
    qDebug() << " QVariant ComposerEditorWebEnginePrivate::evaluateJavascript(const QString &command)" << command;
    q->page()->runJavaScript(command);
    return QVariant();
    //TODO fix me return value.
}

void ComposerEditorWebEnginePrivate::_k_slotDeleteText()
{
    evaluateJavascript(QStringLiteral("setDeleteSelectedText()"));
}

void ComposerEditorWebEnginePrivate::_k_setTextForegroundColor()
{
    QColor newColor = ComposerEditorWebEngine::Utils::convertRgbToQColor(evaluateJavascript(QStringLiteral("getTextForegroundColor()")).toString());
    newColor = QColorDialog::getColor(newColor, q);
    if (newColor.isValid()) {
        execCommand(QStringLiteral("foreColor"), newColor.name());
    }
}

void ComposerEditorWebEnginePrivate::_k_slotAddImage()
{
    QPointer<ComposerEditorWebEngine::ComposerImageDialog> dlg = new ComposerEditorWebEngine::ComposerImageDialog(q);
    if (dlg->exec() == QDialog::Accepted) {
        execCommand(QStringLiteral("insertHTML"), dlg->html());
    }
    delete dlg;
}

void ComposerEditorWebEnginePrivate::_k_slotEditImage()
{
#if 0
    showImageResizeWidget();
    ComposerImageDialog dlg(contextMenuResult.element(), q);
    dlg.exec();
#endif
}

void ComposerEditorWebEnginePrivate::_k_slotInsertTable()
{
    QPointer<ComposerEditorWebEngine::ComposerTableDialog> dlg = new ComposerEditorWebEngine::ComposerTableDialog(q);
    if (dlg->exec() == QDialog::Accepted) {
        execCommand(QStringLiteral("insertHTML"), dlg->html());
    }
    delete dlg;
}

void ComposerEditorWebEnginePrivate::_k_slotInsertHorizontalRule()
{
    execCommand(QStringLiteral("insertHorizontalRule"));
}

void ComposerEditorWebEnginePrivate::_k_insertLink()
{
    const QString selectedText = q->selectedText();
    QPointer<ComposerEditorWebEngine::ComposerLinkDialog> dlg = new ComposerEditorWebEngine::ComposerLinkDialog(selectedText, q);
    if (dlg->exec() == QDialog::Accepted) {
        const QString html(dlg->html());
        if (!html.isEmpty()) {
            execCommand(QStringLiteral("insertHTML"), html);
        }
    }
    delete dlg;
}

void ComposerEditorWebEnginePrivate::_k_slotEditLink()
{
#if 0
    ComposerWebEngine::ComposerLinkDialog dlg(contextMenuResult.linkElement(), q);
    dlg.exec();
#endif
}

void ComposerEditorWebEnginePrivate::_k_slotOpenLink()
{
#if 0
    const QString href = contextMenuResult.linkElement().attribute(QStringLiteral("href"));
    if (!href.isEmpty()) {
        new KRun(QUrl(href), 0);
    }
#endif
}

inline QString convertBooleanToString(bool b)
{
    return b ? QStringLiteral("true") : QStringLiteral("false");
}

void ComposerEditorWebEnginePrivate::_k_slotBold(bool b)
{
    execCommand(QStringLiteral("bold"), convertBooleanToString(b));
}

void ComposerEditorWebEnginePrivate::_k_slotItalic(bool b)
{
    execCommand(QStringLiteral("italic"), convertBooleanToString(b));
}

void ComposerEditorWebEnginePrivate::_k_slotUnderline(bool b)
{
    execCommand(QStringLiteral("underline"), convertBooleanToString(b));
}

void ComposerEditorWebEnginePrivate::_k_slotStrikeout(bool b)
{
    execCommand(QStringLiteral("strikeThrough"), convertBooleanToString(b));
}

void ComposerEditorWebEnginePrivate::_k_slotSuperscript(bool b)
{
    execCommand(QStringLiteral("superscript"), convertBooleanToString(b));
}

void ComposerEditorWebEnginePrivate::_k_slotJustifyLeft(bool b)
{
    execCommand(QStringLiteral("justifyLeft"), convertBooleanToString(b));
}

void ComposerEditorWebEnginePrivate::_k_slotJustifyCenter(bool b)
{
    execCommand(QStringLiteral("justifyCenter"), convertBooleanToString(b));
}

void ComposerEditorWebEnginePrivate::_k_slotJustifyRight(bool b)
{
    execCommand(QStringLiteral("justifyRight"), convertBooleanToString(b));
}

void ComposerEditorWebEnginePrivate::_k_slotJustifyFull(bool b)
{
    execCommand(QStringLiteral("justifyFull"), convertBooleanToString(b));
}

void ComposerEditorWebEnginePrivate::_k_slotSubscript(bool b)
{
    execCommand(QStringLiteral("subscript"), convertBooleanToString(b));
}

void ComposerEditorWebEnginePrivate::_k_slotListIndent()
{
    execCommand(QStringLiteral("indent"));
}

void ComposerEditorWebEnginePrivate::_k_slotListDedent()
{
    execCommand(QStringLiteral("outdent"));
}

void ComposerEditorWebEnginePrivate::_k_slotOrderedList(bool b)
{
    execCommand(QStringLiteral("insertOrderedList"), convertBooleanToString(b));
}

void ComposerEditorWebEnginePrivate::_k_slotUnOrderedList(bool b)
{
    execCommand(QStringLiteral("insertUnorderedList"), convertBooleanToString(b));
}

void ComposerEditorWebEnginePrivate::_k_slotResetFormat()
{
    execCommand(QStringLiteral("removeFormat"));
}

void ComposerEditorWebEnginePrivate::_k_slotDirectionLtr()
{
    qDebug() << "void ComposerEditorWebEnginePrivate::_k_slotDirectionLtr() not implemented";
}

void ComposerEditorWebEnginePrivate::_k_slotDirectionRtl()
{
    qDebug() << "void ComposerEditorWebEnginePrivate::_k_slotDirectionRtl() not implemented";
}

void ComposerEditorWebEnginePrivate::_k_setFontSize(int fontSize)
{
    execCommand(QStringLiteral("fontSize"), QString::number(fontSize + 1)); //Verify
}

void ComposerEditorWebEnginePrivate::_k_setFontFamily(const QString &family)
{
    execCommand(QStringLiteral("fontName"), family);
}

void ComposerEditorWebEnginePrivate::_k_slotSpellCheck()
{
#if 0
#if 0
    QString text(execJScript(contextMenuResult.element(), QStringLiteral("this.value")).toString());
    if (contextMenuResult.isContentSelected()) {
        spellTextSelectionStart = qMax(0, execJScript(contextMenuResult.element(), QStringLiteral("this.selectionStart")).toInt());
        spellTextSelectionEnd = qMax(0, execJScript(contextMenuResult.element(), QStringLiteral("this.selectionEnd")).toInt());
        text = text.mid(spellTextSelectionStart, (spellTextSelectionEnd - spellTextSelectionStart));
    } else {
        spellTextSelectionStart = 0;
        spellTextSelectionEnd = 0;
    }
#endif

    if (text.isEmpty()) {
        return;
    }

    Sonnet::BackgroundChecker *backgroundSpellCheck = new Sonnet::BackgroundChecker;
    Sonnet::Dialog *spellDialog = new Sonnet::Dialog(backgroundSpellCheck, q);
    backgroundSpellCheck->setParent(spellDialog);
    spellDialog->setAttribute(Qt::WA_DeleteOnClose, true);

    q->connect(spellDialog, SIGNAL(replace(QString,int,QString)), q, SLOT(_k_spellCheckerCorrected(QString,int,QString)));
    q->connect(spellDialog, SIGNAL(misspelling(QString,int)), q, SLOT(_k_spellCheckerMisspelling(QString,int)));
    if (contextMenuResult.isContentSelected()) {
        q->connect(spellDialog, SIGNAL(done(QString)), q, SLOT(_k_slotSpellCheckDone(QString)));
    }
    spellDialog->setBuffer(text);
    spellDialog->show();
#endif
}

void ComposerEditorWebEnginePrivate::_k_spellCheckerCorrected(const QString &original, int pos, const QString &replacement)
{
    // Adjust the selection end...
    if (spellTextSelectionEnd > 0) {
        spellTextSelectionEnd += qMax(0, (replacement.length() - original.length()));
    }

    const int index = pos + spellTextSelectionStart;
    QString script(QStringLiteral("this.value=this.value.substring(0,"));
    script += QString::number(index);
    script += QStringLiteral(") + \"");
    QString w(replacement);
    script +=  w.replace(QLatin1Char('\''), QStringLiteral("\\\'")); // Escape any Quote marks in replacement word
    script += QStringLiteral("\" + this.value.substring(");
    script += QString::number(index + original.length());
    script += QStringLiteral(")");

    //qCDebug(COMPOSEREDITORNG_LOG) << "**** script:" << script;

    //execJScript(contextMenuResult.element(), script);
}

void ComposerEditorWebEnginePrivate::_k_spellCheckerMisspelling(const QString &text, int pos)
{
    // qCDebug(COMPOSEREDITORNG_LOG) << text << pos;
    QString selectionScript(QStringLiteral("this.setSelectionRange("));
    selectionScript += QString::number(pos + spellTextSelectionStart);
    selectionScript += QLatin1Char(',');
    selectionScript += QString::number(pos + text.length() + spellTextSelectionStart);
    selectionScript += QLatin1Char(')');
    //execJScript(contextMenuResult.element(), selectionScript);
}

void ComposerEditorWebEnginePrivate::_k_slotSpellCheckDone(const QString &)
{
    // Restore the text selection if one was present before we started the
    // spell check.
    if (spellTextSelectionStart > 0 || spellTextSelectionEnd > 0) {
        QString script(QStringLiteral("; this.setSelectionRange("));
        script += QString::number(spellTextSelectionStart);
        script += QLatin1Char(',');
        script += QString::number(spellTextSelectionEnd);
        script += QLatin1Char(')');
        //execJScript(contextMenuResult.element(), script);
    }
}

void ComposerEditorWebEnginePrivate::_k_slotFind()
{
    Q_EMIT q->showFindBar();
}

void ComposerEditorWebEnginePrivate::_k_slotReplace()
{
    //TODO
}

void ComposerEditorWebEnginePrivate::saveHtml(QWebEnginePage *page, const QString &fileName)
{
    if (page) {
        page->toHtml([fileName](const QString &result) {
            QFile file(fileName);
            bool success = file.open(QIODevice::WriteOnly);
            if (success) {
                // FIXME: here we always use UTF-8 encoding
                QByteArray data = result.toUtf8();
                const qint64 c = file.write(data);
            }
        }
        );
    }
}

void ComposerEditorWebEnginePrivate::_k_slotSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(q, i18nc("@title:window", "Save as"), QString(), i18n("HTML Files (*.htm *.html);;All Files (*)"));
    if (fn.isEmpty()) {
        KMessageBox::error(q, i18n("No file selected."), i18nc("@title:window", "Save as"));
        return;
    }
    if (!(fn.endsWith(QStringLiteral(".htm"), Qt::CaseInsensitive) ||
            fn.endsWith(QStringLiteral(".html"), Qt::CaseInsensitive))) {
        fn += QStringLiteral(".htm");
    }
    saveHtml(q->page(), fn);
}

void ComposerEditorWebEnginePrivate::_k_slotPrint()
{
    qDebug() << " void ComposerEditorWebEnginePrivate::_k_slotPrint() not implemented";
}

void ComposerEditorWebEnginePrivate::_k_slotPrintPreview()
{
    qDebug() << "void ComposerEditorWebEnginePrivate::_k_slotPrintPreview() not implemented";
}

void ComposerEditorWebEnginePrivate::_k_slotChangePageColorAndBackground()
{
#if 0
    const QWebElement element = q->page()->mainFrame()->findFirstElement(QStringLiteral("body"));
    if (!element.isNull()) {
        QPointer<PageColorBackgroundDialog> dlg = new PageColorBackgroundDialog(element, q);
        dlg->exec();
        delete dlg;
    }
#endif
}

void ComposerEditorWebEnginePrivate::_k_slotEditList()
{
#if 0
    QWebElement listElement = ListHelper::listElement(contextMenuResult.element());
    if (!listElement.isNull()) {
        QPointer<ComposerListDialog> dlg = new ComposerListDialog(listElement, q);
        if (dlg->exec()) {
            //TODO
        }
        delete dlg;
    }
#endif
}

void ComposerEditorWebEnginePrivate::_k_slotAdjustActions()
{
    if (action_text_bold) {
        //FOLLOW_CHECK(action_text_bold, QWebPage::ToggleBold);
    }
    if (action_text_italic) {
        //FOLLOW_CHECK(action_text_italic, QWebPage::ToggleItalic);
    }
    if (action_text_strikeout) {
        //FOLLOW_CHECK(action_text_strikeout, QWebPage::ToggleStrikethrough);
    }
    if (action_text_underline) {
        //FOLLOW_CHECK(action_text_underline, QWebPage::ToggleUnderline);
    }
    if (action_text_subscript) {
        //FOLLOW_CHECK(action_text_subscript, QWebPage::ToggleSubscript);
    }
    if (action_text_superscript) {
        //FOLLOW_CHECK(action_text_superscript, QWebPage::ToggleSuperscript);
    }
    if (action_ordered_list) {
        //FOLLOW_CHECK(action_ordered_list, QWebPage::InsertOrderedList);
    }
    if (action_unordered_list) {
        //FOLLOW_CHECK(action_unordered_list, QWebPage::InsertUnorderedList);
    }
    if (action_direction_ltr) {
        //FOLLOW_CHECK(action_direction_ltr, QWebPage::SetTextDirectionLeftToRight);
    }
    if (action_direction_rtl) {
        //FOLLOW_CHECK(action_direction_rtl, QWebPage::SetTextDirectionRightToLeft);
    }

    const QString alignment = evaluateJavascript(QStringLiteral("getAlignment()")).toString();
    if (alignment == QStringLiteral("left")) {
        if (action_align_left) {
            action_align_left->setChecked(true);
        }
    } else if (alignment == QStringLiteral("right")) {
        if (action_align_right) {
            action_align_right->setChecked(true);
        }
    } else if (alignment == QStringLiteral("center")) {
        if (action_align_center) {
            action_align_center->setChecked(true);
        }
    } else if (alignment == QStringLiteral("-webkit-auto")) {
        if (action_align_justify) {
            action_align_justify->setChecked(true);
        }
    }

    if (action_font_family) {
        const QString font = evaluateJavascript(QStringLiteral("getFontFamily()")).toString();
        if (!font.isEmpty()) {
            action_font_family->setFont(font);
        }
    }
}

void ComposerEditorWebEnginePrivate::execCommand(const QString &cmd)
{
    const QString js = QStringLiteral("document.execCommand(\"%1\", false, null)").arg(cmd);
    q->page()->runJavaScript(js);
}

void ComposerEditorWebEnginePrivate::execCommand(const QString &cmd, const QString &arg)
{
    const QString js = QStringLiteral("document.execCommand(\"%1\", false, \"%2\")").arg(cmd, arg);
    q->page()->runJavaScript(js);
}

bool ComposerEditorWebEnginePrivate::queryCommandState(const QString &cmd)
{
#if 0
    QWebFrame *frame = q->page()->mainFrame();
    QString js = QStringLiteral("document.queryCommandState(\"%1\", false, null)").arg(cmd);
    const QVariant result = frame->evaluateJavaScript(js);
    return result.toString().simplified().toLower() == QStringLiteral("true");
#else
    return false;
#endif
}

void ComposerEditorWebEnginePrivate::_k_slotSpeakText()
{
    QString text = q->selectedText();
    if (text.isEmpty()) {
        q->page()->toHtml([](const QString & html) {
            KPIMTextEdit::TextToSpeech::self()->say(html);
        });
    }
}

void ComposerEditorWebEnginePrivate::_k_slotPasteWithoutFormatting()
{
#ifndef QT_NO_CLIPBOARD
    if (q->hasFocus()) {
        QString s = QApplication::clipboard()->text();
        if (!s.isEmpty()) {
            s.replace(QStringLiteral("\n"), QStringLiteral("<BR>"));
            execCommand(QStringLiteral("insertHTML"), s);
        }
    }
#endif
}

void ComposerEditorWebEnginePrivate::_k_slotInsertSpecialChar()
{
    KPIMTextEdit::SelectSpecialCharDialog dlg(q);
    dlg.showSelectButton(false);
    dlg.autoInsertChar();
    if (dlg.exec()) {
        execCommand(QStringLiteral("insertHTML"), dlg.currentChar());
    }
}

void ComposerEditorWebEnginePrivate::_k_slotInsertAnchor()
{
    QPointer<ComposerEditorWebEngine::ComposerAnchorDialog> dlg = new ComposerEditorWebEngine::ComposerAnchorDialog(q);
    if (dlg->exec() == QDialog::Accepted) {
        execCommand(QStringLiteral("insertHTML"), dlg->html());
    }
    delete dlg;
}

QMap<QString, QString> ComposerEditorWebEnginePrivate::localImages() const
{
    QMap<QString, QString> lst;
#if 0
    QWebElementCollection images = q->page()->mainFrame()->findAllElements(QStringLiteral("img"));
    Q_FOREACH (const QWebElement &elm, images) {
        if (elm.attribute(QStringLiteral("src")).startsWith(QStringLiteral("file://"))) {
            QUrl url(elm.attribute(QStringLiteral("src")));
            lst.insert(url.fileName(), url.path());
        }
    }
#endif
    return lst;
}

}

