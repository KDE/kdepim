/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
# pragma interface "EmpathPasswordEditWidget.h"
#endif

#ifndef EMPATHPASSWORDEDITWIDGET_H
#define EMPATHPASSWORDEDITWIDGET_H

// Qt includes
#include <qstring.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qwidget.h>

/**
 * @short Password edit widget
 * Password edit widget. Allows user to switch between echo mode and
 * starred mode.
 * @author Rikkus
 */
class EmpathPasswordEditWidget : public QWidget
{
    Q_OBJECT
    
    public:
    
        EmpathPasswordEditWidget(const QString &, QWidget * parent);
        ~EmpathPasswordEditWidget();

        QString text() const;
        void setText(const QString &);
        
    protected slots:
    
        void s_switchEchoMode(bool);
    
    protected:

        QLineEdit     * le_pass_;
        QPushButton   * pb_echoMode_;
};

#endif

// vim:ts=4:sw=4:tw=78
