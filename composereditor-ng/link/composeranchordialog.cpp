/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "composeranchordialog.h"
namespace ComposerEditorNG {

class ComposerAnchorDialogPrivate
{
public:
    ComposerAnchorDialogPrivate(ComposerAnchorDialog *qq)
        : q(qq)
    {

    }

    QString html() const
    {
        //TODO implement it.
        return QString();
    }
    ComposerAnchorDialog *q;
};

ComposerAnchorDialog::ComposerAnchorDialog(QWidget *parent)
    : KDialog(parent), d(new ComposerAnchorDialogPrivate(this))
{
}

ComposerAnchorDialog::~ComposerAnchorDialog()
{
    delete d;
}

QString ComposerAnchorDialog::html() const
{
    return d->html();
}


}
