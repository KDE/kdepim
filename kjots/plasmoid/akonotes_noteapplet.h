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

#ifndef AKONOTES_NOTEAPPLET_H
#define AKONOTES_NOTEAPPLET_H

#include <Plasma/Applet>

#include <akonadi/collection.h>
#include <akonadi/item.h>

class QGraphicsLinearLayout;

class KJob;

namespace Plasma
{
class FrameSvg;
class LineEdit;
class TextEdit;
}

namespace Akonadi
{
class Monitor;
}

class AkonotesNoteApplet : public Plasma::Applet
{
  Q_OBJECT
public:
  AkonotesNoteApplet(QObject* parent, const QVariantList& args);

  virtual void init();

  virtual void paintInterface(QPainter* painter, const QStyleOptionGraphicsItem* option, const QRect& contentsRect);

  virtual void resizeEvent(QGraphicsSceneResizeEvent* event);

protected slots:
  void itemCreateJobFinished( KJob *job );
  void modifyDone( KJob *job );
  void itemsFetched( const Akonadi::Item::List &list );
  void collectionFetchDone( KJob *job );
  void itemFetchDone( KJob *job );

  void itemRemoved();
  void itemChanged( const Akonadi::Item &item );
  void defaultCreated( KJob *job );
  void syncDone( KJob *job );

protected:
  virtual bool eventFilter( QObject* watched, QEvent* event );

private:
  void saveItem();
  void createInDefaultCollection();
  void createDefaultConcreteCollection();

private:
  Plasma::FrameSvg *m_theme;
  Plasma::LineEdit *m_subject;
  Plasma::TextEdit *m_content;
  QGraphicsLinearLayout *m_layout;

  Akonadi::Item m_item;
  Akonadi::Monitor *m_monitor;

};

#endif
