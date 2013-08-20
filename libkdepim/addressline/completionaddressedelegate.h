/*
  This file is part of libkdepim.

  Copyright (c) 2013 Franck Arrecot <franck.arrecot@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KDEPIM_COMPLETIONADDRESSEDELEGATE_H
#define KDEPIM_COMPLETIONADDRESSEDELEGATE_H

#include "kdepim_export.h"
#include <QStyledItemDelegate>
#include <QCache>

class KCompletionBox;
class QItemSelection;
class avatarHelper;

namespace KPIM {


class KDEPIM_EXPORT CompletionAddresseDelegate : public QStyledItemDelegate
{
  Q_OBJECT

  public:
    explicit CompletionAddresseDelegate(KCompletionBox* parent);
    virtual ~CompletionAddresseDelegate();

    // services to feed the memory struct

public Q_SLOTS:
  void onAvatarReady(QString& email, QPixmap avatar);

  private:
    void paint ( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const ;
    QSize sizeHint ( const QStyleOptionViewItem& option, const QModelIndex& index ) const;

    // cache struct !
    QCache<QString,QPixmap> m_cachedPixmaps;
    QPixmap m_defaultPixmap;
    KCompletionBox* m_view ;
};

}

#endif