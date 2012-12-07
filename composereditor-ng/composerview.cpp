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

#include "composerview.h"
#include "managelink.h"
#include "pagecolorbackgrounddialog.h"
#include "composereditorutil_p.h"
#include "composerimagedialog.h"


#include <kpimtextedit/emoticontexteditaction.h>
#include <kpimtextedit/inserthtmldialog.h>
#include <kpimtextedit/insertimagedialog.h>
#include <kpimtextedit/inserttabledialog.h>

#include <Sonnet/Dialog>
#include <sonnet/backgroundchecker.h>


#include <KMenu>
#include <KMessageBox>
#include <KToolInvocation>
#include <KLocale>
#include <KAction>
#include <KFontAction>
#include <KToggleAction>
#include <KAction>
#include <KSelectAction>
#include <KActionCollection>
#include <KColorDialog>
#include <KMessageBox>
#include <KStandardDirs>
#include <KDebug>
#include <KFontAction>
#include <KMenu>


#include <QAction>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QWebFrame>
#include <QWebElement>
#include <QContextMenuEvent>
#include <QDebug>
#include <QPointer>


namespace ComposerEditorNG {

#define FORWARD_ACTION(action1, action2) \
    connect(action1, SIGNAL(triggered()), d->getAction(action2), SLOT(trigger()));\
    connect(d->getAction(action2), SIGNAL(changed()), SLOT(_k_slotAdjustActions()));

#define FOLLOW_CHECK(a1, a2) a1->setChecked(getAction(a2)->isChecked())


class ComposerViewPrivate
{
public:
    ComposerViewPrivate( ComposerView *qq)
        : q(qq)
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

    QAction* getAction ( QWebPage::WebAction action ) const;
    QVariant evaluateJavascript(const QString& command);
    void execCommand(const QString &cmd);
    void execCommand(const QString &cmd, const QString &arg);
    bool queryCommandState(const QString &cmd);

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

    ComposerView *q;
};
}

Q_DECLARE_METATYPE(ComposerEditorNG::ComposerViewPrivate::FormatType)

namespace ComposerEditorNG {

QAction* ComposerViewPrivate::getAction ( QWebPage::WebAction action ) const
{
    if ( action >= 0 && action <= 66 )
        return q->page()->action( static_cast<QWebPage::WebAction>( action ));
    else
        return 0;
}


static QVariant execJScript(QWebElement element, const QString& script)
{
    if (element.isNull())
        return QVariant();
    return element.evaluateJavaScript(script);
}

void ComposerViewPrivate::_k_setFormatType(QAction *act)
{
    if(!act) {
        return;
    }
    QString command;
    switch(act->data().value<ComposerEditorNG::ComposerViewPrivate::FormatType>())
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
    execCommand ( QLatin1String("formatBlock"), command );
}

void ComposerViewPrivate::_k_slotToggleBlockQuote()
{
    execCommand( QLatin1String("formatBlock"), QLatin1String("BLOCKQUOTE"));
}

void ComposerViewPrivate::_k_slotAddEmoticon(const QString& emoticon)
{
    execCommand(QLatin1String("insertHTML"), emoticon);
}

void ComposerViewPrivate::_k_slotInsertHtml()
{
    QPointer<KPIMTextEdit::InsertHtmlDialog> dialog = new KPIMTextEdit::InsertHtmlDialog( q );
    if ( dialog->exec() ) {
        const QString str = dialog->html().remove(QLatin1String("\n"));
        if ( !str.isEmpty() ) {
            execCommand(QLatin1String("insertHTML"), str);
        }
    }
    delete dialog;
}

void ComposerViewPrivate::_k_setTextBackgroundColor()
{
    QColor newColor = ComposerEditorNG::Util::convertRgbToQColor(evaluateJavascript(QLatin1String("getTextBackgroundColor()")).toString());
    const int result = KColorDialog::getColor(newColor,q);
    if(result == QDialog::Accepted) {
        execCommand(QLatin1String("hiliteColor"), newColor.name());
    }
}

QVariant ComposerViewPrivate::evaluateJavascript(const QString& command)
{
    return q->page()->mainFrame()->evaluateJavaScript( command );
}

void ComposerViewPrivate::_k_slotDeleteText()
{
    evaluateJavascript(QLatin1String("setDeleteSelectedText()"));
}

void ComposerViewPrivate::_k_setTextForegroundColor()
{
    QColor newColor = ComposerEditorNG::Util::convertRgbToQColor(evaluateJavascript(QLatin1String("getTextForegroundColor()")).toString());
    const int result = KColorDialog::getColor(newColor,q);
    if(result == QDialog::Accepted) {
        execCommand(QLatin1String("foreColor"), newColor.name());
    }
}

void ComposerViewPrivate::_k_slotAddImage()
{
    QPointer<ComposerImageDialog> dlg = new ComposerImageDialog( q );
    if ( dlg->exec() == KDialog::Accepted ) {
        execCommand(QLatin1String("insertHTML"), dlg->html());
    }
    delete dlg;
}

void ComposerViewPrivate::_k_slotEditImage()
{
    ComposerImageDialog dlg( contextMenuResult.element(),q );
    dlg.exec();
}

void ComposerViewPrivate::_k_slotInsertTable()
{
    QPointer<KPIMTextEdit::InsertTableDialog> dlg = new KPIMTextEdit::InsertTableDialog( q );
    if( dlg->exec() == KDialog::Accepted ) {

        const int numberOfColumns( dlg->columns() );
        const int numberRow( dlg->rows() );

        QString htmlTable = QString::fromLatin1("<table border='%1'>").arg(dlg->border());
        for(int i = 0; i <numberRow; ++i) {
            htmlTable += QLatin1String("<tr>");
            for(int j = 0; j <numberOfColumns; ++j) {
                htmlTable += QLatin1String("<td></td>");
            }
            htmlTable += QLatin1String("</tr>");
        }
        htmlTable += QLatin1String("</table>");
        execCommand(QLatin1String("insertHTML"), htmlTable);
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
    QPointer<ComposerEditorNG::ManageLink> dlg = new ComposerEditorNG::ManageLink( selectedText, q );
    if( dlg->exec() == KDialog::Accepted ) {
        const QString html(dlg->html());
        if(!html.isEmpty())
            execCommand ( QLatin1String("insertHTML"), html );
    }
    delete dlg;
}

void ComposerViewPrivate::_k_slotEditLink()
{
    ComposerEditorNG::ManageLink dlg( contextMenuResult.linkElement(), q );
    dlg.exec();
}

void ComposerViewPrivate::_k_setFontSize(int fontSize)
{
    execCommand(QLatin1String("fontSize"), QString::number(fontSize+1)); //Verify
}

void ComposerViewPrivate::_k_setFontFamily(const QString& family)
{
    execCommand(QLatin1String("fontName"), family);
}

void ComposerViewPrivate::_k_slotSpellCheck()
{
    QString text(execJScript(contextMenuResult.element(), QLatin1String("this.value")).toString());
    if (contextMenuResult.isContentSelected())
    {
        spellTextSelectionStart = qMax(0, execJScript(contextMenuResult.element(), QLatin1String("this.selectionStart")).toInt());
        spellTextSelectionEnd = qMax(0, execJScript(contextMenuResult.element(), QLatin1String("this.selectionEnd")).toInt());
        text = text.mid(spellTextSelectionStart, (spellTextSelectionEnd - spellTextSelectionStart));
    }
    else
    {
        spellTextSelectionStart = 0;
        spellTextSelectionEnd = 0;
    }
    if (text.isEmpty())
    {
        return;
    }

    Sonnet::BackgroundChecker *backgroundSpellCheck = new Sonnet::BackgroundChecker;
    Sonnet::Dialog* spellDialog = new Sonnet::Dialog(backgroundSpellCheck, q);
    backgroundSpellCheck->setParent(spellDialog);
    spellDialog->setAttribute(Qt::WA_DeleteOnClose, true);

    spellDialog->showSpellCheckCompletionMessage(true);
    q->connect(spellDialog, SIGNAL(replace(QString, int, QString)), q, SLOT(_k_spellCheckerCorrected(QString, int, QString)));
    q->connect(spellDialog, SIGNAL(misspelling(QString, int)), q, SLOT(_k_spellCheckerMisspelling(QString, int)));
    if (contextMenuResult.isContentSelected())
        q->connect(spellDialog, SIGNAL(done(QString)), q, SLOT(_k_slotSpellCheckDone(QString)));
    spellDialog->setBuffer(text);
    spellDialog->show();
}

void ComposerViewPrivate::_k_spellCheckerCorrected(const QString& original, int pos, const QString& replacement)
{
    // Adjust the selection end...
    if (spellTextSelectionEnd > 0)
    {
        spellTextSelectionEnd += qMax(0, (replacement.length() - original.length()));
    }

    const int index = pos + spellTextSelectionStart;
    QString script(QLatin1String("this.value=this.value.substring(0,"));
    script += QString::number(index);
    script += QLatin1String(") + \"");
    script +=  replacement;
    script += QLatin1String("\" + this.value.substring(");
    script += QString::number(index + original.length());
    script += QLatin1String(")");

    //kDebug() << "**** script:" << script;
    execJScript(contextMenuResult.element(), script);
}

void ComposerViewPrivate::_k_spellCheckerMisspelling(const QString& text, int pos)
{
    // kDebug() << text << pos;
    QString selectionScript(QLatin1String("this.setSelectionRange("));
    selectionScript += QString::number(pos + spellTextSelectionStart);
    selectionScript += QLatin1Char(',');
    selectionScript += QString::number(pos + text.length() + spellTextSelectionStart);
    selectionScript += QLatin1Char(')');
    execJScript(contextMenuResult.element(), selectionScript);
}

void ComposerViewPrivate::_k_slotSpellCheckDone(const QString&)
{
    // Restore the text selection if one was present before we started the
    // spell check.
    if (spellTextSelectionStart > 0 || spellTextSelectionEnd > 0)
    {
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

void ComposerViewPrivate::_k_slotChangePageColorAndBackground()
{
    PageColorBackgroundDialog dlg(q->page()->mainFrame(), q);
    dlg.exec();
}

void ComposerViewPrivate::_k_slotAdjustActions()
{
    FOLLOW_CHECK(action_text_bold, QWebPage::ToggleBold);
    FOLLOW_CHECK(action_text_italic, QWebPage::ToggleItalic);
    FOLLOW_CHECK(action_text_strikeout, QWebPage::ToggleStrikethrough);
    FOLLOW_CHECK(action_text_underline, QWebPage::ToggleUnderline);
    FOLLOW_CHECK(action_text_subscript, QWebPage::ToggleSubscript);
    FOLLOW_CHECK(action_text_superscript, QWebPage::ToggleSuperscript);
    FOLLOW_CHECK(action_ordered_list, QWebPage::InsertOrderedList);
    FOLLOW_CHECK(action_unordered_list, QWebPage::InsertUnorderedList);
    FOLLOW_CHECK(action_direction_ltr, QWebPage::SetTextDirectionLeftToRight);
    FOLLOW_CHECK(action_direction_rtl, QWebPage::SetTextDirectionRightToLeft);

    const QString alignment = evaluateJavascript(QLatin1String("getAlignment()")).toString();
    if(alignment == QLatin1String("left")) {
        action_align_left->setChecked(true);
    } else if(alignment == QLatin1String("right")) {
        action_align_right->setChecked(true);
    } else if(alignment == QLatin1String("center")) {
        action_align_center->setChecked(true);
    } else if(alignment == QLatin1String("-webkit-auto")) {
        action_align_justify->setChecked(true);
    }
    const QString font = evaluateJavascript(QLatin1String("getFontFamily()")).toString();
    if(!font.isEmpty()) {
      action_font_family->setFont(font);
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
    // If KTTSD not running, start it.
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.kde.kttsd")))
    {
        QString error;
        if (KToolInvocation::startServiceByDesktopName(QLatin1String("kttsd"), QStringList(), &error))
        {
            KMessageBox::error(q, i18n( "Starting Jovie Text-to-Speech Service Failed"), error );
            return;
        }
    }
    QDBusInterface ktts(QLatin1String("org.kde.kttsd"), QLatin1String("/KSpeech"), QLatin1String("org.kde.KSpeech"));
    QString text = q->selectedText();
    if(text.isEmpty())
        text = q->page()->mainFrame()->toPlainText();
    ktts.asyncCall(QLatin1String("say"), text, 0);
}

ComposerView::ComposerView(QWidget *parent)
    : KWebView(parent),d(new ComposerViewPrivate(this))
{
    QFile file ( KStandardDirs::locate ( "data", QLatin1String("composereditor/composereditorinitialhtml") ) );
    kDebug() <<file.fileName();
    if ( !file.open ( QIODevice::ReadOnly ) )
        KMessageBox::error(this, i18n ( "Cannot open template file." ), i18n ( "composer editor" ));
    else
        setContent ( file.readAll());//, "application/xhtml+xml" );

    page()->setContentEditable(true);
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(this, SIGNAL(linkClicked(QUrl)), SIGNAL(openLink(QUrl)));
    connect( page(), SIGNAL (selectionChanged()), this, SLOT(_k_slotAdjustActions()) );

    setWindowModified(false);

}

ComposerView::~ComposerView()
{
    delete d;
}

void ComposerView::createActions(KActionCollection *actionCollection)
{
    Q_ASSERT(actionCollection);
    d->htmlEditorActionList.clear();

    //format
    d->action_text_bold = new KToggleAction(KIcon(QLatin1String("format-text-bold")), i18nc("@action boldify selected text", "&Bold"), actionCollection);
    QFont bold;
    bold.setBold(true);
    d->action_text_bold->setFont(bold);
    d->htmlEditorActionList.append((d->action_text_bold));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_bold"), d->action_text_bold);
    d->action_text_bold->setShortcut(KShortcut(Qt::CTRL + Qt::Key_B));
    FORWARD_ACTION(d->action_text_bold, QWebPage::ToggleBold);

    d->action_text_italic = new KToggleAction(KIcon(QLatin1String("format-text-italic")), i18nc("@action italicize selected text", "&Italic"), actionCollection);
    QFont italic;
    italic.setItalic(true);
    d->action_text_italic->setFont(italic);
    d->htmlEditorActionList.append((d->action_text_italic));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_italic"), d->action_text_italic);
    d->action_text_italic->setShortcut(KShortcut(Qt::CTRL + Qt::Key_I));
    FORWARD_ACTION(d->action_text_italic, QWebPage::ToggleItalic);

    d->action_text_underline = new KToggleAction(KIcon(QLatin1String("format-text-underline")), i18nc("@action underline selected text", "&Underline"), actionCollection);
    QFont underline;
    underline.setUnderline(true);
    d->action_text_underline->setFont(underline);
    d->htmlEditorActionList.append((d->action_text_underline));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_underline"), d->action_text_underline);
    d->action_text_underline->setShortcut(KShortcut(Qt::CTRL + Qt::Key_U));
    FORWARD_ACTION(d->action_text_underline, QWebPage::ToggleUnderline);

    d->action_text_strikeout = new KToggleAction(KIcon(QLatin1String("format-text-strikethrough")), i18nc("@action", "&Strike Out"), actionCollection);
    d->htmlEditorActionList.append((d->action_text_strikeout));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_strikeout"), d->action_text_strikeout);
    d->action_text_strikeout->setShortcut(KShortcut(Qt::CTRL + Qt::Key_L));
    FORWARD_ACTION(d->action_text_strikeout, QWebPage::ToggleStrikethrough);

    //Alignment
    d->action_align_left = new KToggleAction(KIcon(QLatin1String("format-justify-left")), i18nc("@action", "Align &Left"), actionCollection);
    d->action_align_left->setIconText(i18nc("@label left justify", "Left"));
    d->htmlEditorActionList.append((d->action_align_left));
    actionCollection->addAction(QLatin1String("htmleditor_format_align_left"), d->action_align_left);
    FORWARD_ACTION(d->action_align_left, QWebPage::AlignLeft);

    d->action_align_center = new KToggleAction(KIcon(QLatin1String("format-justify-center")), i18nc("@action", "Align &Center"), actionCollection);
    d->action_align_center->setIconText(i18nc("@label center justify", "Center"));
    d->htmlEditorActionList.append((d->action_align_center));
    actionCollection->addAction(QLatin1String("htmleditor_format_align_center"), d->action_align_center);
    FORWARD_ACTION(d->action_align_center, QWebPage::AlignCenter);

    d->action_align_right = new KToggleAction(KIcon(QLatin1String("format-justify-right")), i18nc("@action", "Align &Right"), actionCollection);
    d->action_align_right->setIconText(i18nc("@label right justify", "Right"));
    d->htmlEditorActionList.append((d->action_align_right));
    actionCollection->addAction(QLatin1String("htmleditor_format_align_right"), d->action_align_right);
    FORWARD_ACTION(d->action_align_right, QWebPage::AlignRight);

    d->action_align_justify = new KToggleAction(KIcon(QLatin1String("format-justify-fill")), i18nc("@action", "&Justify"), actionCollection);
    d->action_align_justify->setIconText(i18nc("@label justify fill", "Justify"));
    d->htmlEditorActionList.append((d->action_align_justify));
    actionCollection->addAction(QLatin1String("htmleditor_format_align_justify"), d->action_align_justify);
    FORWARD_ACTION(d->action_align_justify, QWebPage::AlignJustified);

    QActionGroup *alignmentGroup = new QActionGroup(this);
    alignmentGroup->addAction(d->action_align_left);
    alignmentGroup->addAction(d->action_align_center);
    alignmentGroup->addAction(d->action_align_right);
    alignmentGroup->addAction(d->action_align_justify);

    //Direction
    d->action_direction_ltr = new KToggleAction(KIcon(QLatin1String("format-text-direction-ltr")), i18nc("@action", "Left-to-Right"), actionCollection);
    d->action_direction_ltr->setIconText(i18nc("@label left-to-right", "Left-to-Right"));
    d->htmlEditorActionList.append(d->action_direction_ltr);
    actionCollection->addAction(QLatin1String("htmleditor_direction_ltr"), d->action_direction_ltr);
    FORWARD_ACTION(d->action_direction_ltr, QWebPage::SetTextDirectionLeftToRight);

    d->action_direction_rtl = new KToggleAction(KIcon(QLatin1String("format-text-direction-rtl")), i18nc("@action", "Right-to-Left"), actionCollection);
    d->action_direction_rtl->setIconText(i18nc("@label right-to-left", "Right-to-Left"));
    d->htmlEditorActionList.append(d->action_direction_rtl);
    actionCollection->addAction(QLatin1String("htmleditor_direction_rtl"), d->action_direction_rtl);
    FORWARD_ACTION(d->action_direction_ltr, QWebPage::SetTextDirectionRightToLeft);

    QActionGroup *directionGroup = new QActionGroup(this);
    directionGroup->addAction(d->action_direction_ltr);
    directionGroup->addAction(d->action_direction_rtl);

    //indent
    d->action_list_indent = new KAction(KIcon(QLatin1String("format-indent-more")), i18nc("@action", "Increase Indent"), actionCollection);
    d->htmlEditorActionList.append((d->action_list_indent));
    actionCollection->addAction(QLatin1String("htmleditor_format_list_indent_more"), d->action_list_indent);
    FORWARD_ACTION(d->action_list_indent, QWebPage::Indent);

    d->action_list_dedent = new KAction(KIcon(QLatin1String("format-indent-less")), i18nc("@action", "Decrease Indent"), actionCollection);
    d->htmlEditorActionList.append(d->action_list_dedent);
    actionCollection->addAction(QLatin1String("htmleditor_format_list_indent_less"), d->action_list_dedent);
    FORWARD_ACTION(d->action_list_dedent, QWebPage::Outdent);

    //horizontal line
    d->action_insert_horizontal_rule = new KAction(KIcon(QLatin1String("insert-horizontal-rule")), i18nc("@action", "Insert Rule Line"), actionCollection);
    d->htmlEditorActionList.append((d->action_insert_horizontal_rule));
    actionCollection->addAction(QLatin1String("htmleditor_insert_horizontal_rule"), d->action_insert_horizontal_rule);
    connect( d->action_insert_horizontal_rule, SIGNAL(triggered(bool)), SLOT(_k_slotInsertHorizontalRule()) );


    //Superscript/subScript
    d->action_text_subscript = new KToggleAction(KIcon(QLatin1String("format-text-subscript")), i18nc("@action", "Subscript"), actionCollection);
    d->htmlEditorActionList.append((d->action_text_subscript));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_subscript"), d->action_text_subscript);
    FORWARD_ACTION(d->action_text_subscript, QWebPage::ToggleSubscript);

    d->action_text_superscript = new KToggleAction(KIcon(QLatin1String("format-text-superscript")), i18nc("@action", "Superscript"), actionCollection);
    d->htmlEditorActionList.append((d->action_text_superscript));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_superscript"), d->action_text_superscript);
    FORWARD_ACTION(d->action_text_superscript, QWebPage::ToggleSuperscript);

    d->action_ordered_list = new KToggleAction(KIcon(QLatin1String("format-list-ordered")), i18n("Ordered Style"), actionCollection);
    d->htmlEditorActionList.append(d->action_ordered_list);
    actionCollection->addAction(QLatin1String("htmleditor_format_list_ordered"), d->action_ordered_list);
    FORWARD_ACTION(d->action_ordered_list, QWebPage::InsertOrderedList);


    d->action_unordered_list = new KToggleAction( KIcon( QLatin1String("format-list-unordered" )), i18n( "Unordered List" ), actionCollection );
    d->htmlEditorActionList.append(d->action_unordered_list);
    actionCollection->addAction(QLatin1String("htmleditor_format_list_unordered"), d->action_unordered_list);
    FORWARD_ACTION(d->action_unordered_list, QWebPage::InsertUnorderedList);



    d->action_format_type = new KSelectAction(KIcon(QLatin1String("format-list-unordered")), i18nc("@title:menu", "List Style"), actionCollection);
    KAction *act = d->action_format_type->addAction(i18n( "Paragraph" ));
    act->setData(QVariant::fromValue(ComposerViewPrivate::Paragraph));
    act = d->action_format_type->addAction(i18n( "Heading 1" ));
    act->setData(QVariant::fromValue(ComposerViewPrivate::Header1));
    act = d->action_format_type->addAction(i18n( "Heading 2" ));
    act->setData(QVariant::fromValue(ComposerViewPrivate::Header2));
    act = d->action_format_type->addAction(i18n( "Heading 3" ));
    act->setData(QVariant::fromValue(ComposerViewPrivate::Header3));
    act = d->action_format_type->addAction(i18n( "Heading 4" ));
    act->setData(QVariant::fromValue(ComposerViewPrivate::Header4));
    act = d->action_format_type->addAction(i18n( "Heading 5" ));
    act->setData(QVariant::fromValue(ComposerViewPrivate::Header5));
    act = d->action_format_type->addAction(i18n( "Heading 6" ));
    act->setData(QVariant::fromValue(ComposerViewPrivate::Header6));
    act = d->action_format_type->addAction(i18n( "Pre" ));
    act->setData(QVariant::fromValue(ComposerViewPrivate::Pre));
    act = d->action_format_type->addAction(i18n( "Address" ));
    act->setData(QVariant::fromValue(ComposerViewPrivate::Address));
    d->action_format_type->setCurrentItem(0);
    d->htmlEditorActionList.append(d->action_format_type);
    actionCollection->addAction(QLatin1String("htmleditor_format_type"), d->action_format_type);
    connect(d->action_format_type, SIGNAL(triggered(QAction*)),
            this, SLOT(_k_setFormatType(QAction*)));

    //BlockQuote
    d->action_block_quote = new KAction(KIcon(QLatin1String("format-text-blockquote")), i18n( "Blockquote" ), this );
    d->htmlEditorActionList.append(d->action_block_quote);
    actionCollection->addAction(QLatin1String("htmleditor_block_quote"), d->action_block_quote);
    connect( d->action_block_quote, SIGNAL(triggered()), this, SLOT(_k_slotToggleBlockQuote()) );


    //Color
    //Foreground Color
    d->action_text_foreground_color = new KAction(KIcon(QLatin1String("format-stroke-color")), i18nc("@action", "Text &Color..."), actionCollection);
    d->action_text_foreground_color->setIconText(i18nc("@label stroke color", "Color"));
    d->htmlEditorActionList.append((d->action_text_foreground_color));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_foreground_color"), d->action_text_foreground_color);
    connect(d->action_text_foreground_color, SIGNAL(triggered()), this, SLOT(_k_setTextForegroundColor()));

    //Background Color
    d->action_text_background_color = new KAction(KIcon(QLatin1String("format-fill-color")), i18nc("@action", "Text &Highlight..."), actionCollection);
    d->htmlEditorActionList.append((d->action_text_background_color));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_background_color"), d->action_text_background_color);
    connect(d->action_text_background_color, SIGNAL(triggered()), this, SLOT(_k_setTextBackgroundColor()));




    d->action_add_emoticon = new KPIMTextEdit::EmoticonTextEditAction(actionCollection);
    actionCollection->addAction(QLatin1String("htmleditor_add_emoticon"), d->action_add_emoticon);
    connect( d->action_add_emoticon, SIGNAL(emoticonActivated(QString)),
             this, SLOT(_k_slotAddEmoticon(QString)) );

    d->action_insert_html = new KAction( i18n( "Insert HTML" ), this );
    actionCollection->addAction( QLatin1String( "htmleditor_insert_html" ), d->action_insert_html );
    connect( d->action_insert_html, SIGNAL(triggered(bool)), SLOT(_k_slotInsertHtml()) );

    d->action_insert_image = new KAction( KIcon( QLatin1String( "insert-image" ) ), i18n( "Add Image" ), actionCollection );
    actionCollection->addAction( QLatin1String( "htmleditor_add_image" ), d->action_insert_image );
    connect( d->action_insert_image, SIGNAL(triggered(bool)), SLOT(_k_slotAddImage()) );

    d->action_format_reset = new KAction( KIcon( QLatin1String("draw-eraser") ), i18n("Reset Font Settings"), actionCollection );
    actionCollection->addAction( QLatin1String("htmleditor_format_reset"), d->action_format_reset );
    FORWARD_ACTION(d->action_format_reset, QWebPage::RemoveFormat);


    //link
    d->action_insert_link = new KAction(KIcon(QLatin1String("insert-link")), i18nc("@action", "Link"), actionCollection);
    d->htmlEditorActionList.append(d->action_insert_link);
    actionCollection->addAction(QLatin1String("htmleditor_insert_link"), d->action_insert_link);
    connect(d->action_insert_link, SIGNAL(triggered(bool)), this, SLOT(_k_insertLink()));

    //Font
    d->action_font_size = new KSelectAction(i18nc("@action", "Font &Size"), actionCollection);
    d->htmlEditorActionList.append(d->action_font_size);
    QStringList sizes;
    sizes << QLatin1String("xx-small");
    sizes << QLatin1String("x-small");
    sizes << QLatin1String("small");
    sizes << QLatin1String("medium");
    sizes << QLatin1String("large");
    sizes << QLatin1String("x-large");
    sizes << QLatin1String("xx-large");
    d->action_font_size->setItems(sizes);
    d->action_font_size->setCurrentItem(0);
    actionCollection->addAction(QLatin1String("htmleditor_format_font_size"), d->action_font_size);
    connect(d->action_font_size, SIGNAL(triggered(int)), this, SLOT(_k_setFontSize(int)));

    d->action_font_family = new KFontAction(i18nc("@action", "&Font"), actionCollection);
    d->htmlEditorActionList.append((d->action_font_family));
    actionCollection->addAction(QLatin1String("htmleditor_format_font_family"), d->action_font_family);
    connect(d->action_font_family, SIGNAL(triggered(QString)), this, SLOT(_k_setFontFamily(QString)));

    //Spell Checking
    d->action_spell_check = new KAction(KIcon(QLatin1String("tools-check-spelling")), i18n("Check Spelling..."), actionCollection);
    d->htmlEditorActionList.append(d->action_spell_check);
    actionCollection->addAction(QLatin1String("htmleditor_spell_check"), d->action_spell_check);
    connect(d->action_spell_check, SIGNAL(triggered(bool)), this, SLOT(_k_slotSpellCheck()));

    //Find
    d->action_find = KStandardAction::find(this, SLOT(_k_slotFind()), actionCollection);
    d->htmlEditorActionList.append(d->action_find);
    actionCollection->addAction(QLatin1String("htmleditor_find"), d->action_find);

    //Replace
    d->action_replace = KStandardAction::replace(this, SLOT(_k_slotReplace()), actionCollection);
    d->htmlEditorActionList.append(d->action_replace);
    actionCollection->addAction(QLatin1String("htmleditor_replace"), d->action_replace);

    //Table
    d->action_insert_table = new KAction( i18n( "Table..." ), this );
    d->htmlEditorActionList.append(d->action_insert_table);
    actionCollection->addAction( QLatin1String( "htmleditor_insert_new_table" ), d->action_insert_table );
    connect( d->action_insert_table, SIGNAL(triggered(bool)), SLOT(_k_slotInsertTable()) );

    //Page Color
    d->action_page_color = new KAction( i18n( "Page Color and Background..." ), this );
    d->htmlEditorActionList.append(d->action_page_color);
    actionCollection->addAction( QLatin1String( "htmleditor_page_color_and_background" ), d->action_page_color );
    connect( d->action_page_color, SIGNAL(triggered(bool)), SLOT(_k_slotChangePageColorAndBackground()) );
}


void ComposerView::contextMenuEvent(QContextMenuEvent* event)
{
    d->contextMenuResult = page()->mainFrame()->hitTestContent(event->pos());

    const bool linkSelected = !d->contextMenuResult.linkElement().isNull();
    const bool imageSelected = !d->contextMenuResult.imageUrl().isEmpty();

    const QWebElement elm = d->contextMenuResult.element();
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
    menu->addSeparator();
    menu->addAction(page()->action(QWebPage::SelectAll));
    menu->addSeparator();
    if(!emptyDocument) {
        menu->addAction(d->action_find);
        menu->addSeparator();
    }
    if(imageSelected) {
        QAction *editImageAction = menu->addAction(i18n("Edit Image..."));
        connect( editImageAction, SIGNAL(triggered(bool)), this, SLOT(_k_slotEditImage()) );
    } else if(linkSelected) {
        QAction *editLinkAction = menu->addAction(i18n("Edit Link..."));
        connect( editLinkAction, SIGNAL(triggered(bool)), this, SLOT(_k_slotEditLink()) );
    }
    menu->addSeparator();
    menu->addAction(d->action_spell_check);
    menu->addSeparator();

    QAction *speakAction = menu->addAction(i18n("Speak Text"));
    speakAction->setIcon(KIcon(QLatin1String("preferences-desktop-text-to-speech")));
    speakAction->setEnabled(!emptyDocument );
    connect( speakAction, SIGNAL(triggered(bool)), this, SLOT(_k_slotSpeakText()) );
    menu->exec(event->globalPos());
    delete menu;
}

void ComposerView::setActionsEnabled(bool enabled)
{
    foreach(QAction* action, d->htmlEditorActionList)
    {
        action->setEnabled(enabled);
    }
}

}

#include "composerview.moc"
