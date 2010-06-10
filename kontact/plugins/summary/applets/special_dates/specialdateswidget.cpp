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

#include "specialdateswidget.h"

#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/DataEngine>

SpecialDate::SpecialDate(QString date, QString name )
    : Plasma::Label(),
      m_dateString(date),
      m_name(name)
{
    kDebug() << "new label" << date << name;
    
    init();
}

void SpecialDate::init()
{
    QDate currentDate = QDate::currentDate();
    m_date = QDate::fromString(m_dateString, "yyyy-MM-dd");
    
    QString text = "<table><tr><td width='33%'><h4>%1</h4></td><td width='57%' style='text-align:center'>%2</td><td width='10%' style='foreground:red'><b>%3<b></td></tr><table>";
    text = text.arg( m_date.toString("ddd, d MMM") );
    text = text.arg( m_name );
    text = text.arg( currentDate.daysTo(m_date) );
    
    setText(text);
}

#include "specialdateswidget.moc"