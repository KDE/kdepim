/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "composerview_p.h"
#include "utils/composereditorutils_p.h"
#include "link/composerlinkdialog.h"
#include "link/composeranchordialog.h"
#include "list/composerlistdialog.h"
#include "image/composerimagedialog.h"
#include "table/composertabledialog.h"
#include "image/composerimageresizewidget.h"
#include "pagecolor/pagecolorbackgrounddialog.h"
#include "helper/listhelper_p.h"
#include "globalsettings_base.h"

#include <kpimtextedit/emoticontexteditaction.h>
#include <kpimtextedit/inserthtmldialog.h>
#include <kpimtextedit/selectspecialchardialog.h>
#include "pimcommon/texttospeech/texttospeech.h"

#include <Sonnet/Dialog>
#include <sonnet/backgroundchecker.h>

#include <KToolInvocation>
#include <KLocalizedString>
#include <KToggleAction>
#include <QAction>
#include <KSelectAction>
#include <QColorDialog>
#include <KMessageBox>
#include <QDebug>
#include <KFontAction>
#include <KPrintPreview>
#include <kdeprintdialog.h>
#include <KRun>
#include <QUrl>
#include <QIcon>
#include <KStandardShortcut>

#include <QAction>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QWebFrame>
#include <QWebElement>
#include <QDebug>
#include <QPointer>
#include <QPrinter>
#include <QPrintDialog>
#include <QClipboard>
#include <QFileDialog>

namespace ComposerEditorNG
{
#define FORWARD_ACTION(action1, action2) \
    q->connect(action1, SIGNAL(triggered()), getAction(action2), SLOT(trigger()));\
    q->connect(getAction(action2), SIGNAL(changed()), SLOT(_k_slotAdjustActions()));

#define FOLLOW_CHECK(a1, a2) a1->setChecked(getAction(a2)->isChecked())

void ComposerViewPrivate::createAction(ComposerView::ComposerViewAction type)
{
    switch (type) {
    case ComposerView::Bold: {
        if (!action_text_bold) {
            action_text_bold = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-bold")), i18nc("@action boldify selected text", "&Bold"), q);
            QFont bold;
            bold.setBold(true);
            action_text_bold->setFont(bold);
            action_text_bold->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
            FORWARD_ACTION(action_text_bold, QWebPage::ToggleBold);
            htmlEditorActionList.append(action_text_bold);
        }
        break;
    }
    case ComposerView::Italic: {
        if (!action_text_italic) {
            action_text_italic = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-italic")), i18nc("@action italicize selected text", "&Italic"), q);
            QFont italic;
            italic.setItalic(true);
            action_text_italic->setFont(italic);
            action_text_italic->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
            FORWARD_ACTION(action_text_italic, QWebPage::ToggleItalic);
            htmlEditorActionList.append(action_text_italic);
        }
        break;
    }
    case ComposerView::Underline: {
        if (!action_text_underline) {
            action_text_underline = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-underline")), i18nc("@action underline selected text", "&Underline"), q);
            QFont underline;
            underline.setUnderline(true);
            action_text_underline->setFont(underline);
            action_text_underline->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
            FORWARD_ACTION(action_text_underline, QWebPage::ToggleUnderline);
            htmlEditorActionList.append(action_text_underline);
        }
        break;
    }
    case ComposerView::StrikeOut: {
        if (!action_text_strikeout) {
            action_text_strikeout = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-strikethrough")), i18nc("@action", "&Strike Out"), q);
            action_text_strikeout->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
            FORWARD_ACTION(action_text_strikeout, QWebPage::ToggleStrikethrough);
            htmlEditorActionList.append(action_text_strikeout);
        }
        break;
    }
    case ComposerView::AlignLeft: {
        if (!action_align_left) {
            action_align_left = new KToggleAction(QIcon::fromTheme(QLatin1String("format-justify-left")), i18nc("@action", "Align &Left"), q);
            action_align_left->setIconText(i18nc("@label left justify", "Left"));
            htmlEditorActionList.append((action_align_left));
            FORWARD_ACTION(action_align_left, QWebPage::AlignLeft);
        }
        break;
    }
    case ComposerView::AlignCenter: {
        if (!action_align_center) {
            action_align_center = new KToggleAction(QIcon::fromTheme(QLatin1String("format-justify-center")), i18nc("@action", "Align &Center"), q);
            action_align_center->setIconText(i18nc("@label center justify", "Center"));
            htmlEditorActionList.append((action_align_center));
            FORWARD_ACTION(action_align_center, QWebPage::AlignCenter);
        }
        break;
    }
    case ComposerView::AlignRight: {
        if (!action_align_right) {
            action_align_right = new KToggleAction(QIcon::fromTheme(QLatin1String("format-justify-right")), i18nc("@action", "Align &Right"), q);
            action_align_right->setIconText(i18nc("@label right justify", "Right"));
            htmlEditorActionList.append((action_align_right));
            FORWARD_ACTION(action_align_right, QWebPage::AlignRight);
        }
        break;
    }
    case ComposerView::AlignJustify: {
        if (!action_align_justify) {
            action_align_justify = new KToggleAction(QIcon::fromTheme(QLatin1String("format-justify-fill")), i18nc("@action", "&Justify"), q);
            action_align_justify->setIconText(i18nc("@label justify fill", "Justify"));
            htmlEditorActionList.append((action_align_justify));
            FORWARD_ACTION(action_align_justify, QWebPage::AlignJustified);
        }
        break;
    }
    case ComposerView::DirectionLtr: {
        if (!action_direction_ltr) {
            action_direction_ltr = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-direction-ltr")), i18nc("@action", "Left-to-Right"), q);
            action_direction_ltr->setIconText(i18nc("@label left-to-right", "Left-to-Right"));
            htmlEditorActionList.append(action_direction_ltr);
            FORWARD_ACTION(action_direction_ltr, QWebPage::SetTextDirectionLeftToRight);
        }
        break;
    }
    case ComposerView::DirectionRtl: {
        if (!action_direction_rtl) {
            action_direction_rtl = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-direction-rtl")), i18nc("@action", "Right-to-Left"), q);
            action_direction_rtl->setIconText(i18nc("@label right-to-left", "Right-to-Left"));
            htmlEditorActionList.append(action_direction_rtl);
            FORWARD_ACTION(action_direction_rtl, QWebPage::SetTextDirectionRightToLeft);
        }
        break;
    }
    case ComposerView::SubScript: {
        if (!action_text_subscript) {
            action_text_subscript = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-subscript")), i18nc("@action", "Subscript"), q);
            htmlEditorActionList.append((action_text_subscript));
            FORWARD_ACTION(action_text_subscript, QWebPage::ToggleSubscript);
        }
        break;
    }
    case ComposerView::SuperScript: {
        if (!action_text_superscript) {
            action_text_superscript = new KToggleAction(QIcon::fromTheme(QLatin1String("format-text-superscript")), i18nc("@action", "Superscript"), q);
            htmlEditorActionList.append((action_text_superscript));
            FORWARD_ACTION(action_text_superscript, QWebPage::ToggleSuperscript);
        }
        break;
    }
    case ComposerView::HorizontalRule: {
        if (!action_insert_horizontal_rule) {
            action_insert_horizontal_rule = new QAction(QIcon::fromTheme(QLatin1String("insert-horizontal-rule")), i18nc("@action", "Insert Rule Line"), q);
            htmlEditorActionList.append((action_insert_horizontal_rule));
            q->connect(action_insert_horizontal_rule, SIGNAL(triggered(bool)), SLOT(_k_slotInsertHorizontalRule()));
        }
        break;
    }
    case ComposerView::ListIndent: {
        if (!action_list_indent) {
            action_list_indent = new QAction(QIcon::fromTheme(QLatin1String("format-indent-more")), i18nc("@action", "Increase Indent"), q);
            htmlEditorActionList.append((action_list_indent));
            FORWARD_ACTION(action_list_indent, QWebPage::Indent);
        }
        break;
    }
    case ComposerView::ListDedent: {
        if (!action_list_dedent) {
            action_list_dedent = new QAction(QIcon::fromTheme(QLatin1String("format-indent-less")), i18nc("@action", "Decrease Indent"), q);
            htmlEditorActionList.append(action_list_dedent);
            FORWARD_ACTION(action_list_dedent, QWebPage::Outdent);
        }
        break;
    }
    case ComposerView::OrderedList: {
        if (!action_ordered_list) {
            action_ordered_list = new KToggleAction(QIcon::fromTheme(QLatin1String("format-list-ordered")), i18n("Ordered Style"), q);
            htmlEditorActionList.append(action_ordered_list);
            FORWARD_ACTION(action_ordered_list, QWebPage::InsertOrderedList);
        }
        break;
    }
    case ComposerView::UnorderedList: {
        if (!action_unordered_list) {
            action_unordered_list = new KToggleAction(QIcon::fromTheme(QLatin1String("format-list-unordered")), i18n("Unordered List"), q);
            htmlEditorActionList.append(action_unordered_list);
            FORWARD_ACTION(action_unordered_list, QWebPage::InsertUnorderedList);
        }
        break;
    }
    case ComposerView::FormatType: {
        if (!action_format_type) {
            action_format_type = new KSelectAction(QIcon::fromTheme(QLatin1String("format-list-unordered")), i18nc("@title:menu", "List Style"), q);
            QAction *act = action_format_type->addAction(i18n("Paragraph"));
            act->setData(QVariant::fromValue(ComposerViewPrivate::Paragraph));
            act = action_format_type->addAction(i18n("Heading 1"));
            act->setData(QVariant::fromValue(ComposerViewPrivate::Header1));
            act = action_format_type->addAction(i18n("Heading 2"));
            act->setData(QVariant::fromValue(ComposerViewPrivate::Header2));
            act = action_format_type->addAction(i18n("Heading 3"));
            act->setData(QVariant::fromValue(ComposerViewPrivate::Header3));
            act = action_format_type->addAction(i18n("Heading 4"));
            act->setData(QVariant::fromValue(ComposerViewPrivate::Header4));
            act = action_format_type->addAction(i18n("Heading 5"));
            act->setData(QVariant::fromValue(ComposerViewPrivate::Header5));
            act = action_format_type->addAction(i18n("Heading 6"));
            act->setData(QVariant::fromValue(ComposerViewPrivate::Header6));
            act = action_format_type->addAction(i18n("Pre"));
            act->setData(QVariant::fromValue(ComposerViewPrivate::Pre));
            act = action_format_type->addAction(i18n("Address"));
            act->setData(QVariant::fromValue(ComposerViewPrivate::Address));
            action_format_type->setCurrentItem(0);
            htmlEditorActionList.append(action_format_type);

            q->connect(action_format_type, SIGNAL(triggered(QAction*)),
                       q, SLOT(_k_setFormatType(QAction*)));
        }
        break;
    }
    case ComposerView::FontSize: {
        if (!action_font_size) {
            action_font_size = new KSelectAction(i18nc("@action", "Font &Size"), q);
            htmlEditorActionList.append(action_font_size);
            QStringList sizes;
            sizes << QLatin1String("xx-small");
            sizes << QLatin1String("x-small");
            sizes << QLatin1String("small");
            sizes << QLatin1String("medium");
            sizes << QLatin1String("large");
            sizes << QLatin1String("x-large");
            sizes << QLatin1String("xx-large");
            action_font_size->setItems(sizes);
            action_font_size->setCurrentItem(0);
            q->connect(action_font_size, SIGNAL(triggered(int)), q, SLOT(_k_setFontSize(int)));
        }
        break;
    }
    case ComposerView::FontFamily: {
        if (!action_font_family) {
            action_font_family = new KFontAction(i18nc("@action", "&Font"), q);
            htmlEditorActionList.append((action_font_family));
            q->connect(action_font_family, SIGNAL(triggered(QString)), q, SLOT(_k_setFontFamily(QString)));
        }
        break;
    }
    case ComposerView::Emoticon: {
        if (!action_add_emoticon) {
            action_add_emoticon = new KPIMTextEdit::EmoticonTextEditAction(q);
            q->connect(action_add_emoticon, SIGNAL(emoticonActivated(QString)),
                       q, SLOT(_k_slotAddEmoticon(QString)));
        }
        break;
    }
    case ComposerView::InsertImage: {
        if (!action_insert_image) {
            action_insert_image = new QAction(QIcon::fromTheme(QLatin1String("insert-image")), i18n("Add Image"), q);
            q->connect(action_insert_image, SIGNAL(triggered(bool)), SLOT(_k_slotAddImage()));
        }
        break;
    }
    case ComposerView::InsertHtml: {
        if (!action_insert_html) {
            action_insert_html = new QAction(i18n("Insert HTML"), q);
            q->connect(action_insert_html, SIGNAL(triggered(bool)), SLOT(_k_slotInsertHtml()));
        }
        break;
    }
    case ComposerView::InsertTable: {
        if (!action_insert_table) {
            action_insert_table = new QAction(QIcon::fromTheme(QLatin1String("insert-table")), i18n("Table..."), q);
            htmlEditorActionList.append(action_insert_table);
            q->connect(action_insert_table, SIGNAL(triggered(bool)), SLOT(_k_slotInsertTable()));
        }
        break;
    }
    case ComposerView::InsertLink: {
        if (!action_insert_link) {
            action_insert_link = new QAction(QIcon::fromTheme(QLatin1String("insert-link")), i18nc("@action", "Link"), q);
            htmlEditorActionList.append(action_insert_link);
            q->connect(action_insert_link, SIGNAL(triggered(bool)), q, SLOT(_k_insertLink()));
        }
        break;
    }
    case ComposerView::TextForegroundColor: {
        if (!action_text_foreground_color) {
            action_text_foreground_color = new QAction(QIcon::fromTheme(QLatin1String("format-stroke-color")), i18nc("@action", "Text &Color..."), q);
            action_text_foreground_color->setIconText(i18nc("@label stroke color", "Color"));
            htmlEditorActionList.append((action_text_foreground_color));
            q->connect(action_text_foreground_color, SIGNAL(triggered()), q, SLOT(_k_setTextForegroundColor()));
        }
        break;
    }
    case ComposerView::TextBackgroundColor: {
        if (!action_text_background_color) {
            action_text_background_color = new QAction(QIcon::fromTheme(QLatin1String("format-fill-color")), i18nc("@action", "Text &Highlight..."), q);
            htmlEditorActionList.append((action_text_background_color));
            q->connect(action_text_background_color, SIGNAL(triggered()), q, SLOT(_k_setTextBackgroundColor()));
        }
        break;
    }
    case ComposerView::FormatReset: {
        if (!action_format_reset) {
            action_format_reset = new QAction(QIcon::fromTheme(QLatin1String("draw-eraser")), i18n("Reset Font Settings"), q);
            FORWARD_ACTION(action_format_reset, QWebPage::RemoveFormat);
        }
        break;
    }
    case ComposerView::SpellCheck: {
        if (!action_spell_check) {
            action_spell_check = new QAction(QIcon::fromTheme(QLatin1String("tools-check-spelling")), i18n("Check Spelling..."), q);
            htmlEditorActionList.append(action_spell_check);
            q->connect(action_spell_check, SIGNAL(triggered(bool)), q, SLOT(_k_slotSpellCheck()));
        }
        break;
    }
    case ComposerView::PageColor: {
        if (!action_page_color) {
            action_page_color = new QAction(i18n("Page Color and Background..."), q);
            htmlEditorActionList.append(action_page_color);
            q->connect(action_page_color, SIGNAL(triggered(bool)), SLOT(_k_slotChangePageColorAndBackground()));
        }
        break;
    }
    case ComposerView::BlockQuote: {
        if (!action_block_quote) {
            action_block_quote = new QAction(QIcon::fromTheme(QLatin1String("format-text-blockquote")), i18n("Blockquote"), q);
            htmlEditorActionList.append(action_block_quote);
            q->connect(action_block_quote, SIGNAL(triggered()), q, SLOT(_k_slotToggleBlockQuote()));
        }
        break;
    }
    case ComposerView::Find: {
        if (!action_find) {
            action_find = new QAction(QIcon::fromTheme(QLatin1String("edit-find")), i18n("&Find..."), q);
            action_find->setShortcut(KStandardShortcut::find().first());
            htmlEditorActionList.append(action_find);
            q->connect(action_find, SIGNAL(triggered()), q, SLOT(_k_slotFind()));
        }
        break;
    }
    case ComposerView::Replace: {
        if (!action_replace) {
            action_replace = new QAction(QIcon::fromTheme(QLatin1String("edit-replace")), i18n("&Replace..."), q);
            htmlEditorActionList.append(action_replace);
            action_replace->setShortcut(KStandardShortcut::replace().first());
            q->connect(action_replace, SIGNAL(triggered()), q, SLOT(_k_slotReplace()));
        }
        break;
    }
    case ComposerView::SaveAs: {
        if (!action_save_as) {
            action_save_as = new QAction(QIcon::fromTheme(QLatin1String("file_save_as")), i18n("Save &As..."), q);
            htmlEditorActionList.append(action_save_as);
            action_replace->setShortcut(KStandardShortcut::save().first());
            q->connect(action_save_as, SIGNAL(triggered()), q, SLOT(_k_slotSaveAs()));
        }
        break;
    }
    case ComposerView::Print: {
        if (!action_print) {
            action_print = new QAction(QIcon::fromTheme(QLatin1String("file_print")), i18n("&Print..."), q);
            htmlEditorActionList.append(action_print);
            action_replace->setShortcut(KStandardShortcut::print().first());
            q->connect(action_print, SIGNAL(triggered()), q, SLOT(_k_slotPrint()));
        }
        break;
    }
    case ComposerView::PrintPreview: {
        if (!action_print_preview) {
            action_print_preview = new QAction(QIcon::fromTheme(QLatin1String("file_print_preview")), i18n("Print Previe&w"), q);
            htmlEditorActionList.append(action_print_preview);
            q->connect(action_print_preview, SIGNAL(triggered()), q, SLOT(_k_slotPrintPreview()));
        }
        break;
    }
    case ComposerView::PasteWithoutFormatting: {
        if (!action_paste_withoutformatting) {
            action_paste_withoutformatting = new QAction(i18n("Paste Without Formatting"), q);
            htmlEditorActionList.append(action_paste_withoutformatting);
            q->connect(action_paste_withoutformatting, SIGNAL(triggered()), q, SLOT(_k_slotPasteWithoutFormatting()));
        }
        break;
    }
    case ComposerView::InsertSpecialChar: {
        if (!action_insert_specialchar) {
            action_insert_specialchar = new QAction(i18n("Insert Special Char..."), q);
            htmlEditorActionList.append(action_insert_specialchar);
            q->connect(action_insert_specialchar, SIGNAL(triggered()), q, SLOT(_k_slotInsertSpecialChar()));
        }
        break;
    }
    case ComposerView::InsertAnchor: {
        if (!action_insert_anchor) {
            action_insert_anchor = new QAction(i18n("Insert Anchor..."), q);
            htmlEditorActionList.append(action_insert_anchor);
            q->connect(action_insert_anchor, SIGNAL(triggered()), q, SLOT(_k_slotInsertAnchor()));
        }
        break;
    }
    case ComposerView::Separator:
        //nothing
        break;
    case ComposerView::LastType:
        //nothing
        break;
    }

}

void ComposerViewPrivate::connectActionGroup()
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

#if QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 3, 0)
bool ComposerViewPrivate::checkSpellingEnabled()
{
    return ComposerEditorNG::GlobalSettingsBase::autoSpellChecking();
}
#endif

void ComposerViewPrivate::_k_changeAutoSpellChecking(bool checked)
{
#if QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 3, 0)
    ComposerEditorNG::GlobalSettingsBase::setAutoSpellChecking(checked);
#else
    Q_UNUSED(checked);
#endif
}

QAction *ComposerViewPrivate::getAction(QWebPage::WebAction action) const
{
    if (action >= 0 && action <= 66) {
        return q->page()->action(static_cast<QWebPage::WebAction>(action));
    } else {
        return 0;
    }
}

void ComposerViewPrivate::hideImageResizeWidget()
{
    delete imageResizeWidget;
    imageResizeWidget = 0;
}

void ComposerViewPrivate::showImageResizeWidget()
{
    if (!imageResizeWidget) {
        imageResizeWidget = new ComposerImageResizeWidget(contextMenuResult.element(), q);
        imageResizeWidget->move(contextMenuResult.element().geometry().topLeft());
        imageResizeWidget->show();
    }
}

static QVariant execJScript(QWebElement element, const QString &script)
{
    if (element.isNull()) {
        return QVariant();
    }
    return element.evaluateJavaScript(script);
}

void ComposerViewPrivate::_k_setFormatType(QAction *act)
{
    if (!act) {
        return;
    }
    QString command;
    switch (act->data().value<ComposerEditorNG::ComposerViewPrivate::FormatType>()) {
    case Paragraph:
        command = QLatin1String("p");
        break;
    case Header1:
        command = QLatin1String("h1");
        break;
    case Header2:
        command = QLatin1String("h2");
        break;
    case Header3:
        command = QLatin1String("h3");
        break;
    case Header4:
        command = QLatin1String("h4");
        break;
    case Header5:
        command = QLatin1String("h5");
        break;
    case Header6:
        command = QLatin1String("h6");
        break;
    case Pre:
        command = QLatin1String("pre");
        break;
    case Address:
        command = QLatin1String("address");
        break;
    }
    execCommand(QLatin1String("formatBlock"), command);
}

void ComposerViewPrivate::_k_slotToggleBlockQuote()
{
    execCommand(QLatin1String("formatBlock"), QLatin1String("BLOCKQUOTE"));
}

void ComposerViewPrivate::_k_slotAddEmoticon(const QString &emoticon)
{
    execCommand(QLatin1String("insertHTML"), emoticon);
}

void ComposerViewPrivate::_k_slotInsertHtml()
{
    QPointer<KPIMTextEdit::InsertHtmlDialog> dialog = new KPIMTextEdit::InsertHtmlDialog(q);
    if (dialog->exec()) {
        const QString str = dialog->html().remove(QLatin1String("\n"));
        if (!str.isEmpty()) {
            execCommand(QLatin1String("insertHTML"), str);
        }
    }
    delete dialog;
}

void ComposerViewPrivate::_k_setTextBackgroundColor()
{
    QColor newColor = ComposerEditorNG::Utils::convertRgbToQColor(evaluateJavascript(QLatin1String("getTextBackgroundColor()")).toString());
    newColor = QColorDialog::getColor(newColor, q);
    if (newColor.isValid()) {
        execCommand(QLatin1String("hiliteColor"), newColor.name());
    }
}

QVariant ComposerViewPrivate::evaluateJavascript(const QString &command)
{
    return q->page()->mainFrame()->evaluateJavaScript(command);
}

void ComposerViewPrivate::_k_slotDeleteText()
{
    evaluateJavascript(QLatin1String("setDeleteSelectedText()"));
}

void ComposerViewPrivate::_k_setTextForegroundColor()
{
    QColor newColor = ComposerEditorNG::Utils::convertRgbToQColor(evaluateJavascript(QLatin1String("getTextForegroundColor()")).toString());
    newColor = QColorDialog::getColor(newColor, q);
    if (newColor.isValid()) {
        execCommand(QLatin1String("foreColor"), newColor.name());
    }
}

void ComposerViewPrivate::_k_slotAddImage()
{
    QPointer<ComposerImageDialog> dlg = new ComposerImageDialog(q);
    if (dlg->exec() == QDialog::Accepted) {
        execCommand(QLatin1String("insertHTML"), dlg->html());
    }
    delete dlg;
}

void ComposerViewPrivate::_k_slotEditImage()
{
    showImageResizeWidget();
    ComposerImageDialog dlg(contextMenuResult.element(), q);
    dlg.exec();
}

void ComposerViewPrivate::_k_slotInsertTable()
{
    QPointer<ComposerTableDialog> dlg = new ComposerTableDialog(q);
    if (dlg->exec() == QDialog::Accepted) {
        execCommand(QLatin1String("insertHTML"), dlg->html());
    }
    delete dlg;
}

void ComposerViewPrivate::_k_slotInsertHorizontalRule()
{
    execCommand(QLatin1String("insertHTML"), QLatin1String("<hr>"));
}

void ComposerViewPrivate::_k_insertLink()
{
    const QString selectedText = q->selectedText();
    QPointer<ComposerEditorNG::ComposerLinkDialog> dlg = new ComposerEditorNG::ComposerLinkDialog(selectedText, q);
    if (dlg->exec() == QDialog::Accepted) {
        const QString html(dlg->html());
        if (!html.isEmpty()) {
            execCommand(QLatin1String("insertHTML"), html);
        }
    }
    delete dlg;
}

void ComposerViewPrivate::_k_slotEditLink()
{
    ComposerEditorNG::ComposerLinkDialog dlg(contextMenuResult.linkElement(), q);
    dlg.exec();
}

void ComposerViewPrivate::_k_slotOpenLink()
{
    const QString href = contextMenuResult.linkElement().attribute(QLatin1String("href"));
    if (!href.isEmpty()) {
        new KRun(QUrl(href), 0);
    }
}

void ComposerViewPrivate::_k_setFontSize(int fontSize)
{
    execCommand(QLatin1String("fontSize"), QString::number(fontSize + 1)); //Verify
}

void ComposerViewPrivate::_k_setFontFamily(const QString &family)
{
    execCommand(QLatin1String("fontName"), family);
}

void ComposerViewPrivate::_k_slotSpellCheck()
{
    QString text(execJScript(contextMenuResult.element(), QLatin1String("this.value")).toString());
    if (contextMenuResult.isContentSelected()) {
        spellTextSelectionStart = qMax(0, execJScript(contextMenuResult.element(), QLatin1String("this.selectionStart")).toInt());
        spellTextSelectionEnd = qMax(0, execJScript(contextMenuResult.element(), QLatin1String("this.selectionEnd")).toInt());
        text = text.mid(spellTextSelectionStart, (spellTextSelectionEnd - spellTextSelectionStart));
    } else {
        spellTextSelectionStart = 0;
        spellTextSelectionEnd = 0;
    }

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
}

void ComposerViewPrivate::_k_spellCheckerCorrected(const QString &original, int pos, const QString &replacement)
{
    // Adjust the selection end...
    if (spellTextSelectionEnd > 0) {
        spellTextSelectionEnd += qMax(0, (replacement.length() - original.length()));
    }

    const int index = pos + spellTextSelectionStart;
    QString script(QLatin1String("this.value=this.value.substring(0,"));
    script += QString::number(index);
    script += QLatin1String(") + \"");
    QString w(replacement);
    script +=  w.replace(QLatin1Char('\''), QLatin1String("\\\'")); // Escape any Quote marks in replacement word
    script += QLatin1String("\" + this.value.substring(");
    script += QString::number(index + original.length());
    script += QLatin1String(")");

    //qDebug() << "**** script:" << script;
    execJScript(contextMenuResult.element(), script);
}

void ComposerViewPrivate::_k_spellCheckerMisspelling(const QString &text, int pos)
{
    // qDebug() << text << pos;
    QString selectionScript(QLatin1String("this.setSelectionRange("));
    selectionScript += QString::number(pos + spellTextSelectionStart);
    selectionScript += QLatin1Char(',');
    selectionScript += QString::number(pos + text.length() + spellTextSelectionStart);
    selectionScript += QLatin1Char(')');
    execJScript(contextMenuResult.element(), selectionScript);
}

void ComposerViewPrivate::_k_slotSpellCheckDone(const QString &)
{
    // Restore the text selection if one was present before we started the
    // spell check.
    if (spellTextSelectionStart > 0 || spellTextSelectionEnd > 0) {
        QString script(QLatin1String("; this.setSelectionRange("));
        script += QString::number(spellTextSelectionStart);
        script += QLatin1Char(',');
        script += QString::number(spellTextSelectionEnd);
        script += QLatin1Char(')');
        execJScript(contextMenuResult.element(), script);
    }
}

void ComposerViewPrivate::_k_slotFind()
{
    Q_EMIT q->showFindBar();
}

void ComposerViewPrivate::_k_slotReplace()
{
    //TODO
}

void ComposerViewPrivate::_k_slotSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(q, i18n("Save as..."), QString(), i18n("HTML-Files (*.htm *.html);;All Files (*)"));
    if (fn.isEmpty()) {
        KMessageBox::error(q, i18n("Not file selected."), i18n("Save as"));
        return;
    }
    if (!(fn.endsWith(QLatin1String(".htm"), Qt::CaseInsensitive) ||
            fn.endsWith(QLatin1String(".html"), Qt::CaseInsensitive))) {
        fn += QLatin1String(".htm");
    }
    QFile file(fn);
    bool success = file.open(QIODevice::WriteOnly);
    if (success) {
        // FIXME: here we always use UTF-8 encoding
        const QString content = q->page()->mainFrame()->toHtml();
        QByteArray data = content.toUtf8();
        const qint64 c = file.write(data);
        success = (c >= data.length());
    }
}

void ComposerViewPrivate::_k_slotPrint()
{
    QPrinter printer;
    QPointer<QPrintDialog> dlg(KdePrint::createPrintDialog(&printer));

    if (dlg->exec() == QDialog::Accepted) {
        q->print(&printer);
    }
    delete dlg;
}

void ComposerViewPrivate::_k_slotPrintPreview()
{
    QPrinter printer;
    KPrintPreview previewdlg(&printer, q);
    q->print(&printer);
    previewdlg.exec();
}

void ComposerViewPrivate::_k_slotChangePageColorAndBackground()
{
    const QWebElement element = q->page()->mainFrame()->findFirstElement(QLatin1String("body"));
    if (!element.isNull()) {
        QPointer<PageColorBackgroundDialog> dlg = new PageColorBackgroundDialog(element, q);
        dlg->exec();
        delete dlg;
    }
}

void ComposerViewPrivate::_k_slotEditList()
{
    QWebElement listElement = ListHelper::listElement(contextMenuResult.element());
    if (!listElement.isNull()) {
        QPointer<ComposerListDialog> dlg = new ComposerListDialog(listElement, q);
        if (dlg->exec()) {
            //TODO
        }
        delete dlg;
    }
}

void ComposerViewPrivate::_k_slotAdjustActions()
{
    if (action_text_bold) {
        FOLLOW_CHECK(action_text_bold, QWebPage::ToggleBold);
    }
    if (action_text_italic) {
        FOLLOW_CHECK(action_text_italic, QWebPage::ToggleItalic);
    }
    if (action_text_strikeout) {
        FOLLOW_CHECK(action_text_strikeout, QWebPage::ToggleStrikethrough);
    }
    if (action_text_underline) {
        FOLLOW_CHECK(action_text_underline, QWebPage::ToggleUnderline);
    }
    if (action_text_subscript) {
        FOLLOW_CHECK(action_text_subscript, QWebPage::ToggleSubscript);
    }
    if (action_text_superscript) {
        FOLLOW_CHECK(action_text_superscript, QWebPage::ToggleSuperscript);
    }
    if (action_ordered_list) {
        FOLLOW_CHECK(action_ordered_list, QWebPage::InsertOrderedList);
    }
    if (action_unordered_list) {
        FOLLOW_CHECK(action_unordered_list, QWebPage::InsertUnorderedList);
    }
    if (action_direction_ltr) {
        FOLLOW_CHECK(action_direction_ltr, QWebPage::SetTextDirectionLeftToRight);
    }
    if (action_direction_rtl) {
        FOLLOW_CHECK(action_direction_rtl, QWebPage::SetTextDirectionRightToLeft);
    }

    const QString alignment = evaluateJavascript(QLatin1String("getAlignment()")).toString();
    if (alignment == QLatin1String("left")) {
        if (action_align_left) {
            action_align_left->setChecked(true);
        }
    } else if (alignment == QLatin1String("right")) {
        if (action_align_right) {
            action_align_right->setChecked(true);
        }
    } else if (alignment == QLatin1String("center")) {
        if (action_align_center) {
            action_align_center->setChecked(true);
        }
    } else if (alignment == QLatin1String("-webkit-auto")) {
        if (action_align_justify) {
            action_align_justify->setChecked(true);
        }
    }

    if (action_font_family) {
        const QString font = evaluateJavascript(QLatin1String("getFontFamily()")).toString();
        if (!font.isEmpty()) {
            action_font_family->setFont(font);
        }
    }
}

void ComposerViewPrivate::execCommand(const QString &cmd)
{
    QWebFrame *frame = q->page()->mainFrame();
    const QString js = QString::fromLatin1("document.execCommand(\"%1\", false, null)").arg(cmd);
    frame->evaluateJavaScript(js);
}

void ComposerViewPrivate::execCommand(const QString &cmd, const QString &arg)
{
    QWebFrame *frame = q->page()->mainFrame();
    const QString js = QString::fromLatin1("document.execCommand(\"%1\", false, \"%2\")").arg(cmd).arg(arg);
    frame->evaluateJavaScript(js);
}

bool ComposerViewPrivate::queryCommandState(const QString &cmd)
{
    QWebFrame *frame = q->page()->mainFrame();
    QString js = QString::fromLatin1("document.queryCommandState(\"%1\", false, null)").arg(cmd);
    const QVariant result = frame->evaluateJavaScript(js);
    return result.toString().simplified().toLower() == QLatin1String("true");
}

void ComposerViewPrivate::_k_slotSpeakText()
{
    QString text = q->selectedText();
    if (text.isEmpty()) {
        text = q->page()->mainFrame()->toPlainText();
    }
    PimCommon::TextToSpeech::self()->say(text);
}

void ComposerViewPrivate::_k_slotPasteWithoutFormatting()
{
#ifndef QT_NO_CLIPBOARD
    if (q->hasFocus()) {
        QString s = QApplication::clipboard()->text();
        if (!s.isEmpty()) {
            s.replace(QLatin1String("\n"), QLatin1String("<BR>"));
            execCommand(QLatin1String("insertHTML"), s);
        }
    }
#endif
}

void ComposerViewPrivate::_k_slotInsertSpecialChar()
{
    KPIMTextEdit::SelectSpecialCharDialog dlg(q);
    dlg.showSelectButton(false);
    dlg.autoInsertChar();
    if (dlg.exec()) {
        execCommand(QLatin1String("insertHTML"), dlg.currentChar());
    }
}

void ComposerViewPrivate::_k_slotInsertAnchor()
{
    QPointer<ComposerAnchorDialog> dlg = new ComposerAnchorDialog(q);
    if (dlg->exec() == QDialog::Accepted) {
        execCommand(QLatin1String("insertHTML"), dlg->html());
    }
    delete dlg;
}

QMap<QString, QString> ComposerViewPrivate::localImages() const
{
    QMap<QString, QString> lst;
    QWebElementCollection images = q->page()->mainFrame()->findAllElements(QLatin1String("img"));
    Q_FOREACH (const QWebElement &elm, images) {
        if (elm.attribute(QLatin1String("src")).startsWith(QLatin1String("file://"))) {
            QUrl url(elm.attribute(QLatin1String("src")));
            lst.insert(url.fileName(), url.path());
        }
    }
    return lst;
}

}

