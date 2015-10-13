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

#include "richtextcomposeractionstest.h"
#include "../richtextcomposercontroler.h"
#include "../richtextcomposeractions.h"
#include "../richtextcomposer.h"
#include <KActionCollection>
#include <QAction>
#include <qtest.h>

RichTextComposerActionsTest::RichTextComposerActionsTest(QObject *parent)
    : QObject(parent)
{

}

RichTextComposerActionsTest::~RichTextComposerActionsTest()
{

}

void RichTextComposerActionsTest::shouldHaveDefaultValue()
{
    MessageComposer::RichTextComposer composer;
    MessageComposer::RichTextComposerControler controler(&composer);
    MessageComposer::RichTextComposerActions composerActions(&controler);

    KActionCollection *actionCollection = new KActionCollection(&composerActions);
    QVERIFY(actionCollection->actions().isEmpty());

    composerActions.createActions(actionCollection);

    QVERIFY(!actionCollection->actions().isEmpty());
    QCOMPARE(composerActions.numberOfActions(), actionCollection->actions().count() - 4);
}

void RichTextComposerActionsTest::shouldHaveActions()
{
    MessageComposer::RichTextComposer composer;
    MessageComposer::RichTextComposerControler controler(&composer);
    MessageComposer::RichTextComposerActions composerActions(&controler);

    KActionCollection *actionCollection = new KActionCollection(&composerActions);
    composerActions.createActions(actionCollection);

    QStringList lst;
    lst << QStringLiteral("format_align_left")
        << QStringLiteral("format_align_center")
        << QStringLiteral("format_align_right")
        << QStringLiteral("format_align_justify")
        << QStringLiteral("direction_ltr")
        << QStringLiteral("direction_rtl")
        << QStringLiteral("format_text_subscript")
        << QStringLiteral("format_text_superscript")
        << QStringLiteral("format_text_bold")
        << QStringLiteral("format_text_italic")
        << QStringLiteral("format_text_underline")
        << QStringLiteral("format_text_strikeout")
        << QStringLiteral("format_font_family")
        << QStringLiteral("format_font_size")
        << QStringLiteral("insert_horizontal_rule")
        << QStringLiteral("format_text_foreground_color")
        << QStringLiteral("format_text_background_color")
        << QStringLiteral("manage_link")
        << QStringLiteral("format_list_indent_less")
        << QStringLiteral("format_list_indent_more")
        << QStringLiteral("format_list_style")
        << QStringLiteral("add_image")
        << QStringLiteral("add_emoticon")
        << QStringLiteral("insert_html")
        << QStringLiteral("insert_table")
        << QStringLiteral("delete_line")
        << QStringLiteral("format_reset")
        << QStringLiteral("format_painter");

    QStringList actionNoRichText;
    actionNoRichText << QStringLiteral("paste_quoted")
                     << QStringLiteral("tools_quote")
                     << QStringLiteral("tools_unquote")
                     << QStringLiteral("paste_without_formatting");

    QCOMPARE(lst.count(), composerActions.numberOfActions());
    Q_FOREACH (QAction *act, actionCollection->actions()) {
        const QString actionName = act->objectName();
        if (!actionNoRichText.contains(actionName)) {
            QVERIFY(lst.contains(actionName));
        }
    }
}

void RichTextComposerActionsTest::shouldChangeEnableState()
{
    MessageComposer::RichTextComposer composer;
    MessageComposer::RichTextComposerControler controler(&composer);
    MessageComposer::RichTextComposerActions composerActions(&controler);

    KActionCollection *actionCollection = new KActionCollection(&composerActions);
    composerActions.createActions(actionCollection);

    QStringList actionNoRichText;
    actionNoRichText << QStringLiteral("paste_quoted")
                     << QStringLiteral("tools_quote")
                     << QStringLiteral("tools_unquote")
                     << QStringLiteral("paste_without_formatting");

    composerActions.setActionsEnabled(false);
    Q_FOREACH (QAction *act, actionCollection->actions()) {
        if (!actionNoRichText.contains(act->objectName())) {
            QVERIFY(!act->isEnabled());
        }
    }
    composerActions.setActionsEnabled(true);
    Q_FOREACH (QAction *act, actionCollection->actions()) {
        if (!actionNoRichText.contains(act->objectName())) {
            QVERIFY(act->isEnabled());
        }
    }
}

QTEST_MAIN(RichTextComposerActionsTest)
