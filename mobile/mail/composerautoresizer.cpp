/*
    Copyright (c) 2010 Anselmo Lacerda S. de Melo <anselmolsm@gmail.com>
    Copyright (c) 2010 Artur Duque de Souza <asouza@kde.org>

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

#include "composerautoresizer.h"

#ifdef Q_MODULE_MAEMO5
#include <QtGui/qabstractkineticscroller.h>
#endif

#include <QDebug>

ComposerAutoResizer::ComposerAutoResizer(QWidget *parent)
    : QObject(parent),
      composer(qobject_cast<QTextEdit *>(parent)),
      edit(qobject_cast<QFrame *>(parent))
{
    Q_ASSERT(composer);
    // detect when the text changes
    connect(parent, SIGNAL(textChanged()), this, SLOT(textEditChanged()));
    connect(parent, SIGNAL(cursorPositionChanged()), this, SLOT(textEditChanged()));
}

void ComposerAutoResizer::textEditChanged()
{
    QTextDocument *doc = composer->document();
    const QSize s = doc->size().toSize();
    const QRect fr = edit->frameRect();
    const QRect cr = edit->contentsRect();

    edit->setMinimumHeight(qMax(60, s.height() + (fr.height() - cr.height() - 1)));

#ifdef Q_MODULE_MAEMO5
    QRect cursor = composer->cursorRect();

    // make sure the cursor is visible so the user doesn't loose track of the kb focus
    QPoint pos = edit->pos();
    QWidget *pw = edit->parentWidget();
    while (pw) {
        if (pw->parentWidget()) {
            if (QAbstractScrollArea *area =
                qobject_cast<QAbstractScrollArea *>(pw->parentWidget()->parentWidget())) {
                if (QAbstractKineticScroller * scroller=
                    area->property("kineticScroller").value<QAbstractKineticScroller *>()) {
                    scroller->ensureVisible(pos + cursor.center(), 10 + cursor.width(),
                                            2 * cursor.height());
                }
                break;
             }
        }
        pos = pw->mapToParent(pos);
        pw = pw->parentWidget();
    }
#endif

 }
