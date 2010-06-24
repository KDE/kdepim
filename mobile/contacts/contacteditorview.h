/*
    Copyright (c) 2010 Kevin Krammer <kevin.krammer@gmx.at>

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

#ifndef CONTACTEDITORVIEW_H
#define CONTACTEDITORVIEW_H

#include "kdeclarativefullscreenview.h"

namespace Akonadi
{
  class Collection;
  class Item;
}

class EditorBusiness;
class EditorCrypto;
class EditorGeneral;
class EditorLocation;
class EditorMore;

class ContactEditorView : public KDeclarativeFullScreenView
{
  Q_OBJECT

  public:
    explicit ContactEditorView( QWidget *parent = 0 );

    ~ContactEditorView();

    void setEditorGeneral( EditorGeneral *editor );

    void setEditorBusiness( EditorBusiness *editor );

    void setEditorLocation( EditorLocation *editor );

    void setEditorCrypto( EditorCrypto *editor );

    void setEditorMore( EditorMore *editor );

    void loadContact( const Akonadi::Item &item );

  public Q_SLOTS:
    void save();
    void cancel();

  Q_SIGNALS:
    void requestLaunchAccountWizard();

  protected:
    void closeEvent( QCloseEvent *event );
    
  private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void saveFinished() );
    Q_PRIVATE_SLOT( d, void saveFailed( const QString& ) );
    Q_PRIVATE_SLOT( d, void collectionChanged( const Akonadi::Collection& ) )
};

#endif
