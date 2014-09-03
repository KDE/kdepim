/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    Refactored from earlier code by:
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "multiplyingline.h"

#include <QKeyEvent>

using namespace KPIM;

MultiplyingLine::MultiplyingLine(QWidget *parent) : QWidget(parent)
{
}

void MultiplyingLine::slotReturnPressed()
{
    emit returnPressed(this);
}

void MultiplyingLine::slotPropagateDeletion()
{
    emit deleteLine(this);
}

void MultiplyingLine::slotFocusUp()
{
    emit upPressed(this);
}

void MultiplyingLine::slotFocusDown()
{
    emit downPressed(this);
}

void MultiplyingLine::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Up) {
        emit upPressed(this);
    } else if (ev->key() == Qt::Key_Down) {
        emit downPressed(this);
    }
}

void MultiplyingLine::aboutToBeDeleted()
{
}

