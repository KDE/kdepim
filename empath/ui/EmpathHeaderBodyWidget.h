/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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

#ifdef __GNUG__
# pragma interface "EmpathHeaderBodyWidget.h"
#endif

#ifndef EMPATHHEADERBODYWIDGET_H
#define EMPATHHEADERBODYWIDGET_H

// Qt includes
#include <qwidget.h>

/**
 * Abstract base class for widgets in which the user can edit a header,
 * such as EmpathAddressSelectionWidget for addresses and EmpathText-
 * SelectionWidget for the subject. Both use a KLineEdit widget for this
 * but if you are crazy enough you can use a combobox for example.
 */
class EmpathHeaderBodyWidget : public QWidget
{
    Q_OBJECT
    
    public:
    
        EmpathHeaderBodyWidget(QWidget * parent = 0, const char * name = 0);
        virtual ~EmpathHeaderBodyWidget();

        virtual QString text() const = 0;
        virtual void setText(const QString &) = 0;
};

#endif

// vim:ts=4:sw=4:tw=78
