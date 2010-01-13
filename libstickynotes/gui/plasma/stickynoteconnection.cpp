#include "stickynoteconnection.h"

#include <StickyNotes/MemoryNoteItem>
#include <StickyNotes/RemoteNoteItem>
#include <StickyNotes/RemoteNoteController>

#include <Plasma/Containment>

#include <QTcpSocket>
#include <QTimer>

#include "stickynote.h"
#include "stickynoteitem.h"
#include "stickynotewidget.h"

StickyNoteConnection::StickyNoteConnection(StickyNote &_parent, QTcpSocket &_socket)
: QObject(&_parent),
    m_controller(new StickyNotes::RemoteNoteController(_socket)),
    m_parent(_parent)
{
	connect(m_controller, SIGNAL(createdItem(StickyNotes::RemoteNoteItem &)),
	    this, SLOT(on_controller_createdItem(StickyNotes::RemoteNoteItem &)));

	connect(&_socket, SIGNAL(error(QAbstractSocket::SocketError)),
	    this, SLOT(on_socket_error(QAbstractSocket::SocketError)));

	m_controller->startListening();
}
 
StickyNoteConnection::~StickyNoteConnection(void)
{
	foreach(StickyNotes::BaseNoteItem *item, m_roots.values())
	    delete item;
	m_roots.clear();

	m_controller->stopListening();

	disconnect(m_controller, SIGNAL(createdItem(StickyNotes::RemoteNoteItem &)),
	    this, SLOT(on_controller_createdItem(StickyNotes::RemoteNoteItem &)));

	delete m_controller;
}

void
StickyNoteConnection::on_controller_createdItem(StickyNotes::RemoteNoteItem &_item)
{
	StickyNoteItem *item;
	StickyNotes::MemoryNoteItem *memitem;

	// memItem->_item->item
	memitem = new StickyNotes::MemoryNoteItem;
	_item.setParent(memitem);
	item = new StickyNoteItem(&_item);

	m_roots.insert(memitem);
	m_parent.containment()->addApplet(item->applet());
}

void
StickyNoteConnection::on_socket_error(QAbstractSocket::SocketError _error)
{
	Q_UNUSED(_error);

	m_controller->stopListening();

	QTimer::singleShot(0, this, SLOT(deleteLater()));
}

