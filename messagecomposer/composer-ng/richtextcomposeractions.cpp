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

#include <KToggleAction>
#include <KLocalizedString>

using namespace MessageComposer;

class RichTextComposerActions::RichTextComposerActionsPrivate
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
          action_text_subscript(Q_NULLPTR)
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

void RichTextComposerActions::createActions()
{
    //Alignment
    d->action_align_left = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-left")),
            i18nc("@action", "Align &Left"), this);
    d->action_align_left->setIconText(i18nc("@label left justify", "Left"));
    d->richTextActionList.append((d->action_align_left));
    d->action_align_left->setObjectName(QStringLiteral("format_align_left"));
    connect(d->action_align_left, SIGNAL(triggered()),
            d->composerControler, SLOT(alignLeft()));

    d->action_align_center = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-center")),
            i18nc("@action", "Align &Center"), this);
    d->action_align_center->setIconText(i18nc("@label center justify", "Center"));
    d->richTextActionList.append((d->action_align_center));
    d->action_align_center->setObjectName(QStringLiteral("format_align_center"));
    connect(d->action_align_center, SIGNAL(triggered()),
            this, SLOT(alignCenter()));

    d->action_align_right = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-right")),
            i18nc("@action", "Align &Right"), this);
    d->action_align_right->setIconText(i18nc("@label right justify", "Right"));
    d->richTextActionList.append((d->action_align_right));
    d->action_align_right->setObjectName(QStringLiteral("format_align_right"));
    connect(d->action_align_right, SIGNAL(triggered()),
            this, SLOT(alignRight()));

    d->action_align_justify = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-fill")),
            i18nc("@action", "&Justify"), this);
    d->action_align_justify->setIconText(i18nc("@label justify fill", "Justify"));
    d->richTextActionList.append((d->action_align_justify));
    d->action_align_justify->setObjectName(QStringLiteral("format_align_justify"));
    connect(d->action_align_justify, SIGNAL(triggered()),
            this, SLOT(alignJustify()));

    QActionGroup *alignmentGroup = new QActionGroup(this);
    alignmentGroup->addAction(d->action_align_left);
    alignmentGroup->addAction(d->action_align_center);
    alignmentGroup->addAction(d->action_align_right);
    alignmentGroup->addAction(d->action_align_justify);
}
