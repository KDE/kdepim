/*    
	imaddresswidget.h
	
	IM address editor widget for KAddressbook
	
	Copyright (c) 2004 Will Stephenson   <lists@stevello.free-online.co.uk>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef IMADDRESSWIDGET_H
#define IMADDRESSWIDGET_H

#include <qvaluelist.h>

#include "imaddressbase.h"
#include "imeditorwidget.h"

class KPluginInfo;

/* Note regarding Context:
 * It wasn not possible to get an idea of Context into Kopete in time for KDE 3.3,
 * so it has been removed from the UI and functionally disabled in the code.
 */

/**
 * A widget for editing an instant messaging address
 */
class IMAddressWidget : public IMAddressBase
{
Q_OBJECT
public:
	IMAddressWidget( QWidget *parent, QValueList<KPluginInfo *> protocols);
	IMAddressWidget( QWidget *parent, QValueList<KPluginInfo *> protocols, KPluginInfo *protocol, const QString& address, const IMContext& context = Any );
	KPluginInfo * protocol();
	IMContext context();
	QString address();
	QValueList<KPluginInfo *> mProtocols;
	
signals:
	void inValidState(bool );
protected:
	/**
	 * Populate combobox with protocols
	 */
	void populateProtocols();
	
protected slots:
	void slotProtocolChanged();

	void slotAddressChanged(const QString &text);
private:
	void init();
};

#endif

