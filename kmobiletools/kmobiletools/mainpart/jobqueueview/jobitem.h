/***************************************************************************
   Copyright (C) 2007
   by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef JOBITEM_H
#define JOBITEM_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsItem>
#include <KDE/ThreadWeaver/Job>

namespace KMobileTools {
    class JobXP;
}
/**
 * @author Matthias Lechner <matthias@lmme.de>
 */
class JobItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    JobItem( KMobileTools::JobXP* job, QGraphicsItem* parent = 0 );
    ~JobItem();

    QRectF boundingRect() const;

protected:
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0 );

Q_SIGNALS:
    void removeItem( JobItem* item );

private Q_SLOTS:
    void jobSuccessful( ThreadWeaver::Job* );

private:
    bool m_firstPaint;
    QRectF m_boundingRect;

};

#endif
