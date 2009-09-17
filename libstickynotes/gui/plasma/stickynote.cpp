#include "stickynote.h"

#include <StickyNotes/MemoryNoteItem>
#include <StickyNotes/RemoteNoteItem>
#include <StickyNotes/RemoteNoteController>

#include <Plasma/Containment>

#include <QHostAddress>
#include <QTcpSocket>
#include <QTimer>

#include "stickynoteitem.h"
#include "stickynotewidget.h"

K_EXPORT_PLASMA_APPLET(stickynote, StickyNote)

StickyNote::StickyNote(QObject *_parent, const QVariantList &_args)
: Plasma::Applet(_parent, _args), m_controller(0), m_socket(0), m_timer(0)
{
	setAspectRatioMode(Plasma::IgnoreAspectRatio);
	setBackgroundHints(Plasma::Applet::NoBackground);

	m_timer = new QTimer(this);
	m_timer->setInterval(3000);

	connect(m_timer, SIGNAL(timeout()),
	    this, SLOT(on_timer_timeout()));

	resize(0, 0);

	reset();
}
 
StickyNote::~StickyNote(void)
{
	delete m_timer;

	clear();
}

void
StickyNote::init(void)
{
}
 
void
StickyNote::paintInterface(QPainter *_painter,
    const QStyleOptionGraphicsItem *_option, const QRect &_rect)
{
	Q_UNUSED(_option);
	Q_UNUSED(_painter);
	Q_UNUSED(_rect);
}

void
StickyNote::clear(void)
{
	foreach(StickyNotes::BaseNoteItem *item, m_roots.values())
	    delete item;
	m_roots.clear();

	if (m_controller) {
		m_controller->stopListening();

		disconnect(m_controller, SIGNAL(createdItem(StickyNotes::RemoteNoteItem &)),
		    this, SLOT(on_controller_createdItem(StickyNotes::RemoteNoteItem &)));

		delete m_controller;
		m_controller = 0;
	}

	if (m_socket) {
		disconnect(m_socket, SIGNAL(connected()),
		    this, SLOT(on_socket_connected()));

		disconnect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
		    this, SLOT(on_socket_error(QAbstractSocket::SocketError)));

		if (m_socket->isValid()
		    && QAbstractSocket::ConnectedState == m_socket->state()) 
			m_socket->disconnectFromHost();

		delete m_socket;
	}
	m_socket = 0;
}

void
StickyNote::reset(void)
{
	clear();

	m_socket = new QTcpSocket(this);

	connect(m_socket, SIGNAL(connected()),
	    this, SLOT(on_socket_connected()));

	connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
	    this, SLOT(on_socket_error(QAbstractSocket::SocketError)));

	// TODO: make configurable later on
	m_socket->connectToHost(QHostAddress::LocalHost, 1234);
}

void
StickyNote::on_controller_createdItem(StickyNotes::RemoteNoteItem &_item)
{
	StickyNoteItem *item;
	StickyNotes::MemoryNoteItem *memitem;

	// memItem->_item->item
	memitem = new StickyNotes::MemoryNoteItem;
	_item.setParent(memitem);
	item = new StickyNoteItem(&_item);

	m_roots.insert(memitem);
	containment()->addApplet(item->applet());
}

void
StickyNote::on_socket_connected(void)
{
	m_timer->stop();

	m_controller = new StickyNotes::RemoteNoteController(*m_socket);

	connect(m_controller, SIGNAL(createdItem(StickyNotes::RemoteNoteItem &)),
	    this, SLOT(on_controller_createdItem(StickyNotes::RemoteNoteItem &)));

	m_controller->startListening();
}

void
StickyNote::on_socket_error(QAbstractSocket::SocketError _error)
{
	Q_UNUSED(_error);
	clear();
	m_timer->start();
}

void
StickyNote::on_timer_timeout(void)
{
	m_timer->stop();

	reset();
}

