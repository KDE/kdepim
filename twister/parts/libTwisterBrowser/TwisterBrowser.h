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

#ifndef TWISTER_BROWSER_H
#define TWISTER_BROWSER_H

// Qt includes
#include <qwidget.h>

// KDE includes
#include <kparts/part.h>
#include <klibloader.h>

// KDE includes
#include <kparts/part.h>

class TwisterBrowser : public QWidget
{
    Q_OBJECT

    public:
        
        TwisterBrowser(QWidget * parent);
        ~TwisterBrowser();

    private:

        void _connectUp();
};

class TwisterBrowserPartFactory : public KLibFactory
{
    Q_OBJECT

    public:

        TwisterBrowserPartFactory();
        virtual ~TwisterBrowserPartFactory();

        virtual QObject * create(
            QObject * parent = 0,
            const char * name = 0,
            const char * classname = "QObject",
            const QStringList & args = QStringList());

        static KInstance * instance();

    private:

        static KInstance * instance_;
};

class TwisterBrowserPart : public KParts::ReadWritePart
{
    Q_OBJECT

    public:
        
        TwisterBrowserPart(QWidget * parent = 0, const char * name = 0);
        virtual ~TwisterBrowserPart();
        void _initActions();

    protected slots:

    signals:
 
    protected:

        virtual bool openFile() { return false; }
        virtual bool saveFile() { return false; }

        void enableAllActions(bool);

    private:

        TwisterBrowser * widget_;
};


#endif
// vim:ts=4:sw=4:tw=78
