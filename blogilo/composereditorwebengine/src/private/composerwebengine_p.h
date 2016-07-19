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

#include "composerwebengine.h"

#include <kpimtextedit/emoticontexteditaction.h>

class KToggleAction;
class QAction;
class KSelectAction;
class KFontAction;
class ComposerImageResizeWidget;

namespace KPIMTextEdit
{
class EmoticonTextEditAction;
}
namespace WebEngineViewer
{
class WebEnginePage;
}

namespace ComposerEditorWebEngine
{
class ComposerImageResizeWidget;

class Q_DECL_HIDDEN ComposerEditorWebEnginePrivate
{
public:
    ComposerEditorWebEnginePrivate(ComposerWebEngine *qq);

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
    void _k_slotBold(bool);
    void _k_slotItalic(bool);
    void _k_slotUnderline(bool);
    void _k_slotStrikeout(bool);
    void _k_slotSuperscript(bool b);
    void _k_slotJustifyLeft(bool b);
    void _k_slotJustifyCenter(bool b);
    void _k_slotJustifyRight(bool b);
    void _k_slotJustifyFull(bool b);
    void _k_slotSubscript(bool b);
    void _k_slotListIndent();
    void _k_slotListDedent();
    void _k_slotOrderedList(bool b);
    void _k_slotUnOrderedList(bool b);
    void _k_slotResetFormat();
    void _k_slotDirectionLtr();
    void _k_slotDirectionRtl();
    QAction *getAction(QWebEnginePage::WebAction action) const;
    QVariant evaluateJavascript(const QString &command);
    void execCommand(const QString &cmd);
    void execCommand(const QString &cmd, const QString &arg);
    bool queryCommandState(const QString &cmd);

    void hideImageResizeWidget();
    void showImageResizeWidget();
    bool checkSpellingEnabled();

    void connectActionGroup();
    void createAction(ComposerWebEngine::ComposerWebEngineAction type);

    QMap<QString, QString> localImages() const;

    int spellTextSelectionStart;
    int spellTextSelectionEnd;
    void saveHtml(QWebEnginePage *page, const QString &fileName);

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

    ComposerWebEngine *q;
    //ComposerImageResizeWidget *imageResizeWidget;
    WebEngineViewer::WebEnginePage *mPageEngine;
};
}

Q_DECLARE_METATYPE(ComposerEditorWebEngine::ComposerEditorWebEnginePrivate::FormatType)
