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

#ifndef IMPORTHANDLERBASE_H
#define IMPORTHANDLERBASE_H

#include "mobileui_export.h"

#include <AkonadiCore/item.h>

#include <QtCore/QObject>

class KJob;
class KProgressDialog;
class QItemSelectionModel;

class MOBILEUI_EXPORT ImportHandlerBase : public QObject
{
  Q_OBJECT

  public:
    explicit ImportHandlerBase( QObject *parent = 0 );
    ~ImportHandlerBase();

    void setSelectionModel( QItemSelectionModel *model );

  public Q_SLOTS:
    void exec();

  protected:
    virtual QString fileDialogNameFilter() const = 0;
    virtual QString fileDialogTitle() const = 0;
    virtual QString collectionDialogText() const = 0;
    virtual QString collectionDialogTitle() const = 0;
    virtual QString importDialogText( int count, const QString &collectionName ) const = 0;
    virtual QString importDialogTitle() const = 0;

    virtual QStringList mimeTypes() const = 0;
    virtual Akonadi::Item::List createItems( const QStringList &fileNames, bool *ok ) = 0;

  private Q_SLOTS:
    void slotImportJobDone( KJob* );

  private:
    KProgressDialog *mImportProgressDialog;
    QItemSelectionModel *mSelectionModel;
};

#endif
