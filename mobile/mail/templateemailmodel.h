/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef TEMPLATEEMAILMODEL_H
#define TEMPLATEEMAILMODEL_H

#include <AkonadiCore/selectionproxymodel.h>

/** A simple model that overrides the data() method, so the display role is the subject of the mails.
 * Then in QML one can use "display" to get it.
 */
class TemplateEmailModel : public Akonadi::SelectionProxyModel {
  Q_OBJECT
public:
    explicit TemplateEmailModel( QItemSelectionModel *selectionModel, QObject *parent = 0 ) : Akonadi::SelectionProxyModel( selectionModel, parent)  {
   }

   QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
};


#endif // TEMPLATEEMAILMODEL_H
