/*-
 * Copyright 2009 KDAB and Guillermo A. Amaral B. gamaral@amaral.com.mx
 */

#include "noteserver.h"

#include <QTcpServer>
#include <QTcpSocket>

#include <StickyNotes/MemoryNoteItem>

#include "noteconnection.h"

NoteServer::NoteServer(QObject *_parent)
{
	m_server = new QTcpServer(this);

	connect(m_server, SIGNAL(newConnection()),
	    this, SLOT(on_server_newConnection()));

	createItems();
}

NoteServer::~NoteServer(void)
{
	foreach(StickyNotes::AbstractNoteItem *item, m_items)
		delete item;

	delete m_server;
}

const QList<StickyNotes::BaseNoteItem *> &
NoteServer::items(void) const
{
	return (m_items);
}

bool
NoteServer::listen(const QHostAddress &_address, quint16 _port)
{
	return (m_server->listen(_address, _port));
}

void
NoteServer::createItems(void)
{
	m_items.append(new StickyNotes::MemoryNoteItem(QString("Shopping"),
	    QString("<center>Milk<br />Soap<br />Shampoo</center>")));
}

void
NoteServer::on_server_newConnection(void)
{
	QTcpSocket *sock = m_server->nextPendingConnection();

	if (!sock)
		return;

	new NoteConnection(*this, *sock);
}

