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
# pragma interface "EmpathViewFactory.h"
#endif

#ifndef EMPATHVIEWFACTORY_H
#define EMPATHVIEWFACTORY_H

// Qt includes
#include <qmime.h>
#include <qstring.h>
#include <qcstring.h>

// Local includes
#include <RMM_BodyPart.h>

class EmpathXMLMessage : public QMimeSource
{
    public:

        EmpathXMLMessage(RMM::RBodyPart &);

        virtual const char * format (int n = 0) const;
        virtual bool provides (const char *) const;
        virtual QByteArray encodedData (const char *) const;
        
        const char * className() const { return "EmpathXMLMessage"; }

    private:

        void _encode();

        QByteArray data_;
        QCString messageData_;

        bool encoded_;
};

class EmpathViewFactory : public QMimeSourceFactory
{
    public:

        EmpathViewFactory();
        ~EmpathViewFactory();
        void init();
        virtual const QMimeSource * data(const QString & abs_name) const;

        const char * className() const { return "EmpathViewFactory"; }
};

#endif

// vim:ts=4:sw=4:tw=78
