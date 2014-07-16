/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#ifndef COMPOSERVIEW_H
#define COMPOSERVIEW_H
#include "composereditor_export.h"
#include <KWebView>

class KActionCollection;
class KToolBar;

namespace ComposerEditorNG
{
class ComposerViewPrivate;

class COMPOSEREDITORNG_EXPORT ComposerView : public KWebView
{
    Q_OBJECT
public:
    explicit ComposerView(QWidget *parent = 0);
    ~ComposerView();

    /**
     * @brief initialHtml
     * @return initial html file. Needs to initialize view
     */
    virtual QString initialHtml();

    enum ComposerViewAction {
        //Separator
        Separator,
        //Real Actions
        Bold,
        Italic,
        Underline,
        StrikeOut,
        AlignLeft,
        AlignCenter,
        AlignRight,
        AlignJustify,
        DirectionLtr,
        DirectionRtl,
        SubScript,
        SuperScript,
        HorizontalRule,
        ListIndent,
        ListDedent,
        OrderedList,
        UnorderedList,
        FormatType,
        FontSize,
        FontFamily,
        Emoticon,
        InsertHtml,
        InsertImage,
        InsertTable,
        InsertLink,
        InsertSpecialChar,
        TextForegroundColor,
        TextBackgroundColor,
        FormatReset,
        SpellCheck,
        Find,
        Replace,
        PageColor,
        BlockQuote,
        SaveAs,
        Print,
        PrintPreview,
        PasteWithoutFormatting,
        InsertAnchor,

        //Keep at end
        LastType
    };

    virtual void addCreatedActionsToActionCollection(KActionCollection *actionCollection);
    virtual void addExtraAction(QMenu *menu);

    void createActions(const QList<ComposerViewAction> &);

    void createAllActions();

    void createToolBar(const QList<ComposerViewAction> &lstAction, KToolBar *toolbar);

    void setActionsEnabled(bool enabled);

    void setHtmlContent( const QString &html );

    QAction *action(ComposerViewAction actionType) const;

    /**
     * @brief evaluateJavascript
     * @param javascript evaluate javascript function.
     */
    void evaluateJavascript( const QString &javascript);

    QMap<QString, QString> localImages() const;

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *);

Q_SIGNALS:
    void showFindBar();
    void openLink(const QUrl&);

private:
    friend class ComposerViewPrivate;
    ComposerViewPrivate * const d;
    Q_PRIVATE_SLOT( d, void _k_slotSpeakText() )
    Q_PRIVATE_SLOT( d, void _k_slotAdjustActions() )
    Q_PRIVATE_SLOT( d, void _k_setFormatType(QAction *) )
    Q_PRIVATE_SLOT( d, void _k_slotAddEmoticon(const QString &) )
    Q_PRIVATE_SLOT( d, void _k_slotInsertHtml() )
    Q_PRIVATE_SLOT( d, void _k_slotAddImage() )
    Q_PRIVATE_SLOT( d, void _k_slotInsertTable() )
    Q_PRIVATE_SLOT( d, void _k_setTextForegroundColor() )
    Q_PRIVATE_SLOT( d, void _k_setTextBackgroundColor() )
    Q_PRIVATE_SLOT( d, void _k_slotInsertHorizontalRule() )
    Q_PRIVATE_SLOT( d, void _k_insertLink() )
    Q_PRIVATE_SLOT( d, void _k_setFontSize(int) )
    Q_PRIVATE_SLOT( d, void _k_setFontFamily(const QString &) )
    Q_PRIVATE_SLOT( d, void _k_slotSpellCheck() )
    Q_PRIVATE_SLOT( d, void _k_spellCheckerCorrected(const QString &original, int pos, const QString &replacement) )
    Q_PRIVATE_SLOT( d, void _k_spellCheckerMisspelling(const QString &, int) )
    Q_PRIVATE_SLOT( d, void _k_slotSpellCheckDone(const QString &) )
    Q_PRIVATE_SLOT( d, void _k_slotFind() )
    Q_PRIVATE_SLOT( d, void _k_slotReplace() )
    Q_PRIVATE_SLOT( d, void _k_slotDeleteText() )
    Q_PRIVATE_SLOT( d, void _k_slotChangePageColorAndBackground() )
    Q_PRIVATE_SLOT( d, void _k_slotEditLink() )
    Q_PRIVATE_SLOT( d, void _k_slotOpenLink() )
    Q_PRIVATE_SLOT( d, void _k_slotToggleBlockQuote() )
    Q_PRIVATE_SLOT( d, void _k_slotEditImage() )
    Q_PRIVATE_SLOT( d, void _k_slotSaveAs() )
    Q_PRIVATE_SLOT( d, void _k_slotPrint() )
    Q_PRIVATE_SLOT( d, void _k_slotPrintPreview() )
    Q_PRIVATE_SLOT( d, void _k_changeAutoSpellChecking(bool) )
    Q_PRIVATE_SLOT( d, void _k_slotEditList() )
    Q_PRIVATE_SLOT( d, void _k_slotPasteWithoutFormatting() )
    Q_PRIVATE_SLOT( d, void _k_slotInsertSpecialChar() )
};
}

#endif // COMPOSERVIEW_H
