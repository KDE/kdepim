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

#ifndef TWISTER_UI_H
#define TWISTER_UI_H

// Qt includes
#include <qobject.h>

class KActionCollection;
class KAction;

/**
 * @short A KDE interface to Twister
 * A KDE interface to Twister.
 */
class TwisterUI : public QObject
{
    Q_OBJECT

    public:

        ~TwisterUI();

        static TwisterUI * instance()
        {
            if (0 == instance_)
                instance_ = new TwisterUI;

            return instance_;
        }

        KActionCollection * actionCollection()
        { return actionCollection_; }

    protected:

        TwisterUI();
        
    private:
        
        void _init();
        void _connectUp();
        void _initActions();

        static TwisterUI * instance_;
        KActionCollection * actionCollection_;
};

#endif

// vim:ts=4:sw=4:tw=78
