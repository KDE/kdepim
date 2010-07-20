/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
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

#ifndef SPECIALDATEWIDGET_H
#define SPECIALDATEWIDGET_H

#include <Plasma/Frame>

namespace Plasma {
    class IconWidget;
}

class KUrl;

class QVariant;
class QIcon;
class QGraphicsLinearLayout;
#include <QDate>
#include <KUrl>

class SpecialDateWidget : public Plasma::Frame
{
Q_OBJECT
public:
    explicit SpecialDateWidget(QGraphicsWidget* parent, QString text, QString icon, KUrl uri, QDate date);
    
    Plasma::IconWidget* icon();
    void setIcon(Plasma::IconWidget* icon);

    QDate date();
    void setDate(QDate date);
    
    void init();
    
    KUrl uri();
    void setUri(KUrl uri);
    
private slots:
    void click();

private:
    QVariant m_data;
    QDate m_date;
    Plasma::IconWidget* m_icon;
    QString m_text;
    KUrl m_uri;
    
    QGraphicsLinearLayout* m_layout;
};

#endif
