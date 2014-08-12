/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#ifndef EXPORTHANDLERBASE_H
#define EXPORTHANDLERBASE_H

#include "mobileui_export.h"

#include <AkonadiCore/item.h>

#include <QtCore/QObject>

class QItemSelectionModel;

class MOBILEUI_EXPORT ExportHandlerBase : public QObject
{
  Q_OBJECT

  public:
    explicit ExportHandlerBase( QObject *parent = 0 );
    ~ExportHandlerBase();

    void setSelectionModel( QItemSelectionModel *model );
    void setItemSelectionModel( QItemSelectionModel *model );

  public Q_SLOTS:
    void exec();

  protected:
    virtual QString dialogText() const = 0;
    virtual QString dialogAllText() const = 0;
    virtual QString dialogLocalOnlyText() const = 0;

    virtual QStringList mimeTypes() const = 0;
    virtual bool exportItems( const Akonadi::Item::List &items ) = 0;

  private:
    QItemSelectionModel *mSelectionModel;
    QItemSelectionModel *mItemSelectionModel;
};

#endif
