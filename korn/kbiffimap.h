/*
 * kbiffimap.h -- Declaration of class KBiffImap
 */
#ifndef KBIFFIMAP_H
#define KBIFFIMAP_H

#include <qstring.h>

/**
 * @internal
 */
class KBiffSocket
{
public:
	KBiffSocket();
	virtual ~KBiffSocket();

	bool connect(const QString & host, unsigned int port);

	int messages() const { return (_messages > -1) ? _messages : 0; }

	void close();

protected:
	QString readLine();
	int writeLine(const QString& line);

	int _socket;
	int _messages;
};

/**
 * IMAP4 Client
 * @author Kurt Granroth (granroth@kde.org)
 * $version $Id$
 */
class KBiffImap : public KBiffSocket
{
public:
	bool command(const QString& line, unsigned int seq);
	QString mungeUser(const QString& old_user);
};
#endif // KBIFFIMAP_H
