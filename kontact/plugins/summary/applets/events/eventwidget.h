/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 * 
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef EVENTWIDGET_H
#define EVENTWIDGET_H

#include <Plasma/Frame>

#include <kdatetime.h>

namespace Plasma
{
    class IconWidget;
    class Label;
} // namespace Plasma

class QGraphicsLinearLayout;
class GradientProgressWidget;

class EventWidget : public Plasma::Frame
{
Q_OBJECT
public:
    EventWidget( QVariantHash data, QGraphicsWidget* parent = 0 );
public slots:
    void setData( QVariantHash data );
    void initUI();
    void updateSummaryUI();
    void updateFullUI();
    void toggleMoreInfo();

    void mouseEnterEvent( QMouseEvent* ev );
    void mouseLeaveEvent( QMouseEvent* ev );

    void setMoreInfoVisible(bool visible = true );
    bool moreInfoVisible();

    QString summary();

private:
    QGraphicsWidget* m_parent;

    KDateTime m_startDate;
    KDateTime m_endDate;
    QString m_summary;
    bool m_allDay;
    QString m_type;
    bool m_moreInfoVisible;
    QString m_description;

    Plasma::IconWidget* m_icon;
    QGraphicsLinearLayout* m_masterLayout;
    QGraphicsWidget* m_summaryWidget;
    QGraphicsWidget* m_fullViewWidget;
    Plasma::Label* m_summaryLabel;
    GradientProgressWidget* m_timetil;
    Plasma::IconWidget* m_moreInfoIcon;
    Plasma::Label* m_startDateLabel;
    Plasma::Label* m_descriptionLabel;
};


#endif
