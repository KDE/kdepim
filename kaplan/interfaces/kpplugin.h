/*
    This file is part of Kaplan
    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    Copyright (c) 2002 Daniel Molkentin <molkentin@kde.org>

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

// $Id$

#ifndef KP_PLUGIN_H
#define KP_PLUGIN_H

#include <qobject.h>

#include <kxmlguiclient.h>

class DCOPClient;

namespace Kaplan
{
	class Core;

    /**
      * Base class for all Plugins in Kaplan. Inherit from it
      * to get a plugin. It can insert an icon into the sidepane,
      * add widgets to the widgetstack and add menu items via XMLGUI.
      */
	class Plugin : public QObject, virtual public KXMLGUIClient
	{
	Q_OBJECT

	public:
        /**
          * Creates a new Plugin, note that @param name is required if
          * you want your plugin to do dcop via it's own instance of 
          * @ref DCOPClient by calling @ref dcopClient.
          */
		Plugin(Core *core, QObject *parent, const char *name);
		~Plugin();

		Core *core() const;

        /** 
          * Retrieve the current DCOP Client for the plugin.
          *
          * The clients name is taken from the name argument in the constructor.
          * @note: The DCOPClient object will only be created when this method is
          * called for the first time.
          */
        DCOPClient *dcopClient() const;

	private:
		class Private;
        Private *d;

	};

}


#endif

// vim: ts=4 sw=4 et
