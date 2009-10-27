/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

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

#ifndef AKONOTESPLASMOID_H
#define AKONOTESPLASMOID_H

#include <Plasma/PopupApplet>

#include <Akonadi/Collection>

#include "ui_akonotes_applet.h"

class QAbstractItemModel;
class QModelIndex;

namespace Akonadi
{
class Item;
}

class AkonotesMasterApplet : public Plasma::PopupApplet
{
  Q_OBJECT
public:
  AkonotesMasterApplet( QObject *parent, const QVariantList &args );
  virtual ~AkonotesMasterApplet();

  void paintInterface(QPainter *painter,
        const QStyleOptionGraphicsItem *option,
        const QRect& contentsRect);

  void init();

protected:
  void initExtenderItem( Plasma::ExtenderItem *item, const QModelIndex &idx );
  void updateItem( const Akonadi::Item &item );

  void createConfigurationInterface (KConfigDialog *parent);

protected slots:
  void itemsAdded( const QModelIndex &parent, int start, int end );
  void itemsRemoved( const QModelIndex &parent, int start, int end );
  void dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );

  void configAccepted();

private:
  Plasma::Svg m_svg;
  Akonadi::Collection::Id m_rootCollectionId;

  Ui::AkonotesConfig ui;

  QAbstractItemModel *m_model;

  QString nameForItem( const Akonadi::Item &item );
  Plasma::ExtenderItem* extenderItemForItem( const Akonadi::Item &item );

//   virtual QWidget *widget();

// private:
//   KJotsWidget *w;
};


#endif
