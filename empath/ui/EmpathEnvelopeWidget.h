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
# pragma interface "EmpathEnvelopeWidget.h"
#endif

#ifndef EMPATHENVELOPEWIDGET_H
#define EMPATHENVELOPEWIDGET_H

// Qt includes
#include <qvbox.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathURL.h"
#include "EmpathHeaderSpecWidget.h"
#include <RMM_Message.h>

/**
 * Widget for specifying the headers when composing a message.
 */
class EmpathEnvelopeWidget : public QVBox
{
    // Q_OBJECT

    public:
        
        /**
         * Standard ctor
         */
        EmpathEnvelopeWidget(
            RMM::REnvelope & headers,
            QWidget * parent = 0, const char * name = 0);

        /**
         * dtor
         */
        ~EmpathEnvelopeWidget();

        RMM::REnvelope & headers();
        
        bool haveTo();
        bool haveSubject();
        
    private:

        void    _addHeader(RMM::RHeader &);
        void    _lineUpHeaders();
       
        RMM::REnvelope headers_;
        
        QList<EmpathHeaderSpecWidget> headerSpecList_;

        int maxSizeColOne_;
};

#endif
// vim:ts=4:sw=4:tw=78
