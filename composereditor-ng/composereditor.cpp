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

#include <kpimtextedit/emoticontexteditaction.h>
#include <kpimtextedit/inserthtmldialog.h>
#include <kpimtextedit/insertimagedialog.h>

#include <KAction>
#include <KToggleAction>
#include <KLocale>
#include <KSelectAction>
#include <KActionCollection>
#include <KColorDialog>
#include <KMessageBox>
#include <KStandardDirs>
#include <KDebug>
#include <KFontAction>

#include <QWebFrame>
#include <QWebPage>
#include <QDebug>
#include <QPointer>
#include <QFile>


namespace ComposerEditorNG {

#define FORWARD_ACTION(action1, action2) \
    connect(action1, SIGNAL(triggered()), d->getAction(action2), SLOT(trigger()));\
    connect(d->getAction(action2), SIGNAL(changed()), SLOT(_k_slotAdjustActions()));

#define FOLLOW_CHECK(a1, a2) a1->setChecked(getAction(a2)->isChecked())

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

    enum ListType {
        None,
        Disc,
        Circle,
        Square,
        Decimal,
        LowerAlpha,
        UpperAlpha,
        LowerRoman,
        UpperRoman
    };

    void _k_slotAdjustActions();
    void _k_setListStyle(QAction *act);
    void _k_setFormatType(QAction* action);
    void _k_slotAddEmoticon(const QString&);
    void _k_slotInsertHtml();
    void _k_slotAddImage();
    void _k_setTextForegroundColor();
    void _k_setTextBackgroundColor();
    void _k_slotInsertHorizontalRule();
    void _k_insertLink();
    void _k_setFontSize(int);
    void _k_setFontFamily(const QString&);
    void _k_adjustActions();

    QAction* getAction ( QWebPage::WebAction action ) const;
    void execCommand(const QString &cmd);
    void execCommand(const QString &cmd, const QString &arg);
    bool queryCommandState(const QString &cmd);

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
    KSelectAction *action_font_size;
    KFontAction *action_font_family;
    KPIMTextEdit::EmoticonTextEditAction *action_add_emoticon;
    KAction *action_insert_html;
    KAction *action_insert_image;
    KAction *action_text_foreground_color;
    KAction *action_text_background_color;
    KAction *action_format_reset;
    KAction *action_insert_link;
    bool richTextEnabled;
};
}
Q_DECLARE_METATYPE(ComposerEditorNG::ComposerEditorPrivate::FormatType)
Q_DECLARE_METATYPE(ComposerEditorNG::ComposerEditorPrivate::ListType)

namespace ComposerEditorNG {
QAction* ComposerEditorPrivate::getAction ( QWebPage::WebAction action ) const
{
    if ( action >= 0 && action <= 66 )
        return q->page()->action( static_cast<QWebPage::WebAction>( action ));
    else
        return 0;
}

void ComposerEditorPrivate::_k_setListStyle(QAction *act)
{
    if(!act) {
        return;
    }
    QString command;
    switch(act->data().value<ComposerEditorNG::ComposerEditorPrivate::ListType>())
    {
    case None:
        break;
    case Disc:
        break;
    case Circle:
        break;
    case Square:
        break;
    case Decimal:
        break;
    case LowerAlpha:
        break;
    case UpperAlpha:
        break;
    case LowerRoman:
        break;
    case UpperRoman:
    break;

    }
    //execCommand(QLatin1String("insertOrderedList"), QLatin1String("newsL"));
    execCommand(QLatin1String("insertHTML"), QLatin1String("<li> </li>"));
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
    execCommand ( QLatin1String("formatBlock"), command );
}

void ComposerEditorPrivate::_k_slotAddEmoticon(const QString& emoticon)
{
    execCommand(QLatin1String("insertHTML"), emoticon);
}

void ComposerEditorPrivate::_k_slotInsertHtml()
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

void ComposerEditorPrivate::_k_setTextBackgroundColor()
{
    //TODO
#if 0
    const int result = KColorDialog::getColor(currentTextBackgroundColor, KColorScheme(QPalette::Active, KColorScheme::View).foreground().color() , q);
    if (!currentTextBackgroundColor.isValid())
        currentTextBackgroundColor = KColorScheme(QPalette::Active, KColorScheme::View).foreground().color() ;
    if (result != QDialog::Accepted)
        return;

    q->setTextBackgroundColor(currentTextBackgroundColor);
    execCommand("hiliteColor", color.name());
#endif
}

void ComposerEditorPrivate::_k_setTextForegroundColor()
{
#if 0
    //TODO
    execCommand("foreColor", color.name());
#endif
}

void ComposerEditorPrivate::_k_adjustActions()
{
    //TODO
}

void ComposerEditorPrivate::_k_slotAddImage()
{
    QPointer<KPIMTextEdit::InsertImageDialog> dlg = new KPIMTextEdit::InsertImageDialog( q );
    if ( dlg->exec() == KDialog::Accepted ) {
        const KUrl url = dlg->imageUrl();
        int imageWidth = -1;
        int imageHeight = -1;
        if ( !dlg->keepOriginalSize() ) {
            imageWidth = dlg->imageWidth();
            imageHeight = dlg->imageHeight();
        }
        QString imageHtml = QString::fromLatin1("<img %1 %2 %3 />").arg((imageWidth>0) ? QString::fromLatin1("width=%1").arg(imageWidth) : QString())
                .arg((imageHeight>0) ? QString::fromLatin1("height=%1").arg(imageHeight) : QString())
                .arg(url.isEmpty() ? QString() : QString::fromLatin1("src='file://%1'").arg(url.path()));
        qDebug()<<" imageHtml"<<imageHtml;
        execCommand(QLatin1String("insertHTML"), imageHtml);
    }
    delete dlg;
}

void ComposerEditorPrivate::_k_slotInsertHorizontalRule()
{
    execCommand(QLatin1String("insertHTML"), QLatin1String("<hr>"));
}

void ComposerEditorPrivate::_k_insertLink()
{
    //TODO
}

void ComposerEditorPrivate::_k_setFontSize(int fontSize)
{
    execCommand(QLatin1String("fontSize"), QString::number(fontSize+1)); //Verify
}

void ComposerEditorPrivate::_k_setFontFamily(const QString& family)
{
    execCommand(QLatin1String("fontName"), family);
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


bool ComposerEditorPrivate::queryCommandState(const QString &cmd)
{
    QWebFrame *frame = q->page()->mainFrame();
    QString js = QString::fromLatin1("document.queryCommandState(\"%1\", false, null)").arg(cmd);
    const QVariant result = frame->evaluateJavaScript(js);
    return result.toString().simplified().toLower() == QLatin1String("true");
}


ComposerEditor::ComposerEditor(QWidget *parent)
    : KWebView(parent), d(new ComposerEditorPrivate(this))
{
    QFile file ( KStandardDirs::locate ( "data", QLatin1String("composereditor/composereditorinitialhtml") ) );
    kDebug() <<file.fileName();
    if ( !file.open ( QIODevice::ReadOnly ) )
        KMessageBox::error(this, i18n ( "Cannot open template file." ), i18n ( "composer editor" ));
    else
        setContent ( file.readAll());//, "application/xhtml+xml" );

    page()->setContentEditable(true);
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect( page(), SIGNAL (selectionChanged()), this, SLOT(_k_adjustActions()) );
}

ComposerEditor::~ComposerEditor()
{
    QString content = page()->mainFrame()->toHtml();
    qDebug()<<"content "<<content;
    delete d;
}

void ComposerEditor::createActions(KActionCollection *actionCollection)
{
    Q_ASSERT(actionCollection);

    //format
    d->action_text_bold = new KToggleAction(KIcon(QLatin1String("format-text-bold")), i18nc("@action boldify selected text", "&Bold"), actionCollection);
    QFont bold;
    bold.setBold(true);
    d->action_text_bold->setFont(bold);
    d->richTextActionList.append((d->action_text_bold));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_bold"), d->action_text_bold);
    d->action_text_bold->setShortcut(KShortcut(Qt::CTRL + Qt::Key_B));
    FORWARD_ACTION(d->action_text_bold, QWebPage::ToggleBold);

    d->action_text_italic = new KToggleAction(KIcon(QLatin1String("format-text-italic")), i18nc("@action italicize selected text", "&Italic"), actionCollection);
    QFont italic;
    italic.setItalic(true);
    d->action_text_italic->setFont(italic);
    d->richTextActionList.append((d->action_text_italic));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_italic"), d->action_text_italic);
    d->action_text_italic->setShortcut(KShortcut(Qt::CTRL + Qt::Key_I));
    FORWARD_ACTION(d->action_text_italic, QWebPage::ToggleItalic);

    d->action_text_underline = new KToggleAction(KIcon(QLatin1String("format-text-underline")), i18nc("@action underline selected text", "&Underline"), actionCollection);
    QFont underline;
    underline.setUnderline(true);
    d->action_text_underline->setFont(underline);
    d->richTextActionList.append((d->action_text_underline));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_underline"), d->action_text_underline);
    d->action_text_underline->setShortcut(KShortcut(Qt::CTRL + Qt::Key_U));
    FORWARD_ACTION(d->action_text_underline, QWebPage::ToggleUnderline);

    d->action_text_strikeout = new KToggleAction(KIcon(QLatin1String("format-text-strikethrough")), i18nc("@action", "&Strike Out"), actionCollection);
    d->richTextActionList.append((d->action_text_strikeout));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_strikeout"), d->action_text_strikeout);
    d->action_text_strikeout->setShortcut(KShortcut(Qt::CTRL + Qt::Key_L));
    FORWARD_ACTION(d->action_text_strikeout, QWebPage::ToggleStrikethrough);

    //Alignment
    d->action_align_left = new KToggleAction(KIcon(QLatin1String("format-justify-left")), i18nc("@action", "Align &Left"), actionCollection);
    d->action_align_left->setIconText(i18nc("@label left justify", "Left"));
    d->richTextActionList.append((d->action_align_left));
    actionCollection->addAction(QLatin1String("htmleditor_format_align_left"), d->action_align_left);
    FORWARD_ACTION(d->action_align_left, QWebPage::AlignLeft);

    d->action_align_center = new KToggleAction(KIcon(QLatin1String("format-justify-center")), i18nc("@action", "Align &Center"), actionCollection);
    d->action_align_center->setIconText(i18nc("@label center justify", "Center"));
    d->richTextActionList.append((d->action_align_center));
    actionCollection->addAction(QLatin1String("htmleditor_format_align_center"), d->action_align_center);
    FORWARD_ACTION(d->action_align_center, QWebPage::AlignCenter);

    d->action_align_right = new KToggleAction(KIcon(QLatin1String("format-justify-right")), i18nc("@action", "Align &Right"), actionCollection);
    d->action_align_right->setIconText(i18nc("@label right justify", "Right"));
    d->richTextActionList.append((d->action_align_right));
    actionCollection->addAction(QLatin1String("htmleditor_format_align_right"), d->action_align_right);
    FORWARD_ACTION(d->action_align_right, QWebPage::AlignRight);

    d->action_align_justify = new KToggleAction(KIcon(QLatin1String("format-justify-fill")), i18nc("@action", "&Justify"), actionCollection);
    d->action_align_justify->setIconText(i18nc("@label justify fill", "Justify"));
    d->richTextActionList.append((d->action_align_justify));
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
    d->richTextActionList.append(d->action_direction_ltr);
    actionCollection->addAction(QLatin1String("htmleditor_direction_ltr"), d->action_direction_ltr);
    FORWARD_ACTION(d->action_direction_ltr, QWebPage::SetTextDirectionLeftToRight);

    d->action_direction_rtl = new KToggleAction(KIcon(QLatin1String("format-text-direction-rtl")), i18nc("@action", "Right-to-Left"), actionCollection);
    d->action_direction_rtl->setIconText(i18nc("@label right-to-left", "Right-to-Left"));
    d->richTextActionList.append(d->action_direction_rtl);
    actionCollection->addAction(QLatin1String("htmleditor_direction_rtl"), d->action_direction_rtl);
    FORWARD_ACTION(d->action_direction_ltr, QWebPage::SetTextDirectionRightToLeft);

    QActionGroup *directionGroup = new QActionGroup(this);
    directionGroup->addAction(d->action_direction_ltr);
    directionGroup->addAction(d->action_direction_rtl);


    //indent
    d->action_list_indent = new KAction(KIcon(QLatin1String("format-indent-more")), i18nc("@action", "Increase Indent"), actionCollection);
    d->richTextActionList.append((d->action_list_indent));
    actionCollection->addAction(QLatin1String("htmleditor_format_list_indent_more"), d->action_list_indent);
    FORWARD_ACTION(d->action_list_indent, QWebPage::Indent);

    d->action_list_dedent = new KAction(KIcon(QLatin1String("format-indent-less")), i18nc("@action", "Decrease Indent"), actionCollection);
    d->richTextActionList.append(d->action_list_dedent);
    actionCollection->addAction(QLatin1String("htmleditor_format_list_indent_less"), d->action_list_dedent);
    FORWARD_ACTION(d->action_list_dedent, QWebPage::Outdent);

    //horizontal line
    d->action_insert_horizontal_rule = new KAction(KIcon(QLatin1String("insert-horizontal-rule")), i18nc("@action", "Insert Rule Line"), actionCollection);
    d->richTextActionList.append((d->action_insert_horizontal_rule));
    actionCollection->addAction(QLatin1String("htmleditor_insert_horizontal_rule"), d->action_insert_horizontal_rule);
    connect( d->action_insert_horizontal_rule, SIGNAL(triggered(bool)), SLOT(_k_slotInsertHorizontalRule()) );


    //Superscript/subScript
    d->action_text_subscript = new KToggleAction(KIcon(QLatin1String("format-text-subscript")), i18nc("@action", "Subscript"), actionCollection);
    d->richTextActionList.append((d->action_text_subscript));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_subscript"), d->action_text_subscript);
    FORWARD_ACTION(d->action_text_subscript, QWebPage::ToggleSubscript);

    d->action_text_superscript = new KToggleAction(KIcon(QLatin1String("format-text-superscript")), i18nc("@action", "Superscript"), actionCollection);
    d->richTextActionList.append((d->action_text_superscript));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_superscript"), d->action_text_superscript);
    FORWARD_ACTION(d->action_text_superscript, QWebPage::ToggleSuperscript);

    d->action_list_style = new KSelectAction(KIcon(QLatin1String("format-list-unordered")), i18nc("@title:menu", "List Style"), actionCollection);
    KAction *act = d->action_list_style->addAction(i18nc("@item:inmenu no list style", "None"));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::None));
    act = d->action_list_style->addAction(i18nc("@item:inmenu disc list style", "Disc"));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::Disc));
    act = d->action_list_style->addAction(i18nc("@item:inmenu circle list style", "Circle"));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::Circle));
    act = d->action_list_style->addAction(i18nc("@item:inmenu square list style", "Square"));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::Square));
    act = d->action_list_style->addAction(i18nc("@item:inmenu numbered lists", "123"));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::Decimal));
    act = d->action_list_style->addAction(i18nc("@item:inmenu lowercase abc lists", "abc"));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::LowerAlpha));
    act = d->action_list_style->addAction(i18nc("@item:inmenu uppercase abc lists", "ABC"));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::UpperAlpha));
    act = d->action_list_style->addAction(i18nc("@item:inmenu lower case roman numerals", "i ii iii"));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::LowerRoman));
    act = d->action_list_style->addAction(i18nc("@item:inmenu upper case roman numerals", "I II III"));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::UpperRoman));
    d->action_list_style->setCurrentItem(0);
    d->richTextActionList.append((d->action_list_style));
    actionCollection->addAction(QLatin1String("htmleditor_format_list_style"), d->action_list_style);
    connect(d->action_list_style, SIGNAL(triggered(QAction*)),
            this, SLOT(_k_setListStyle(QAction*)));

    d->action_format_type = new KSelectAction(KIcon(QLatin1String("format-list-unordered")), i18nc("@title:menu", "List Style"), actionCollection);
    act = d->action_format_type->addAction(i18n( "Paragraph" ));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::Paragraph));
    act = d->action_format_type->addAction(i18n( "Heading 1" ));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::Header1));
    act = d->action_format_type->addAction(i18n( "Heading 2" ));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::Header2));
    act = d->action_format_type->addAction(i18n( "Heading 3" ));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::Header3));
    act = d->action_format_type->addAction(i18n( "Heading 4" ));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::Header4));
    act = d->action_format_type->addAction(i18n( "Heading 5" ));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::Header5));
    act = d->action_format_type->addAction(i18n( "Heading 6" ));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::Header6));
    act = d->action_format_type->addAction(i18n( "Pre" ));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::Pre));
    act = d->action_format_type->addAction(i18n( "Address" ));
    act->setData(QVariant::fromValue(ComposerEditorPrivate::Address));
    d->action_format_type->setCurrentItem(0);
    d->richTextActionList.append(d->action_format_type);
    actionCollection->addAction(QLatin1String("htmleditor_format_type"), d->action_format_type);
    connect(d->action_format_type, SIGNAL(triggered(QAction*)),
            this, SLOT(_k_setFormatType(QAction*)));

    //Color
    //Foreground Color
    d->action_text_foreground_color = new KAction(KIcon(QLatin1String("format-stroke-color")), i18nc("@action", "Text &Color..."), actionCollection);
    d->action_text_foreground_color->setIconText(i18nc("@label stroke color", "Color"));
    d->richTextActionList.append((d->action_text_foreground_color));
    actionCollection->addAction(QLatin1String("htmleditor_format_text_foreground_color"), d->action_text_foreground_color);
    connect(d->action_text_foreground_color, SIGNAL(triggered()), this, SLOT(_k_setTextForegroundColor()));

    //Background Color
    d->action_text_background_color = new KAction(KIcon(QLatin1String("format-fill-color")), i18nc("@action", "Text &Highlight..."), actionCollection);
    d->richTextActionList.append((d->action_text_background_color));
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
    d->richTextActionList.append(d->action_insert_link);
    actionCollection->addAction(QLatin1String("htmleditor_insert_link"), d->action_insert_link);
    connect(d->action_insert_link, SIGNAL(triggered(bool)), this, SLOT(_k_insertLink()));

    //Font
    d->action_font_size = new KSelectAction(i18nc("@action", "Font &Size"), actionCollection);
    d->richTextActionList.append(d->action_font_size);
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
    d->richTextActionList.append((d->action_font_family));
    actionCollection->addAction(QLatin1String("htmleditor_format_font_family"), d->action_font_family);
    connect(d->action_font_family, SIGNAL(triggered(QString)), this, SLOT(_k_setFontFamily(QString)));

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

void ComposerEditor::contextMenuEvent(QContextMenuEvent* event)
{
    //TODO
    KWebView::contextMenuEvent(event);
}

}

#include "composereditor.moc"
