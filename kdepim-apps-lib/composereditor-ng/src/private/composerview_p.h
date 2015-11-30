/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include <QWebHitTestResult>

class KToggleAction;
class QAction;
class KSelectAction;
class KFontAction;
class ComposerImageResizeWidget;
class QWebHitTestResult;

namespace KPIMTextEdit
{
class EmoticonTextEditAction;
}

namespace ComposerEditorNG
{
class ComposerImageResizeWidget;

class Q_DECL_HIDDEN ComposerViewPrivate
{
public:
    ComposerViewPrivate(ComposerView *qq)
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
          imageResizeWidget(Q_NULLPTR)
    {
    }

    QWebHitTestResult contextMenuResult;
    enum FormatType {
        Paragraph,
        Header1,
        Header2,
        Header3,
        Header4,
        Header5,
        Header6,
        Pre,
        Address
    };

    void _k_slotAdjustActions();
    void _k_setFormatType(QAction *action);
    void _k_slotAddEmoticon(const QString &);
    void _k_slotInsertHtml();
    void _k_slotInsertTable();
    void _k_slotAddImage();
    void _k_setTextForegroundColor();
    void _k_setTextBackgroundColor();
    void _k_slotInsertHorizontalRule();
    void _k_insertLink();
    void _k_slotEditLink();
    void _k_setFontSize(int);
    void _k_setFontFamily(const QString &);
    void _k_adjustActions();
    void _k_slotSpellCheck();
    void _k_spellCheckerCorrected(const QString &original, int pos, const QString &replacement);
    void _k_spellCheckerMisspelling(const QString &, int);
    void _k_slotSpellCheckDone(const QString &);
    void _k_slotFind();
    void _k_slotReplace();
    void _k_slotSpeakText();
    void _k_slotDeleteText();
    void _k_slotChangePageColorAndBackground();
    void _k_slotToggleBlockQuote();
    void _k_slotEditImage();
    void _k_slotSaveAs();
    void _k_slotPrint();
    void _k_slotPrintPreview();
    void _k_changeAutoSpellChecking(bool);
    void _k_slotEditList();
    void _k_slotPasteWithoutFormatting();
    void _k_slotInsertSpecialChar();
    void _k_slotInsertAnchor();
    void _k_slotOpenLink();

    QAction *getAction(QWebPage::WebAction action) const;
    QVariant evaluateJavascript(const QString &command);
    void execCommand(const QString &cmd);
    void execCommand(const QString &cmd, const QString &arg);
    bool queryCommandState(const QString &cmd);

    void hideImageResizeWidget();
    void showImageResizeWidget();
    bool checkSpellingEnabled();

    void connectActionGroup();
    void createAction(ComposerView::ComposerViewAction type);

    QMap<QString, QString> localImages() const;

    int spellTextSelectionStart;
    int spellTextSelectionEnd;

    QList<QAction *> htmlEditorActionList;
    KToggleAction *action_text_bold;
    KToggleAction *action_text_italic;
    KToggleAction *action_text_underline;
    KToggleAction *action_text_strikeout;
    KToggleAction *action_align_left;
    KToggleAction *action_align_center;
    KToggleAction *action_align_right;
    KToggleAction *action_align_justify;
    KToggleAction *action_direction_ltr;
    KToggleAction *action_direction_rtl;
    KToggleAction *action_text_subscript;
    KToggleAction *action_text_superscript;
    QAction *action_insert_horizontal_rule;
    QAction *action_list_indent;
    QAction *action_list_dedent;
    KToggleAction *action_ordered_list;
    KToggleAction *action_unordered_list;
    KSelectAction *action_format_type;
    KSelectAction *action_font_size;
    KFontAction *action_font_family;
    KPIMTextEdit::EmoticonTextEditAction *action_add_emoticon;
    QAction *action_insert_html;
    QAction *action_insert_image;
    QAction *action_insert_table;
    QAction *action_text_foreground_color;
    QAction *action_text_background_color;
    QAction *action_format_reset;
    QAction *action_insert_link;
    QAction *action_spell_check;
    QAction *action_find;
    QAction *action_replace;
    QAction *action_page_color;
    QAction *action_block_quote;
    QAction *action_save_as;
    QAction *action_print;
    QAction *action_print_preview;
    QAction *action_paste_withoutformatting;
    QAction *action_insert_specialchar;
    QAction *action_insert_anchor;

    ComposerView *q;
    ComposerImageResizeWidget *imageResizeWidget;
};
}

Q_DECLARE_METATYPE(ComposerEditorNG::ComposerViewPrivate::FormatType)
