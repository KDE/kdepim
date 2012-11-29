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

#ifndef COMPOSERVIEW_H
#define COMPOSERVIEW_H

#include <KWebView>
class KActionCollection;

namespace ComposerEditorNG
{
class ComposerViewPrivate;

class ComposerView : public KWebView
{
    Q_OBJECT
public:
    explicit ComposerView(QWidget * parent = 0);
    ~ComposerView();

    void createActions(KActionCollection *actionCollection);

    void setActionsEnabled(bool enabled);

protected:
    void contextMenuEvent(QContextMenuEvent* event);

Q_SIGNALS:
    void showFindBar();

private:
    friend class ComposerViewPrivate;
    ComposerViewPrivate * const d;
    Q_PRIVATE_SLOT(d, void _k_slotSpeakText())
    Q_PRIVATE_SLOT( d, void _k_slotAdjustActions() )
    Q_PRIVATE_SLOT( d, void _k_setFormatType(QAction *) )
    Q_PRIVATE_SLOT( d, void _k_slotAddEmoticon(const QString&) )
    Q_PRIVATE_SLOT( d, void _k_slotInsertHtml() )
    Q_PRIVATE_SLOT( d, void _k_slotAddImage() )
    Q_PRIVATE_SLOT( d, void _k_slotInsertTable() )
    Q_PRIVATE_SLOT( d, void _k_setTextForegroundColor() )
    Q_PRIVATE_SLOT( d, void _k_setTextBackgroundColor() )
    Q_PRIVATE_SLOT( d, void _k_slotInsertHorizontalRule() )
    Q_PRIVATE_SLOT( d, void _k_insertLink() )
    Q_PRIVATE_SLOT( d, void _k_setFontSize(int) )
    Q_PRIVATE_SLOT( d, void _k_setFontFamily(const QString&) )
    Q_PRIVATE_SLOT( d, void _k_adjustActions() )
    Q_PRIVATE_SLOT( d, void _k_slotSpellCheck() )
    Q_PRIVATE_SLOT( d, void _k_spellCheckerCorrected(const QString& original, int pos, const QString& replacement) )
    Q_PRIVATE_SLOT( d, void _k_spellCheckerMisspelling(const QString&, int) )
    Q_PRIVATE_SLOT( d, void _k_slotSpellCheckDone(const QString&) )
    Q_PRIVATE_SLOT( d, void _k_slotFind() )
    Q_PRIVATE_SLOT( d, void _k_slotReplace() )
    Q_PRIVATE_SLOT( d, void _k_slotDeleteText() )

};
}

#endif // COMPOSERVIEW_H
