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

#ifndef COMPOSEREDITOR_H
#define COMPOSEREDITOR_H

#include "composereditor_export.h"

#include <KWebView>

class KActionCollection;

namespace ComposerEditorNG
{
class ComposerEditorPrivate;

class COMPOSEREDITORNG_EXPORT ComposerEditor : public KWebView
{
    Q_OBJECT
public:
    explicit ComposerEditor(QWidget *parent);
    ~ComposerEditor();

    virtual void createActions(KActionCollection *actionCollection);

    QString plainTextContent() const;

    bool enableRichText() const;

public Q_SLOTS:
    void setEnableRichText(bool richTextEnabled);

private:
    friend class ComposerEditorPrivate;
    ComposerEditorPrivate * const d;
    Q_PRIVATE_SLOT( d, void _k_slotAdjustActions() )
};
}

#endif // COMPOSEREDITOR_H
