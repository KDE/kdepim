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

#ifndef EMPATH_ENVELOPE_WIDGET_H
#define EMPATH_ENVELOPE_WIDGET_H

// Qt includes
#include <qvbox.h>
#include <qmap.h>
#include <qstring.h>
#include <qlist.h>
#include <qvaluelist.h>

class EmpathHeaderSpecWidget;

/**
 * Widget for specifying the headers when composing a message. It consists
 * of several EmpathHeaderSpecWidgets. It takes care of the key navigation
 * between the EmpathHeaderSpecWidgets.
 */
class EmpathEnvelopeWidget : public QVBox
{
    Q_OBJECT

    public:
        
        /**
         * Constructor. An 'envelope' containing the headers should be 
         * given. 
         */
        EmpathEnvelopeWidget(QWidget * parent = 0);

        virtual ~EmpathEnvelopeWidget();

        void setHeaders(const QMap<QString, QString> &);

        /** 
         * Retrieve the headers when the user has finished composing.
         */
        QMap<QString, QString> headers();
        
        /** 
         * Check if at least one recipient is specified.
         */
        bool haveRecipient();

        bool haveSubject();

    private:

        void _addHeader(const QString & name, const QString & body);
        void _lineUpHeaders();
       
        QList<EmpathHeaderSpecWidget> headerSpecList_;

        int maxSizeColOne_;
};

#endif
// vim:ts=4:sw=4:tw=78
