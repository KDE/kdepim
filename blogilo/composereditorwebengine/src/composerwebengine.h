/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

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

#ifndef COMPOSERWEBENGINE_H
#define COMPOSERWEBENGINE_H

#include <QWebEngineView>
#include <WebEngineViewer/WebEngineView>

#include "composereditorwebengine_export.h"
class KActionCollection;
class KToolBar;
namespace WebEngineViewer
{
class WebHitTestResult;
}
namespace ComposerEditorWebEngine
{
class ComposerEditorWebEnginePrivate;
class COMPOSEREDITORWEBENGINE_EXPORT ComposerWebEngine : public WebEngineViewer::WebEngineView
{
    Q_OBJECT
public:
    explicit ComposerWebEngine(QWidget *parent = Q_NULLPTR);
    ~ComposerWebEngine();

    /**
     * @brief initialHtml
     * @return initial html file. Needs to initialize view
     */
    virtual QString initialHtml();
    enum ComposerWebEngineAction {
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

    void createActions(const QList<ComposerWebEngineAction> &);

    void createAllActions();

    void createToolBar(const QList<ComposerWebEngineAction> &lstAction, KToolBar *toolbar);

    void setActionsEnabled(bool enabled);

    void setHtmlContent(const QString &html);

    QAction *action(ComposerWebEngineAction actionType) const;

    /**
     * @brief evaluateJavascript
     * @param javascript evaluate javascript function.
     */
    void evaluateJavascript(const QString &javascript);

    QMap<QString, QString> localImages() const;

protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void showFindBar();
    void openLink(const QUrl &);

private Q_SLOTS:
    void slotWebHitFinished(const WebEngineViewer::WebHitTestResult &result);
private:
    friend class ComposerEditorWebEnginePrivate;
    ComposerEditorWebEnginePrivate *const d;
    Q_PRIVATE_SLOT(d, void _k_slotSpeakText())
    Q_PRIVATE_SLOT(d, void _k_slotAdjustActions())
    Q_PRIVATE_SLOT(d, void _k_setFormatType(QAction *))
    Q_PRIVATE_SLOT(d, void _k_slotAddEmoticon(const QString &))
    Q_PRIVATE_SLOT(d, void _k_slotInsertHtml())
    Q_PRIVATE_SLOT(d, void _k_slotAddImage())
    Q_PRIVATE_SLOT(d, void _k_slotInsertTable())
    Q_PRIVATE_SLOT(d, void _k_setTextForegroundColor())
    Q_PRIVATE_SLOT(d, void _k_setTextBackgroundColor())
    Q_PRIVATE_SLOT(d, void _k_slotInsertHorizontalRule())
    Q_PRIVATE_SLOT(d, void _k_insertLink())
    Q_PRIVATE_SLOT(d, void _k_setFontSize(int))
    Q_PRIVATE_SLOT(d, void _k_setFontFamily(const QString &))
    Q_PRIVATE_SLOT(d, void _k_slotSpellCheck())
    Q_PRIVATE_SLOT(d, void _k_spellCheckerCorrected(const QString &original, int pos, const QString &replacement))
    Q_PRIVATE_SLOT(d, void _k_spellCheckerMisspelling(const QString &, int))
    Q_PRIVATE_SLOT(d, void _k_slotSpellCheckDone(const QString &))
    Q_PRIVATE_SLOT(d, void _k_slotFind())
    Q_PRIVATE_SLOT(d, void _k_slotReplace())
    Q_PRIVATE_SLOT(d, void _k_slotDeleteText())
    Q_PRIVATE_SLOT(d, void _k_slotChangePageColorAndBackground())
    Q_PRIVATE_SLOT(d, void _k_slotEditLink())
    Q_PRIVATE_SLOT(d, void _k_slotOpenLink())
    Q_PRIVATE_SLOT(d, void _k_slotToggleBlockQuote())
    Q_PRIVATE_SLOT(d, void _k_slotEditImage())
    Q_PRIVATE_SLOT(d, void _k_slotSaveAs())
    Q_PRIVATE_SLOT(d, void _k_slotPrint())
    Q_PRIVATE_SLOT(d, void _k_slotPrintPreview())
    Q_PRIVATE_SLOT(d, void _k_changeAutoSpellChecking(bool))
    Q_PRIVATE_SLOT(d, void _k_slotEditList())
    Q_PRIVATE_SLOT(d, void _k_slotPasteWithoutFormatting())
    Q_PRIVATE_SLOT(d, void _k_slotInsertSpecialChar())
    Q_PRIVATE_SLOT(d, void _k_slotInsertAnchor())
    Q_PRIVATE_SLOT(d, void _k_slotBold(bool))
    Q_PRIVATE_SLOT(d, void _k_slotItalic(bool))
    Q_PRIVATE_SLOT(d, void _k_slotUnderline(bool b))
    Q_PRIVATE_SLOT(d, void _k_slotStrikeout(bool))
    Q_PRIVATE_SLOT(d, void _k_slotSuperscript(bool b))
    Q_PRIVATE_SLOT(d, void _k_slotJustifyLeft(bool b))
    Q_PRIVATE_SLOT(d, void _k_slotJustifyCenter(bool b))
    Q_PRIVATE_SLOT(d, void _k_slotJustifyRight(bool b))
    Q_PRIVATE_SLOT(d, void _k_slotJustifyFull(bool b))
    Q_PRIVATE_SLOT(d, void _k_slotSubscript(bool b))
    Q_PRIVATE_SLOT(d, void _k_slotListIndent())
    Q_PRIVATE_SLOT(d, void _k_slotListDedent())
    Q_PRIVATE_SLOT(d, void _k_slotOrderedList(bool b))
    Q_PRIVATE_SLOT(d, void _k_slotUnOrderedList(bool b))
    Q_PRIVATE_SLOT(d, void _k_slotResetFormat())
    Q_PRIVATE_SLOT(d, void _k_slotDirectionLtr())
    Q_PRIVATE_SLOT(d, void _k_slotDirectionRtl())
};
}
#endif // COMPOSERWEBENGINE_H
