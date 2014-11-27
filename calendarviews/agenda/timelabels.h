/*
  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2007 Bruno Virlet <bruno@virlet.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef EVENTVIEWS_TIMELABELS_H
#define EVENTVIEWS_TIMELABELS_H

#include "eventviews_export.h"

#include <KDateTime>

#include <QFrame>

#include <boost/shared_ptr.hpp>

namespace EventViews
{

class Agenda;
class TimeLabelsZone;

class Prefs;
typedef boost::shared_ptr<Prefs> PrefsPtr;

class EVENTVIEWS_EXPORT TimeLabels : public QFrame
{
    Q_OBJECT
public:
    typedef QList<TimeLabels *> List;

    TimeLabels(const KDateTime::Spec &spec, int rows,
               TimeLabelsZone *parent = 0, Qt::WindowFlags f = 0);

    /** updates widget's internal state */
    void updateConfig();

    /**  */
    void setAgenda(Agenda *agenda);

    /**  */
    virtual void paintEvent(QPaintEvent *e);

    /** */
    virtual void contextMenuEvent(QContextMenuEvent *event);

    /** Returns time spec of this label */
    KDateTime::Spec timeSpec();

    /**
      Return string which can be used as a header for the time label.
    */
    QString header() const;

    /**
      Return string which can be used as a tool tip for the header.
    */
    QString headerToolTip() const;

    QSize sizeHint() const Q_DECL_OVERRIDE;

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;

private slots:
    /** update the position of the marker showing the mouse position */
    void mousePosChanged(const QPoint &pos);

    void showMousePos();
    void hideMousePos();

    void setCellHeight(double height);

private:
    void colorMousePos();
    KDateTime::Spec mSpec;
    int mRows;
    double mCellHeight;
    int mMiniWidth;
    Agenda *mAgenda;
    TimeLabelsZone *mTimeLabelsZone;

    QFrame *mMousePos;  // shows a marker for the current mouse position in y direction
};

}

#endif
