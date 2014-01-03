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

#include "composerview.h"

#include <QWebHitTestResult>

class KToggleAction;
class KAction;
class KSelectAction;
class KFontAction;
class ComposerImageResizeWidget;
class QWebHitTestResult;

namespace KPIMTextEdit {
class EmoticonTextEditAction;
}


namespace ComposerEditorNG {
class ComposerImageResizeWidget;

class ComposerViewPrivate
{
public:
    ComposerViewPrivate( ComposerView *qq)
        : action_text_bold(0),
          action_text_italic(0),
          action_text_underline(0),
          action_text_strikeout(0),
          action_align_left(0),
          action_align_center(0),
          action_align_right(0),
          action_align_justify(0),
          action_direction_ltr(0),
          action_direction_rtl(0),
          action_text_subscript(0),
          action_text_superscript(0),
          action_insert_horizontal_rule(0),
          action_list_indent(0),
          action_list_dedent(0),
          action_ordered_list(0),
          action_unordered_list(0),
          action_format_type(0),
          action_font_size(0),
          action_font_family(0),
          action_add_emoticon(0),
          action_insert_html(0),
          action_insert_image(0),
          action_insert_table(0),
          action_text_foreground_color(0),
          action_text_background_color(0),
          action_format_reset(0),
          action_insert_link(0),
          action_spell_check(0),
          action_find(0),
          action_replace(0),
          action_page_color(0),
          action_block_quote(0),
          action_save_as(0),
          action_print(0),
          action_print_preview(0),
          action_paste_withoutformatting(0),
          action_insert_specialchar(0),
          action_insert_anchor(0),
          q(qq),
          imageResizeWidget(0)
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
    void _k_setFormatType(QAction* action);
    void _k_slotAddEmoticon(const QString&);
    void _k_slotInsertHtml();
    void _k_slotInsertTable();
    void _k_slotAddImage();
    void _k_setTextForegroundColor();
    void _k_setTextBackgroundColor();
    void _k_slotInsertHorizontalRule();
    void _k_insertLink();
    void _k_slotEditLink();
    void _k_setFontSize(int);
    void _k_setFontFamily(const QString&);
    void _k_adjustActions();
    void _k_slotSpellCheck();
    void _k_spellCheckerCorrected(const QString& original, int pos, const QString& replacement);
    void _k_spellCheckerMisspelling(const QString& , int);
    void _k_slotSpellCheckDone(const QString&);
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

    QAction* getAction ( QWebPage::WebAction action ) const;
    QVariant evaluateJavascript(const QString& command);
    void execCommand(const QString &cmd);
    void execCommand(const QString &cmd, const QString &arg);
    bool queryCommandState(const QString &cmd);

    void hideImageResizeWidget();
    void showImageResizeWidget();
#if QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 3, 0)
    bool checkSpellingEnabled();
#endif

    void connectActionGroup();
    void createAction(ComposerView::ComposerViewAction type);

    QMap<QString, QString> localImages() const;

    int spellTextSelectionStart;
    int spellTextSelectionEnd;

    QList<KAction*> htmlEditorActionList;
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
    KAction *action_insert_horizontal_rule;
    KAction *action_list_indent;
    KAction *action_list_dedent;
    KToggleAction *action_ordered_list;
    KToggleAction *action_unordered_list;
    KSelectAction *action_format_type;
    KSelectAction *action_font_size;
    KFontAction *action_font_family;
    KPIMTextEdit::EmoticonTextEditAction *action_add_emoticon;
    KAction *action_insert_html;
    KAction *action_insert_image;
    KAction *action_insert_table;
    KAction *action_text_foreground_color;
    KAction *action_text_background_color;
    KAction *action_format_reset;
    KAction *action_insert_link;
    KAction *action_spell_check;
    KAction *action_find;
    KAction *action_replace;
    KAction *action_page_color;
    KAction *action_block_quote;
    KAction *action_save_as;
    KAction *action_print;
    KAction *action_print_preview;
    KAction *action_paste_withoutformatting;
    KAction *action_insert_specialchar;
    KAction *action_insert_anchor;

    ComposerView *q;
    ComposerImageResizeWidget *imageResizeWidget;
};
}

Q_DECLARE_METATYPE(ComposerEditorNG::ComposerViewPrivate::FormatType)
