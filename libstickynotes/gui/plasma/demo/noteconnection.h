/*-
 * Copyright 2009 KDAB and Guillermo A. Amaral B. gamaral@amaral.com.mx
 */

#ifndef NOTECONNECTION_H
#define NOTECONNECTION_H

#include <QObject>

#include <QList>

namespace StickyNotes {
	class RemoteNoteItem;
	class RemoteNoteController;
}

class NoteServer;
class QTcpSocket;

class NoteConnection : public QObject
{
Q_OBJECT

public:
	NoteConnection(const NoteServer &_parent, QTcpSocket &_socket);
	virtual ~NoteConnection(void);

private:
	StickyNotes::RemoteNoteController     *m_controller;
	QList<StickyNotes::RemoteNoteItem *> m_widgets;

};

#endif // !NOTECONNECTION_H

