/*
	imeditorwidget.h

	IM addresses editor widget for KAddressbook

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

#ifndef IMEDITORWIDGET_H
#define IMEDITORWIDGET_H

#include "contacteditorwidget.h"
#include <klistview.h>
#include "imeditorbase.h"

class AddressWidget;

enum IMProtocol { AIM, GaduGadu, Jabber, ICQ, IRC, MSN, SMS, Yahoo, Unknown };

enum IMContext { Any, Home, Work };

/**
 * The widget we add to KAddressbook's contact editor dialog
 */
class IMEditorWidget : public KAB::ContactEditorWidget
{
	Q_OBJECT

	public:
		IMEditorWidget( KABC::AddressBook *ab, QWidget *parent, const char *name = 0 );
		~IMEditorWidget() {};

		/**
		 * Reimplemented from KAB::ContactEditorWidget
		 */
		void loadContact( KABC::Addressee *addr );
		void storeContact( KABC::Addressee *addr );
		void setReadOnly( bool readOnly );
	protected slots:
		void slotUpdateButtons();
		void slotAdd();
		void slotEdit();
		void slotDelete();
	protected:
		/**
		 * Helper method to split the contents of an addressbook field up
		 */
		static void splitField( const QString &str, QString &app, QString &name, QString &value );
		static IMProtocol protocolFromString( const QString protocolName );
		static QString protocolToString( const IMProtocol protocol );
	private:
		bool mReadOnly;
		IMEditorBase *mWidget;
		// Used to track changed protocols to reduce KABC writes
		QValueList<IMProtocol> mChangedProtocols;

};

/**
 * List view item representing a single IM address.
 */

// VCard has been disabled as there is no standard VCard location to store IM addresses yet.
class IMAddressLVI : public KListViewItem
{
	public:
		IMAddressLVI( KListView *parent, IMProtocol protocol, QString address, IMContext context/*, bool inVCard*/ );
		//void setInVCard( bool inVCard );
		void setAddress( const QString &address );
		void setProtocol( IMProtocol protocol );
		void setContext( IMContext context );
		void activate();
		IMProtocol protocol() const;
		QString address() const;
		IMContext context() const;
  //bool inVCard() const;
	private:
		IMProtocol mProtocol;
		IMContext mContext;
		//bool mInVCard;
};

/**
 * Factory class used by KAddressbook to get an instance of the widget
 */
class IMEditorWidgetFactory : public KAB::ContactEditorWidgetFactory
{
	public:
		IMEditorWidgetFactory() {};
		KAB::ContactEditorWidget *createWidget( KABC::AddressBook *ab, QWidget *parent, const char *name )
		{
			return new IMEditorWidget( ab, parent, name );
		}

		QString pageTitle() const;
		QString pageIdentifier() const;
};


#endif
