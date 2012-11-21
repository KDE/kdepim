/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "composereditor.h"
#include <KAction>
#include <KToggleAction>
#include <KLocale>
#include <KSelectAction>

#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebPage>



namespace ComposerEditorNG {

#define FORWARD_ACTION(action1, action2) \
    connect(action1, SIGNAL(triggered()), d->getAction(action2), SLOT(trigger()));\
    connect(d->getAction(action2), SIGNAL(changed()), SLOT(_k_slotAdjustActions()));

#define FOLLOW_CHECK(a1, a2) a1->setChecked(d->getAction(a2)->isChecked())

class ComposerEditorPrivate
{
public:
    ComposerEditorPrivate(ComposerEditor *qq)
        : q(qq),
          richTextEnabled(true)
    {

    }

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
    void _k_setListStyle(int);
    void _k_setFormatType(QAction* action);

    QAction* getAction ( QWebPage::WebAction action ) const;
    void execCommand(const QString &cmd);
    void execCommand(const QString &cmd, const QString &arg);

    QList<KAction*> richTextActionList;
    ComposerEditor *q;
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
    KSelectAction *action_list_style;
    KSelectAction *action_format_type;
    bool richTextEnabled;
};
Q_DECLARE_METATYPE(ComposerEditorNG::ComposerEditorPrivate::FormatType)

QAction* ComposerEditorPrivate::getAction ( QWebPage::WebAction action ) const
{
    if ( action >= 0 && action <= 66 )
        return q->page()->action( static_cast<QWebPage::WebAction>( action ));
    else
        return 0;
}

void ComposerEditorPrivate::_k_setListStyle(int style)
{
//TODO
}

void ComposerEditorPrivate::_k_setFormatType(QAction *act)
{
    if(!act) {
        return;
    }
    QString command;
    switch(act->data().value<ComposerEditorNG::ComposerEditorPrivate::FormatType>())
    {
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
    execCommand ( "formatBlock", command );
}

void ComposerEditorPrivate::_k_slotAdjustActions()
{
    //TODO
    FOLLOW_CHECK(action_text_bold, QWebPage::ToggleBold);
    FOLLOW_CHECK(action_text_italic, QWebPage::ToggleItalic);
    FOLLOW_CHECK(action_text_strikeout, QWebPage::ToggleStrikethrough);
    FOLLOW_CHECK(action_text_underline, QWebPage::ToggleUnderline);
    FOLLOW_CHECK(action_text_subscript, QWebPage::ToggleSubscript);
    FOLLOW_CHECK(action_text_superscript, QWebPage::ToggleSuperscript);
}

void ComposerEditorPrivate::execCommand(const QString &cmd)
{
    QWebFrame *frame = q->page()->mainFrame();
    const QString js = QString::fromLatin1("document.execCommand(\"%1\", false, null)").arg(cmd);
    frame->evaluateJavaScript(js);
}

void ComposerEditorPrivate::execCommand(const QString &cmd, const QString &arg)
{
    QWebFrame *frame = q->page()->mainFrame();
    const QString js = QString::fromLatin1("document.execCommand(\"%1\", false, \"%2\")").arg(cmd).arg(arg);
    frame->evaluateJavaScript(js);
}

ComposerEditor::ComposerEditor(QWidget *parent)
    : KWebView(parent), d(new ComposerEditorPrivate)
{
    page()->setContentEditable(true);
}

ComposerEditor::~ComposerEditor()
{
  delete d;
}

void ComposerEditor::createActions(KActionCollection *actionCollection)
{
    Q_ASSERT(actionCollection);

    //format
    d->action_text_bold = new KToggleAction(KIcon("format-text-bold"), i18nc("@action boldify selected text", "&Bold"), actionCollection);
    QFont bold;
    bold.setBold(true);
    d->action_text_bold->setFont(bold);
    d->richTextActionList.append((d->action_text_bold));
    actionCollection->addAction("htmleditor_format_text_bold", d->action_text_bold);
    d->action_text_bold->setShortcut(KShortcut(Qt::CTRL + Qt::Key_B));
    FORWARD_ACTION(d->action_text_bold, QWebPage::ToggleBold);

    d->action_text_italic = new KToggleAction(KIcon("format-text-italic"), i18nc("@action italicize selected text", "&Italic"), actionCollection);
    QFont italic;
    italic.setItalic(true);
    d->action_text_italic->setFont(italic);
    d->richTextActionList.append((d->action_text_italic));
    actionCollection->addAction("htmleditor_format_text_italic", d->action_text_italic);
    d->action_text_italic->setShortcut(KShortcut(Qt::CTRL + Qt::Key_I));
    FORWARD_ACTION(d->action_text_italic, QWebPage::ToggleItalic);

    d->action_text_underline = new KToggleAction(KIcon("format-text-underline"), i18nc("@action underline selected text", "&Underline"), actionCollection);
    QFont underline;
    underline.setUnderline(true);
    d->action_text_underline->setFont(underline);
    d->richTextActionList.append((d->action_text_underline));
    actionCollection->addAction("htmleditor_format_text_underline", d->action_text_underline);
    d->action_text_underline->setShortcut(KShortcut(Qt::CTRL + Qt::Key_U));
    FORWARD_ACTION(d->action_text_italic, QWebPage::ToggleUnderline);

    d->action_text_strikeout = new KToggleAction(KIcon("format-text-strikethrough"), i18nc("@action", "&Strike Out"), actionCollection);
    d->richTextActionList.append((d->action_text_strikeout));
    actionCollection->addAction("htmleditor_format_text_strikeout", d->action_text_strikeout);
    d->action_text_strikeout->setShortcut(KShortcut(Qt::CTRL + Qt::Key_L));
    FORWARD_ACTION(d->action_text_italic, QWebPage::ToggleStrikethrough);

    //Alignment
    d->action_align_left = new KToggleAction(KIcon("format-justify-left"), i18nc("@action", "Align &Left"), actionCollection);
    d->action_align_left->setIconText(i18nc("@label left justify", "Left"));
    d->richTextActionList.append((d->action_align_left));
    actionCollection->addAction("htmleditor_format_align_left", d->action_align_left);
    FORWARD_ACTION(d->action_align_left, QWebPage::AlignLeft);

    d->action_align_center = new KToggleAction(KIcon("format-justify-center"), i18nc("@action", "Align &Center"), actionCollection);
    d->action_align_center->setIconText(i18nc("@label center justify", "Center"));
    d->richTextActionList.append((d->action_align_center));
    actionCollection->addAction("htmleditor_format_align_center", d->action_align_center);
    FORWARD_ACTION(d->action_align_center, QWebPage::AlignCenter);

    d->action_align_right = new KToggleAction(KIcon("format-justify-right"), i18nc("@action", "Align &Right"), actionCollection);
    d->action_align_right->setIconText(i18nc("@label right justify", "Right"));
    d->richTextActionList.append((d->action_align_right));
    actionCollection->addAction("htmleditor_format_align_right", d->action_align_right);
    FORWARD_ACTION(d->action_align_right, QWebPage::AlignRight);

    d->action_align_justify = new KToggleAction(KIcon("format-justify-fill"), i18nc("@action", "&Justify"), actionCollection);
    d->action_align_justify->setIconText(i18nc("@label justify fill", "Justify"));
    d->richTextActionList.append((d->action_align_justify));
    actionCollection->addAction("htmleditor_format_align_justify", d->action_align_justify);
    FORWARD_ACTION(d->action_align_justify, QWebPage::AlignJustified);

    QActionGroup *alignmentGroup = new QActionGroup(this);
    alignmentGroup->addAction(d->action_align_left);
    alignmentGroup->addAction(d->action_align_center);
    alignmentGroup->addAction(d->action_align_right);
    alignmentGroup->addAction(d->action_align_justify);

    //Direction
    d->action_direction_ltr = new KToggleAction(KIcon("format-text-direction-ltr"), i18nc("@action", "Left-to-Right"), actionCollection);
    d->action_direction_ltr->setIconText(i18nc("@label left-to-right", "Left-to-Right"));
    d->richTextActionList.append(d->action_direction_ltr);
    actionCollection->addAction("htmleditor_direction_ltr", d->action_direction_ltr);
    FORWARD_ACTION(d->action_direction_ltr, QWebPage::SetTextDirectionLeftToRight);

    d->action_direction_rtl = new KToggleAction(KIcon("format-text-direction-rtl"), i18nc("@action", "Right-to-Left"), actionCollection);
    d->action_direction_rtl->setIconText(i18nc("@label right-to-left", "Right-to-Left"));
    d->richTextActionList.append(d->action_direction_rtl);
    actionCollection->addAction("htmleditor_direction_rtl", d->action_direction_rtl);
    FORWARD_ACTION(d->action_direction_ltr, QWebPage::SetTextDirectionRightToLeft);

    QActionGroup *directionGroup = new QActionGroup(this);
    directionGroup->addAction(d->action_direction_ltr);
    directionGroup->addAction(d->action_direction_rtl);


    //indent
    d->action_list_indent = new KAction(KIcon("format-indent-more"), i18nc("@action", "Increase Indent"), actionCollection);
    d->richTextActionList.append((d->action_list_indent));
    actionCollection->addAction("htmleditor_format_list_indent_more", d->action_list_indent);
    FORWARD_ACTION(d->action_list_indent, QWebPage::Indent);

    d->action_list_dedent = new KAction(KIcon("format-indent-less"), i18nc("@action", "Decrease Indent"), actionCollection);
    d->richTextActionList.append(d->action_list_dedent);
    actionCollection->addAction("htmleditor_format_list_indent_less", d->action_list_dedent);
    FORWARD_ACTION(d->action_list_indent, QWebPage::Outdent);

    //horizontal line
    d->action_insert_horizontal_rule = new KAction(KIcon("insert-horizontal-rule"), i18nc("@action", "Insert Rule Line"), actionCollection);
    d->richTextActionList.append((d->action_insert_horizontal_rule));
    actionCollection->addAction("htmleditor_insert_horizontal_rule", d->action_insert_horizontal_rule);
    FORWARD_ACTION(d->action_insert_horizontal_rule, QWebPage::InsertLineSeparator); //Verify


    //Superscript/subScript
    d->action_text_subscript = new KToggleAction(KIcon("format-text-subscript"), i18nc("@action", "Subscript"), actionCollection);
    d->richTextActionList.append((d->action_text_subscript));
    actionCollection->addAction("htmleditor_format_text_subscript", d->action_text_subscript);
    FORWARD_ACTION(d->action_insert_horizontal_rule, QWebPage::ToggleSubscript);

    d->action_text_superscript = new KToggleAction(KIcon("format-text-superscript"), i18nc("@action", "Superscript"), actionCollection);
    d->richTextActionList.append((d->action_text_superscript));
    actionCollection->addAction("htmleditor_format_text_superscript", d->action_text_superscript);
    FORWARD_ACTION(d->action_insert_horizontal_rule, QWebPage::ToggleSuperscript);

    d->action_list_style = new KSelectAction(KIcon("format-list-unordered"), i18nc("@title:menu", "List Style"), actionCollection);
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
    actionCollection->addAction("htmleditor_format_list_style", d->action_list_style);
    connect(d->action_list_style, SIGNAL(triggered(int)),
            this, SLOT(_k_setListStyle(int)));

    d->action_format_type = new KSelectAction(KIcon("format-list-unordered"), i18nc("@title:menu", "List Style"), actionCollection);
    KAction *act = d->action_format_type->addAction(i18n( "Paragraph" ));
    act->setData(ComposerEditorPrivate::Paragraph);
    act = d->action_format_type->addAction(i18n( "Heading 1" ));
    act->setData(ComposerEditorPrivate::Header1);
    act = d->action_format_type->addAction(i18n( "Heading 2" ));
    act->setData(ComposerEditorPrivate::Header2);

    act = d->action_format_type->addAction(i18n( "Heading 3" ));
    act->setData(ComposerEditorPrivate::Header3);

    act = d->action_format_type->addAction(i18n( "Heading 4" ));
    act->setData(ComposerEditorPrivate::Header4);

    act = d->action_format_type->addAction(i18n( "Heading 5" ));
    act->setData(ComposerEditorPrivate::Header5);

    act = d->action_format_type->addAction(i18n( "Heading 6" ));
    act->setData(ComposerEditorPrivate::Header6);

    act = d->action_format_type->addAction(i18n( "Pre" ));
    act->setData(ComposerEditorPrivate::Pre);

    act = d->action_format_type->addAction(i18n( "Address" ));
    act->setData(ComposerEditorPrivate::Address);
    d->action_format_type->setCurrentItem(0);
    d->richTextActionList.append(d->action_format_type);
    actionCollection->addAction("htmleditor_format_type", d->action_format_type);
    connect(d->action_format_type, SIGNAL(triggered(QAction)),
            this, SLOT(_k_setFormatType(QAction)));

}


QString ComposerEditor::plainTextContent() const
{
    return page()->mainFrame()->toPlainText();
}

void ComposerEditor::setEnableRichText(bool richTextEnabled)
{
    d->richTextEnabled = richTextEnabled;
}

bool ComposerEditor::enableRichText() const
{
    return d->richTextEnabled;
}

void ComposerEditor::setActionsEnabled(bool enabled)
{
    foreach(QAction* action, d->richTextActionList)
    {
        action->setEnabled(enabled);
    }
    d->richTextEnabled = enabled;
}




}
