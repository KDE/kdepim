/*
    Twister - PIM app for KDE
    
    Copyright 2000
        Rik Hemsley <rik@kde.org>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef TWISTER_PART_HOLDER_H
#define TWISTER_PART_HOLDER_H

// Qt includes
#include <qwidgetstack.h>

class TwisterPartHolder : public QWidgetStack
{
    Q_OBJECT

    public:
        
        TwisterPartHolder(QWidget * parent);
        ~TwisterPartHolder();

    protected slots:

        void s_switchWidget(const QString &);

    private:

        void _loadMailWidget();
        void _loadCalendarWidget();

        QWidget * mailWidget_;
        QWidget * calendarWidget_;
};


#endif
// vim:ts=4:sw=4:tw=78
