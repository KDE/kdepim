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

#ifndef COMPOSERAUTORESIZER_H
#define COMPOSERAUTORESIZER_H

#include <QDeclarativeItem>
#include <QTextEdit>

class ComposerAutoResizer : public QObject
{
  Q_OBJECT

  public:
    explicit ComposerAutoResizer( QTextEdit *parent );

    QDeclarativeItem *findFlickable( QGraphicsItem *parent ) const;

  private Q_SLOTS:
    void textEditChanged();

  private:
    QTextEdit *mComposer;
    QDeclarativeItem *mFlickable;
};

#endif
