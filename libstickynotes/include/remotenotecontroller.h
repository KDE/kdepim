/*-
 * Copyright 2009 KDAB and Guillermo A. Amaral B. <gamaral@amaral.com.mx>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
 
#ifndef LSN_REMOTENOTECONTROLLER_H
#define LSN_REMOTENOTECONTROLLER_H

#include <QObject>

#include <StickyNotes/RemoteServiceMessage>

class QIODevice;

namespace StickyNotes {

class RemoteNoteControllerPrivate;
class RemoteNoteItem;
class RemoteNoteMessage;

class LSN_EXPORT RemoteNoteController : public QObject
{
Q_OBJECT
Q_DECLARE_PRIVATE(RemoteNoteController)

	friend class RemoteNoteItem;
public:
	/*! Construct a RemoteNoteController.
	 * \param _device IO device connected to another RemoteNoteController
	 */
	explicit RemoteNoteController(QIODevice &_device);
	virtual ~RemoteNoteController(void);

	QIODevice & device(void) const;

	/*! Start listening to IODevice.
	 */
	void startListening(void);

	/*! Stop listening to IODevice.
	 */
	void stopListening(void);

	/*! Create new master remote item.
	 */
	RemoteNoteItem * createItem(void);

protected:
	/*! Handle remote service message.
	 */
	void handleMessage(RemoteServiceMessage &_message);

	/*! Remove item by id (internal use only).
	 */
	void removeItem(quint32 _id);

	/*! Send message to other end (internal use only).
	 */
	void sendMessage(RemoteServiceMessage::Type _type, quint32 _id,
	    const RemoteNoteMessage &_message) const; 

protected:
	/*! End of Transmission: Set of characters used to signal message end.
	 */
	static const char * const EOT;

private slots:
	void on_device_finished(void);
	void on_device_readyRead(void);

private:
	RemoteNoteControllerPrivate *d_ptr;

signals:
	/*! Triggered on slave item creation.
	 */
	void createdItem(StickyNotes::RemoteNoteItem &_item);

	/*! Triggered before slave item destruction by master.
	 */
	void destroyingItem(StickyNotes::RemoteNoteItem &_item);
};

} // namespace StickyNotes

#endif // !LSN_REMOTESERVICESOURCE_H

