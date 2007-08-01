/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
 
#ifndef _KMOBILETOOLS_H_
#define _KMOBILETOOLS_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <KParts/MainWindow>

class KToggleAction;

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author Marco Gulino <marco.gulino@gmail.com>
 * @version 0.5.0
 */

class kmobiletools : public KParts::MainWindow
{
    Q_OBJECT
    public:
        /**
        * Default Constructor
        */
        kmobiletools();

        /**
         * Default Destructor
        */
        virtual ~kmobiletools();

    protected:
        bool queryClose();

    private slots:
        void optionsConfigureKeys();
        void optionsConfigureToolbars();
        void applyNewToolbarConfig();

    private:
        void setupActions();

    private:
        KParts::Part *m_part;
};

#endif // _KMOBILETOOLS_H_
