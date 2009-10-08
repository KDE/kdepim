/*-
 * Copyright 2009 KDAB and Guillermo A. Amaral B. gamaral@amaral.com.mx
 */

#include "noteconnection.h"

#include <QTcpSocket>
#include <StickyNotes/RemoteNoteItem>
#include <StickyNotes/RemoteNoteController>

#include "noteserver.h"

NoteConnection::NoteConnection(const NoteServer &_parent, QTcpSocket &_socket)
{
	m_controller = new StickyNotes::RemoteNoteController(_socket);
	m_controller->startListening();

	foreach (StickyNotes::BaseNoteItem *item, _parent.items()) {
		StickyNotes::RemoteNoteItem *rni = m_controller->createItem();
		rni->setParent(item);
		m_widgets.append(rni);
	}
}

NoteConnection::~NoteConnection(void)
{
	foreach (StickyNotes::RemoteNoteItem *item, m_widgets)
		delete item;

	delete m_controller;
}

