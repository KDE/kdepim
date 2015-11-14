/* Copyright (C) 2011-2015 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KSIEVE_KSIEVEUI_SIEVETEXTEDIT_H
#define KSIEVE_KSIEVEUI_SIEVETEXTEDIT_H

#include "ksieveui_export.h"

#include "kpimtextedit/plaintexteditor.h"

#include "PimCommon/SieveSyntaxHighlighterRules"

class QMenu;
namespace PimCommon
{
class SieveSyntaxHighlighterRules;

}
namespace KSieveUi
{

class SieveTextEditPrivate;
class KSIEVEUI_EXPORT SieveTextEdit : public KPIMTextEdit::PlainTextEditor
{
    Q_OBJECT

public:
    explicit SieveTextEdit(QWidget *parent = Q_NULLPTR);
    ~SieveTextEdit();

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;

    void setSieveCapabilities(const QStringList &capabilities);

    void setShowHelpMenu(bool b);

    void uncomment();
    void comment();
    void upperCase();
    void lowerCase();
    void sentenceCase();
    void reverseCase();
    void wordWrap(bool state);
    bool isWordWrap() const;
private Q_SLOTS:
    void slotUpdateLineNumberAreaWidth(int newBlockCount);
    void slotUpdateLineNumberArea(const QRect &, int);
    void slotHelp();

protected:
    void initCompleter();
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void addExtraMenuEntry(QMenu *menu, QPoint pos) Q_DECL_OVERRIDE;
    bool event(QEvent *ev) Q_DECL_OVERRIDE;
    Sonnet::SpellCheckDecorator *createSpellCheckDecorator() Q_DECL_OVERRIDE;

    void updateHighLighter() Q_DECL_OVERRIDE;
    void clearDecorator() Q_DECL_OVERRIDE;
    void createHighlighter() Q_DECL_OVERRIDE;
Q_SIGNALS:
    void openHelp(const QUrl &url);

private:
    bool openVariableHelp();
    bool overrideShortcut(QKeyEvent *event);
    QStringList completerList() const;
    void setCompleterList(const QStringList &list);
    QString selectedWord(const QPoint &pos = QPoint()) const;

    SieveTextEditPrivate *const d;
};

}
#endif

