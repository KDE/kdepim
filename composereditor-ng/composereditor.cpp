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

#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebPage>



namespace ComposerEditorNG {

#define FORWARD_ACTION(action1, action2) \
    connect(action1, SIGNAL(triggered()), getAction(action2), SLOT(trigger()));\
    connect(getAction(action2), SIGNAL(changed()), SLOT(_k_slotAdjustActions()));

class ComposerEditorPrivate
{
public:
    ComposerEditorPrivate(ComposerEditor *qq)
        : q(qq),
          richTextEnabled(true)
    {

    }

    void _k_slotAdjustActions();
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
    bool richTextEnabled;
};

void ComposerEditorPrivate::_k_slotAdjustActions()
{
    //TODO
}

ComposerEditor::ComposerEditor(QWidget *parent)
    : KWebView(parent), d(new ComposerEditorPrivate)
{
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
