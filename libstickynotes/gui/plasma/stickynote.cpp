#include "stickynote.h"

#include <QHostAddress>
#include <QTcpServer>

#include "stickynoteconnection.h"

K_EXPORT_PLASMA_APPLET(stickynote, StickyNote)

StickyNote::StickyNote(QObject *_parent, const QVariantList &_args)
: Plasma::Applet(_parent, _args), m_server(0)
{
	setAspectRatioMode(Plasma::IgnoreAspectRatio);
	setBackgroundHints(Plasma::Applet::NoBackground);

	resize(0, 0);
}
 
StickyNote::~StickyNote(void)
{
	delete m_server;
}

void
StickyNote::init(void)
{
	m_server = new QTcpServer(this);
	
	connect(m_server, SIGNAL(newConnection()),
	    this, SLOT(on_server_newConnection()));

	// TODO: make address and port customizable
	if (!m_server->listen(QHostAddress::LocalHost, 12345))
		setFailedToLaunch(true,
		    i18n("Failed, You might already have a StickyNotes plasmoid running."));
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
StickyNote::on_server_newConnection(void)
{
	QTcpSocket *client;

	if ((client = m_server->nextPendingConnection()))
		new StickyNoteConnection(*this, *client);
}

