/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef INCIDENCEEDITOR_VISUALFREEBUSYWIDGET_H
#define INCIDENCEEDITOR_VISUALFREEBUSYWIDGET_H

#include "incidenceeditors-ng_export.h"

#include <KDateTime>

#include <QWidget>

namespace KDGantt {
  class DateTimeGrid;
  class GraphicsView;
}

class KComboBox;

class QTreeView;

class FreeBusyItemModel;

namespace IncidenceEditorNG {

class FreeBusyGanttProxyModel;
class RowController;

class INCIDENCEEDITORS_NG_EXPORT VisualFreeBusyWidget : public QWidget
{
  Q_OBJECT
  public:
    explicit VisualFreeBusyWidget( FreeBusyItemModel *model, int spacing = 8, QWidget *parent = 0 );
    ~VisualFreeBusyWidget();

  public slots:
    void slotUpdateIncidenceStartEnd( const KDateTime &, const KDateTime & );

  signals:
    void dateTimesChanged( const KDateTime &, const KDateTime & );
    void manualReload();

  protected slots:
    void slotScaleChanged( int );
    void slotCenterOnStart() ;
    void slotZoomToTime();
    void slotPickDate();
    void showAttendeeStatusMenu();
    void slotIntervalColorRectangleMoved( const KDateTime &start, const KDateTime &end );

  private slots:
    void splitterMoved();

  private:
    KDGantt::GraphicsView *mGanttGraphicsView;
    QTreeView *mLeftView;
    RowController *mRowController;
    KDGantt::DateTimeGrid *mGanttGrid;
    KComboBox *mScaleCombo;
    FreeBusyGanttProxyModel *mModel;

    KDateTime mDtStart, mDtEnd;
};

}
#endif
