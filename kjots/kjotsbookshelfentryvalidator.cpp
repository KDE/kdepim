/*
    This file is part of KJots.

    Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

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

#include "kjotsbookshelfentryvalidator.h"

KJotsBookshelfEntryValidator::KJotsBookshelfEntryValidator(QAbstractItemModel *model, QObject *parent)
    : QValidator(parent)
{
    m_model = model;
}

KJotsBookshelfEntryValidator::~KJotsBookshelfEntryValidator()
{

}

QValidator::State KJotsBookshelfEntryValidator::validate(QString &input, int &pos) const
{
    Q_UNUSED(pos);
    if (!m_model) {
        return Invalid;
    }
    if (input.isEmpty()) {
        return Intermediate;
    }

    QModelIndexList list = m_model->match(
                               m_model->index(0, 0),
                               Qt::DisplayRole,
                               input,
                               Qt::MatchStartsWith | Qt::MatchFixedString | Qt::MatchWrap);

    if (list.empty()) {
        return Invalid;
    } else {
        foreach (const QModelIndex &index, list) {
            if (0 == QString::compare(m_model->data(index).toString(), input, Qt::CaseInsensitive)) {
                return Acceptable;
            }
            return Intermediate;
        }
    }
    return Invalid;
}

