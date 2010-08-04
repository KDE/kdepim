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
#include <QGraphicsProxyWidget>

ComposerAutoResizer::ComposerAutoResizer(QWidget *parent)
    : QObject(parent),
      composer(qobject_cast<QTextEdit*>(parent)),
      edit(qobject_cast<QFrame*>(parent)),
      flickable(0)
{
    Q_ASSERT(composer);
    // detect when the text changes
    connect(parent, SIGNAL(textChanged()), this, SLOT(textEditChanged()));
    connect(parent, SIGNAL(cursorPositionChanged()), this, SLOT(textEditChanged()));

    // get the original minimum size of the widget
    minimumHeight = edit->size().height();
}

QDeclarativeItem *ComposerAutoResizer::findFlickable(QGraphicsItem *parent)
{
    // looks for a QML Flickable Item based on the name of the class
    // It's not optimal but it's the only way as
    // QDeclarativeFlickable is not public
    const QString flickableClassName("QDeclarativeFlickable");
    while (parent) {
        QDeclarativeItem *di = qobject_cast<QDeclarativeItem*>(parent);
        if (di) {
            if (!flickableClassName.compare(di->metaObject()->className())) {
                return di;
                break;
            }
        }
        parent = parent->parentItem();
    }
    return 0;
}

void ComposerAutoResizer::textEditChanged()
{
    QTextDocument *doc = composer->document();
    const QRect cursor = composer->cursorRect();
    const QSize s = doc->size().toSize();
    const QRect fr = edit->frameRect();
    const QRect cr = edit->contentsRect();

    // sets the size of the widget dynamically
    edit->setMinimumHeight(qMax(minimumHeight, s.height() + (fr.height() - cr.height() - 1)));
    edit->setMaximumHeight(qMax(minimumHeight, s.height() + (fr.height() - cr.height() - 1)));

    //QWidget *pw = edit->parentWidget();
    QGraphicsProxyWidget *proxy = edit->graphicsProxyWidget();
    QGraphicsItem *pi = proxy->parentItem();

    // position of the widget
    const QPointF pos = proxy->pos();

    // make sure the cursor is visible so the user doesn't loose track of the kb focus
    if (flickable || (flickable = findFlickable(pi))) {
        const int dy = cursor.center().y();
        const int y = pos.y() + dy - minimumHeight;
        if (y >= 0) {
            flickable->setProperty("contentY", y);
        } else {
            flickable->setProperty("contentY", 0);
        }
    }
}
