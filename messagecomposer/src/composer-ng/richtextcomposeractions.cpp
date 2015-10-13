/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "richtextcomposeractions.h"
#include "richtextcomposercontroler.h"
#include "richtextcomposer.h"
#include <kpimtextedit/emoticontexteditaction.h>
#include <kpimtextedit/tableactionmenu.h>
#include <KToggleAction>
#include <KLocalizedString>
#include <KActionCollection>
#include <KFontAction>
#include <KFontSizeAction>
#include <QTextCharFormat>
#include <QTextList>

using namespace MessageComposer;

class Q_DECL_HIDDEN RichTextComposerActions::RichTextComposerActionsPrivate
{
public:
    RichTextComposerActionsPrivate(MessageComposer::RichTextComposerControler *controler)
        : composerControler(controler),
          action_align_left(Q_NULLPTR),
          action_align_right(Q_NULLPTR),
          action_align_center(Q_NULLPTR),
          action_align_justify(Q_NULLPTR),
          action_direction_ltr(Q_NULLPTR),
          action_direction_rtl(Q_NULLPTR),
          action_text_superscript(Q_NULLPTR),
          action_text_subscript(Q_NULLPTR),
          action_text_bold(Q_NULLPTR),
          action_text_italic(Q_NULLPTR),
          action_text_underline(Q_NULLPTR),
          action_text_strikeout(Q_NULLPTR),
          action_font_family(Q_NULLPTR),
          action_font_size(Q_NULLPTR),
          action_insert_horizontal_rule(Q_NULLPTR),
          action_text_foreground_color(Q_NULLPTR),
          action_text_background_color(Q_NULLPTR),
          action_manage_link(Q_NULLPTR),
          action_list_indent(Q_NULLPTR),
          action_list_dedent(Q_NULLPTR),
          action_list_style(Q_NULLPTR),
          action_paste_quotation(Q_NULLPTR),
          action_add_quote_chars(Q_NULLPTR),
          action_remove_quote_chars(Q_NULLPTR),
          action_paste_without_formatting(Q_NULLPTR),
          action_add_image(Q_NULLPTR),
          action_add_emoticon(Q_NULLPTR),
          action_insert_html(Q_NULLPTR),
          action_add_table(Q_NULLPTR),
          action_delete_line(Q_NULLPTR),
          action_format_reset(Q_NULLPTR),
          action_format_painter(Q_NULLPTR),
          richTextEnabled(false)
    {
    }
    QList<QAction *> richTextActionList;

    MessageComposer::RichTextComposerControler *composerControler;
    KToggleAction *action_align_left;
    KToggleAction *action_align_right;
    KToggleAction *action_align_center;
    KToggleAction *action_align_justify;

    KToggleAction *action_direction_ltr;
    KToggleAction *action_direction_rtl;

    KToggleAction *action_text_superscript;
    KToggleAction *action_text_subscript;

    KToggleAction *action_text_bold;
    KToggleAction *action_text_italic;
    KToggleAction *action_text_underline;
    KToggleAction *action_text_strikeout;

    KFontAction *action_font_family;
    KFontSizeAction *action_font_size;

    QAction *action_insert_horizontal_rule;
    QAction *action_text_foreground_color;
    QAction *action_text_background_color;
    QAction *action_manage_link;

    QAction *action_list_indent;
    QAction *action_list_dedent;

    KSelectAction *action_list_style;

    QAction *action_paste_quotation;
    QAction *action_add_quote_chars;
    QAction *action_remove_quote_chars;
    QAction *action_paste_without_formatting;

    QAction *action_add_image;
    QAction *action_add_emoticon;
    QAction *action_insert_html;
    KPIMTextEdit::TableActionMenu *action_add_table;
    QAction *action_delete_line;
    QAction *action_format_reset;

    KToggleAction *action_format_painter;

    bool richTextEnabled;
};

RichTextComposerActions::RichTextComposerActions(MessageComposer::RichTextComposerControler *controler, QObject *parent)
    : QObject(parent),
      d(new RichTextComposerActions::RichTextComposerActionsPrivate(controler))
{

}

RichTextComposerActions::~RichTextComposerActions()
{
    delete d;
}

QList<QAction *> RichTextComposerActions::richTextActionList() const
{
    return d->richTextActionList;
}

int RichTextComposerActions::numberOfActions() const
{
    return d->richTextActionList.count();
}

void RichTextComposerActions::createActions(KActionCollection *ac)
{
    //Alignment
    d->action_align_left = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-left")),
            i18nc("@action", "Align &Left"), this);
    d->action_align_left->setIconText(i18nc("@label left justify", "Left"));
    d->richTextActionList.append((d->action_align_left));
    d->action_align_left->setObjectName(QStringLiteral("format_align_left"));
    connect(d->action_align_left, &KToggleAction::triggered,
            d->composerControler, &RichTextComposerControler::alignLeft);
    ac->addAction(QStringLiteral("format_align_left"), d->action_align_left);

    d->action_align_center = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-center")),
            i18nc("@action", "Align &Center"), this);
    d->action_align_center->setIconText(i18nc("@label center justify", "Center"));
    d->richTextActionList.append((d->action_align_center));
    d->action_align_center->setObjectName(QStringLiteral("format_align_center"));
    connect(d->action_align_center, &KToggleAction::triggered,
            d->composerControler, &RichTextComposerControler::alignCenter);
    ac->addAction(QStringLiteral("format_align_center"), d->action_align_center);

    d->action_align_right = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-right")),
            i18nc("@action", "Align &Right"), this);
    d->action_align_right->setIconText(i18nc("@label right justify", "Right"));
    d->richTextActionList.append((d->action_align_right));
    d->action_align_right->setObjectName(QStringLiteral("format_align_right"));
    connect(d->action_align_right, &KToggleAction::triggered,
            d->composerControler, &RichTextComposerControler::alignRight);
    ac->addAction(QStringLiteral("format_align_right"), d->action_align_right);

    d->action_align_justify = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-fill")),
            i18nc("@action", "&Justify"), this);
    d->action_align_justify->setIconText(i18nc("@label justify fill", "Justify"));
    d->richTextActionList.append((d->action_align_justify));
    d->action_align_justify->setObjectName(QStringLiteral("format_align_justify"));
    connect(d->action_align_justify, &KToggleAction::triggered,
            d->composerControler, &RichTextComposerControler::alignJustify);
    ac->addAction(QStringLiteral("format_align_justify"), d->action_align_justify);

    QActionGroup *alignmentGroup = new QActionGroup(this);
    alignmentGroup->addAction(d->action_align_left);
    alignmentGroup->addAction(d->action_align_center);
    alignmentGroup->addAction(d->action_align_right);
    alignmentGroup->addAction(d->action_align_justify);

    //Align text
    d->action_direction_ltr = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-direction-ltr")),
            i18nc("@action", "Left-to-Right"), this);
    d->action_direction_ltr->setIconText(i18nc("@label left-to-right", "Left-to-Right"));
    d->richTextActionList.append(d->action_direction_ltr);
    d->action_direction_ltr->setObjectName(QStringLiteral("direction_ltr"));
    connect(d->action_direction_ltr, &KToggleAction::triggered,
            d->composerControler, &RichTextComposerControler::makeLeftToRight);
    ac->addAction(QStringLiteral("direction_ltr"), d->action_direction_ltr);

    d->action_direction_rtl = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-direction-rtl")),
            i18nc("@action", "Right-to-Left"), this);
    d->action_direction_rtl->setIconText(i18nc("@label right-to-left", "Right-to-Left"));
    d->richTextActionList.append(d->action_direction_rtl);
    d->action_direction_rtl->setObjectName(QStringLiteral("direction_rtl"));
    connect(d->action_direction_rtl, &KToggleAction::triggered,
            d->composerControler, &RichTextComposerControler::makeRightToLeft);
    ac->addAction(QStringLiteral("direction_rtl"), d->action_direction_rtl);

    QActionGroup *directionGroup = new QActionGroup(this);
    directionGroup->addAction(d->action_direction_ltr);
    directionGroup->addAction(d->action_direction_rtl);

    // Sub/Super script
    d->action_text_subscript = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-subscript")),
            i18nc("@action", "Subscript"), this);
    d->richTextActionList.append((d->action_text_subscript));
    d->action_text_subscript->setObjectName(QStringLiteral("format_text_subscript"));
    connect(d->action_text_subscript, &KToggleAction::triggered,
            d->composerControler, &RichTextComposerControler::setTextSubScript);
    ac->addAction(QStringLiteral("format_text_subscript"), d->action_text_subscript);

    d->action_text_superscript = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-superscript")),
            i18nc("@action", "Superscript"), this);
    d->richTextActionList.append((d->action_text_superscript));
    d->action_text_superscript->setObjectName(QStringLiteral("format_text_superscript"));
    connect(d->action_text_superscript, &KToggleAction::triggered,
            d->composerControler, &RichTextComposerControler::setTextSuperScript);
    ac->addAction(QStringLiteral("format_text_superscript"), d->action_text_superscript);

    d->action_text_bold = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-bold")),
                                            i18nc("@action boldify selected text", "&Bold"), this);
    QFont bold;
    bold.setBold(true);
    d->action_text_bold->setFont(bold);
    d->richTextActionList.append((d->action_text_bold));
    d->action_text_bold->setObjectName(QStringLiteral("format_text_bold"));
    ac->addAction(QStringLiteral("format_text_bold"), d->action_text_bold);
    ac->setDefaultShortcut(d->action_text_bold, Qt::CTRL + Qt::Key_B);
    connect(d->action_text_bold, &KToggleAction::triggered, d->composerControler, &RichTextComposerControler::setTextBold);

    d->action_text_italic = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-italic")),
            i18nc("@action italicize selected text", "&Italic"), this);
    QFont italic;
    italic.setItalic(true);
    d->action_text_italic->setFont(italic);
    d->richTextActionList.append((d->action_text_italic));
    d->action_text_italic->setObjectName(QStringLiteral("format_text_italic"));
    ac->addAction(QStringLiteral("format_text_italic"), d->action_text_italic);
    ac->setDefaultShortcut(d->action_text_italic, Qt::CTRL + Qt::Key_I);
    connect(d->action_text_italic, &KToggleAction::triggered, d->composerControler, &RichTextComposerControler::setTextItalic);

    d->action_text_underline = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-underline")),
            i18nc("@action underline selected text", "&Underline"), this);
    QFont underline;
    underline.setUnderline(true);
    d->action_text_underline->setFont(underline);
    d->richTextActionList.append((d->action_text_underline));
    d->action_text_underline->setObjectName(QStringLiteral("format_text_underline"));
    ac->addAction(QStringLiteral("format_text_underline"), d->action_text_underline);
    ac->setDefaultShortcut(d->action_text_underline, Qt::CTRL + Qt::Key_U);
    connect(d->action_text_underline, &KToggleAction::triggered, d->composerControler, &RichTextComposerControler::setTextUnderline);

    d->action_text_strikeout = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-strikethrough")),
            i18nc("@action", "&Strike Out"), this);
    QFont strikeout;
    strikeout.setStrikeOut(true);
    d->action_text_strikeout->setFont(strikeout);
    d->richTextActionList.append((d->action_text_strikeout));
    ac->addAction(QStringLiteral("format_text_strikeout"), d->action_text_strikeout);
    d->action_text_strikeout->setObjectName(QStringLiteral("format_text_strikeout"));
    ac->setDefaultShortcut(d->action_text_strikeout, Qt::CTRL + Qt::Key_L);
    connect(d->action_text_strikeout, &KToggleAction::triggered, d->composerControler, &RichTextComposerControler::setTextStrikeOut);

    //Font Family
    d->action_font_family = new KFontAction(i18nc("@action", "&Font"), this);
    d->richTextActionList.append((d->action_font_family));
    d->action_font_family->setObjectName(QStringLiteral("format_font_family"));
    ac->addAction(QStringLiteral("format_font_family"), d->action_font_family);
    connect(d->action_font_family, SIGNAL(triggered(QString)), d->composerControler, SLOT(setFontFamily(QString)));

    //Font Size
    d->action_font_size = new KFontSizeAction(i18nc("@action", "Font &Size"), this);
    d->richTextActionList.append((d->action_font_size));
    d->action_font_size->setObjectName(QStringLiteral("format_font_size"));
    ac->addAction(QStringLiteral("format_font_size"), d->action_font_size);
    connect(d->action_font_size, &KFontSizeAction::fontSizeChanged, d->composerControler, &RichTextComposerControler::setFontSize);

    d->action_insert_horizontal_rule = new QAction(QIcon::fromTheme(QStringLiteral("insert-horizontal-rule")),
            i18nc("@action", "Insert Rule Line"), this);
    d->richTextActionList.append((d->action_insert_horizontal_rule));
    d->action_insert_horizontal_rule->setObjectName(QStringLiteral("insert_horizontal_rule"));
    ac->addAction(QStringLiteral("insert_horizontal_rule"), d->action_insert_horizontal_rule);
    connect(d->action_insert_horizontal_rule, &QAction::triggered,
            d->composerControler, &RichTextComposerControler::insertHorizontalRule);

    //Foreground Color
    d->action_text_foreground_color = new QAction(QIcon::fromTheme(QStringLiteral("format-stroke-color")),
            i18nc("@action", "Text &Color..."), this);
    d->action_text_foreground_color->setIconText(i18nc("@label stroke color", "Color"));
    d->richTextActionList.append((d->action_text_foreground_color));
    d->action_text_foreground_color->setObjectName(QStringLiteral("format_text_foreground_color"));
    ac->addAction(QStringLiteral("format_text_foreground_color"), d->action_text_foreground_color);
    connect(d->action_text_foreground_color, &QAction::triggered, d->composerControler, &RichTextComposerControler::setChangeTextForegroundColor);
    //Background Color
    d->action_text_background_color = new QAction(QIcon::fromTheme(QStringLiteral("format-fill-color")),
            i18nc("@action", "Text &Highlight..."), this);
    d->richTextActionList.append((d->action_text_background_color));
    ac->addAction(QStringLiteral("format_text_background_color"), d->action_text_background_color);
    d->action_text_background_color->setObjectName(QStringLiteral("format_text_background_color"));
    connect(d->action_text_background_color, &QAction::triggered, d->composerControler, &RichTextComposerControler::setChangeTextBackgroundColor);

    d->action_manage_link = new QAction(QIcon::fromTheme(QStringLiteral("insert-link")),
                                        i18nc("@action", "Link"), this);
    d->richTextActionList.append((d->action_manage_link));
    d->action_manage_link->setObjectName(QStringLiteral("manage_link"));
    ac->addAction(QStringLiteral("manage_link"), d->action_manage_link);
    connect(d->action_manage_link, &QAction::triggered,
            d->composerControler, &RichTextComposerControler::manageLink);

    d->action_list_indent = new QAction(QIcon::fromTheme(QStringLiteral("format-indent-more")),
                                        i18nc("@action", "Increase Indent"), this);
    d->richTextActionList.append((d->action_list_indent));
    d->action_list_indent->setObjectName(QStringLiteral("format_list_indent_more"));
    ac->addAction(QStringLiteral("format_list_indent_more"), d->action_list_indent);
    connect(d->action_list_indent, &QAction::triggered,
            d->composerControler, &RichTextComposerControler::indentListMore);
    connect(d->action_list_indent, &QAction::triggered,
            this, &RichTextComposerActions::slotUpdateMiscActions);
    d->action_list_dedent = new QAction(QIcon::fromTheme(QStringLiteral("format-indent-less")),
                                        i18nc("@action", "Decrease Indent"), this);
    d->richTextActionList.append((d->action_list_dedent));
    d->action_list_dedent->setObjectName(QStringLiteral("format_list_indent_less"));
    ac->addAction(QStringLiteral("format_list_indent_less"), d->action_list_dedent);
    connect(d->action_list_dedent, &QAction::triggered,
            d->composerControler, &RichTextComposerControler::indentListLess);
    connect(d->action_list_dedent, &QAction::triggered,
            this, &RichTextComposerActions::slotUpdateMiscActions);

    d->action_list_style = new KSelectAction(QIcon::fromTheme(QStringLiteral("format-list-unordered")),
            i18nc("@title:menu", "List Style"), this);
    QStringList listStyles;
    listStyles      << i18nc("@item:inmenu no list style", "None")
                    << i18nc("@item:inmenu disc list style", "Disc")
                    << i18nc("@item:inmenu circle list style", "Circle")
                    << i18nc("@item:inmenu square list style", "Square")
                    << i18nc("@item:inmenu numbered lists", "123")
                    << i18nc("@item:inmenu lowercase abc lists", "abc")
                    << i18nc("@item:inmenu uppercase abc lists", "ABC")
                    << i18nc("@item:inmenu lower case roman numerals", "i ii iii")
                    << i18nc("@item:inmenu upper case roman numerals", "I II III");

    d->action_list_style->setItems(listStyles);
    d->action_list_style->setCurrentItem(0);
    d->richTextActionList.append((d->action_list_style));
    d->action_list_style->setObjectName(QStringLiteral("format_list_style"));
    ac->addAction(QStringLiteral("format_list_style"), d->action_list_style);
    connect(d->action_list_style, SIGNAL(triggered(int)),
            this, SLOT(setListStyle(int)));
    connect(d->action_list_style, SIGNAL(triggered()),
            this, SLOT(slotUpdateMiscActions()));

    d->action_paste_quotation = new QAction(i18n("Pa&ste as Quotation"), this);
    d->action_paste_quotation->setObjectName(QStringLiteral("paste_quoted"));
    ac->addAction(QStringLiteral("paste_quoted"), d->action_paste_quotation);
    ac->setDefaultShortcut(d->action_paste_quotation, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_O));
    connect(d->action_paste_quotation, &QAction::triggered, d->composerControler, &RichTextComposerControler::slotPasteAsQuotation);

    d->action_add_quote_chars = new QAction(i18n("Add &Quote Characters"), this);
    d->action_add_quote_chars->setObjectName(QStringLiteral("tools_quote"));
    ac->addAction(QStringLiteral("tools_quote"), d->action_add_quote_chars);
    connect(d->action_add_quote_chars, &QAction::triggered, d->composerControler, &RichTextComposerControler::slotAddQuotes);

    d->action_remove_quote_chars = new QAction(i18n("Re&move Quote Characters"), this);
    d->action_remove_quote_chars->setObjectName(QStringLiteral("tools_unquote"));
    connect(d->action_remove_quote_chars, &QAction::triggered, d->composerControler, &RichTextComposerControler::slotRemoveQuotes);
    ac->addAction(QStringLiteral("tools_unquote"), d->action_remove_quote_chars);

    d->action_paste_without_formatting = new QAction(i18n("Paste Without Formatting"), this);
    d->action_paste_without_formatting->setObjectName(QStringLiteral("paste_without_formatting"));
    ac->addAction(QStringLiteral("paste_without_formatting"), d->action_paste_without_formatting);
    ac->setDefaultShortcut(d->action_paste_without_formatting, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_V));
    connect(d->action_paste_without_formatting, &QAction::triggered, d->composerControler, &RichTextComposerControler::slotPasteWithoutFormatting);

    d->action_add_image = new QAction(QIcon::fromTheme(QStringLiteral("insert-image")),
                                      i18n("Add Image"), this);
    d->action_add_image->setObjectName(QStringLiteral("add_image"));
    ac->addAction(QStringLiteral("add_image"), d->action_add_image);
    connect(d->action_add_image, &QAction::triggered, d->composerControler, &RichTextComposerControler::slotAddImage);
    d->richTextActionList.append(d->action_add_image);

    d->action_add_emoticon = new KPIMTextEdit::EmoticonTextEditAction(this);
    d->action_add_emoticon->setObjectName(QStringLiteral("add_emoticon"));
    ac->addAction(QStringLiteral("add_emoticon"), d->action_add_emoticon);
    connect(d->action_add_emoticon, SIGNAL(emoticonActivated(QString)), d->composerControler, SLOT(slotAddEmoticon(QString)));
    d->richTextActionList.append(d->action_add_emoticon);

    d->action_insert_html = new QAction(i18n("Insert HTML"), this);
    d->action_insert_html->setObjectName(QStringLiteral("insert_html"));
    ac->addAction(QStringLiteral("insert_html"), d->action_insert_html);
    connect(d->action_insert_html, &QAction::triggered, d->composerControler, &RichTextComposerControler::slotInsertHtml);
    d->richTextActionList.append(d->action_insert_html);

    d->action_add_table = new KPIMTextEdit::TableActionMenu(d->composerControler->richTextComposer());
    d->action_add_table->setIcon(QIcon::fromTheme(QStringLiteral("insert-table")));
    d->action_add_table->setText(i18n("Table"));
    d->action_add_table->setDelayed(false);
    d->action_add_table->setObjectName(QStringLiteral("insert_table"));
    d->richTextActionList.append(d->action_add_table);
    ac->addAction(QStringLiteral("insert_table"), d->action_add_table);

    d->action_delete_line = new QAction(i18n("Delete Line"), this);
    ac->setDefaultShortcut(d->action_delete_line, QKeySequence(Qt::CTRL + Qt::Key_K));
    d->action_delete_line->setObjectName(QStringLiteral("delete_line"));
    ac->addAction(QStringLiteral("delete_line"), d->action_delete_line);
    connect(d->action_delete_line, &QAction::triggered, d->composerControler, &RichTextComposerControler::slotDeleteLine);
    d->richTextActionList.append(d->action_delete_line);

    d->action_format_reset =
        new QAction(QIcon::fromTheme(QStringLiteral("draw-eraser")), i18n("Reset Font Settings"), this);
    d->action_format_reset->setIconText(i18n("Reset Font"));
    d->action_format_reset->setObjectName(QStringLiteral("format_reset"));
    connect(d->action_format_reset, &QAction::triggered, d->composerControler, &RichTextComposerControler::slotFormatReset);
    ac->addAction(QStringLiteral("format_reset"), d->action_format_reset);
    d->richTextActionList.append(d->action_format_reset);

    d->action_format_painter = new KToggleAction(QIcon::fromTheme(QStringLiteral("draw-brush")),
            i18nc("@action", "Format Painter"), this);
    d->richTextActionList.append(d->action_format_painter);
    d->action_format_painter->setObjectName(QStringLiteral("format_painter"));
    ac->addAction(QStringLiteral("format_painter"), d->action_format_painter);
    connect(d->action_format_painter, &QAction::toggled,
            d->composerControler, &RichTextComposerControler::slotFormatPainter);

    disconnect(d->composerControler->richTextComposer(), &QTextEdit::currentCharFormatChanged,
               this, &RichTextComposerActions::slotUpdateCharFormatActions);
    disconnect(d->composerControler->richTextComposer(), &QTextEdit::cursorPositionChanged,
               this, &RichTextComposerActions::slotUpdateMiscActions);

    connect(d->composerControler->richTextComposer(), &QTextEdit::currentCharFormatChanged,
            this, &RichTextComposerActions::slotUpdateCharFormatActions);
    connect(d->composerControler->richTextComposer(), &QTextEdit::cursorPositionChanged,
            this, &RichTextComposerActions::slotUpdateMiscActions);

    updateActionStates();
}

void RichTextComposerActions::updateActionStates()
{
    slotUpdateMiscActions();
    slotUpdateCharFormatActions(d->composerControler->richTextComposer()->currentCharFormat());
}

void RichTextComposerActions::setListStyle(int _styleindex)
{
    d->composerControler->setListStyle(_styleindex);
    //Needed ?
    slotUpdateMiscActions();
}

void RichTextComposerActions::setActionsEnabled(bool enabled)
{
    Q_FOREACH (QAction *action, d->richTextActionList) {
        action->setEnabled(enabled);
    }
    d->richTextEnabled = enabled;
}

void RichTextComposerActions::slotUpdateCharFormatActions(const QTextCharFormat &format)
{
    QFont f = format.font();

    d->action_font_family->setFont(f.family());
    if (f.pointSize() > 0) {
        d->action_font_size->setFontSize((int)f.pointSize());
    }
    d->action_text_bold->setChecked(f.bold());
    d->action_text_italic->setChecked(f.italic());
    d->action_text_underline->setChecked(f.underline());
    d->action_text_strikeout->setChecked(f.strikeOut());
    const QTextCharFormat::VerticalAlignment vAlign = format.verticalAlignment();
    d->action_text_superscript->setChecked(vAlign == QTextCharFormat::AlignSuperScript);
    d->action_text_subscript->setChecked(vAlign == QTextCharFormat::AlignSubScript);
}

void RichTextComposerActions::slotUpdateMiscActions()
{
    const Qt::Alignment a = d->composerControler->richTextComposer()->alignment();
    if (a & Qt::AlignLeft) {
        d->action_align_left->setChecked(true);
    } else if (a & Qt::AlignHCenter) {
        d->action_align_center->setChecked(true);
    } else if (a & Qt::AlignRight) {
        d->action_align_right->setChecked(true);
    } else if (a & Qt::AlignJustify) {
        d->action_align_justify->setChecked(true);
    }
    if (d->composerControler->richTextComposer()->textCursor().currentList()) {
        d->action_list_style->setCurrentItem(-d->composerControler->richTextComposer()->textCursor().currentList()->format().style());
    } else {
        d->action_list_style->setCurrentItem(0);
    }
    if (d->richTextEnabled) {
        d->action_list_indent->setEnabled(d->composerControler->canIndentList());
    } else {
        d->action_list_indent->setEnabled(false);
    }
    if (d->richTextEnabled) {
        d->action_list_dedent->setEnabled(d->composerControler->canDedentList());
    } else {
        d->action_list_dedent->setEnabled(false);
    }

    const Qt::LayoutDirection direction = d->composerControler->richTextComposer()->textCursor().blockFormat().layoutDirection();
    d->action_direction_ltr->setChecked(direction == Qt::LeftToRight);
    d->action_direction_rtl->setChecked(direction == Qt::RightToLeft);
}

void RichTextComposerActions::uncheckActionFormatPainter()
{
    d->action_format_painter->setChecked(false);
}

void RichTextComposerActions::textModeChanged(MessageComposer::RichTextComposer::Mode mode)
{
    d->action_add_table->setRichTextMode(mode == MessageComposer::RichTextComposer::Rich);
}
