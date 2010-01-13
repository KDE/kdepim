/*-
 * Copyright 2009 KDAB and Guillermo A. Amaral B. gamaral@amaral.com.mx
 */

#ifndef NOTESERVER_H
#define NOTESERVER_H

#include <QHostAddress>
#include <QObject>

namespace StickyNotes { class BaseNoteItem; }

class QTcpServer;

class NoteServer : public QObject
{
Q_OBJECT

public:
	NoteServer(QObject *_parent = 0);
	~NoteServer(void);


	const QList<StickyNotes::BaseNoteItem *> & items(void) const;
	bool listen(const QHostAddress &_address = QHostAddress::Any,
	    quint16 _port = 0);

private:
	void createItems(void);

private:
	QTcpServer *m_server;
	QList<StickyNotes::BaseNoteItem *> m_items;

private slots:
	void on_server_newConnection(void);

};

#endif // !NOTESERVER_H

