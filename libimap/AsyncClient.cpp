/*
   RFC 2060 (IMAP4rev1) client.

   Copyright (C) 2000 Rik Hemsley (rikkus) <rik@kde.org>

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/ 

#include <sys/time.h>
#include <unistd.h> // For usleep
#include <qsocket.h>
#include "IMAPClient.h"

using namespace IMAP;

class AsyncClientPrivate
{
  public:

    QString greeting_;
    QSocket * socket_;
    IMAP::State state_;
    ulong commandCount_;
    QString selected_;
    QString endIndicator_;
    QString buffer_;
};

AsyncClient::AsyncClient(QObject * parent, const char * name)
  : QObject(parent, name)
{
  d = new AsyncClientPrivate;
  d->socket_ = new QSocket(this, "IMAP::AsyncClient socket");
  d->state_ = NotAuthenticated;
  d->commandCount_ = 0;

  QObject::connect(d->socket_, SIGNAL(readyRead()),  this, SLOT(slotDataReady()));
  QObject::connect(d->socket_, SIGNAL(hostFound()),  this, SIGNAL(hostFound()));
  QObject::connect(d->socket_, SIGNAL(connect()),    this, SIGNAL(connected()));
}

AsyncClient::~AsyncClient()
{
  logout();
  Response::cleanup();
  delete d;
}

  QString
AsyncClient::greeting() const
{
  return d->greeting_;
}

  void
AsyncClient::capability()
{
  if (d->socket_->state() != QSocket::Connection) {
    qDebug("AsyncClient::capability(): Not connected to server");
    return;
  }

  runCommand("CAPABILITY");
}

  void
AsyncClient::noop()
{
  if (d->socket_->state() != QSocket::Connection) {
    qDebug("AsyncClient::noop(): Not connected to server");
    return;
  }

  runCommand("NOOP");
}

  void
AsyncClient::logout()
{
  if (d->socket_->state() != QSocket::Connection) {
    qDebug("AsyncClient::logout(): Not connected to server");
    return;
  }

  runCommand("LOGOUT");
}

  void
AsyncClient::authenticate(const QString & /* username */, const QString & /* password */, const QString & /* authType */)
{
  qDebug("%s: STUB", __FUNCTION__);

  if (d->state_ < NotAuthenticated) {
    qDebug("AsyncClient::authenticate(): state < NotAuthenticated");
    return;
  }
}

  void
AsyncClient::login(const QString & username, const QString & password)
{
  if (d->state_ < NotAuthenticated) {
    qDebug("AsyncClient::login(): state < NotAuthenticated");
    return;
  }

  runCommand("LOGIN " + username + " " + password);
}

  void
AsyncClient::selectMailbox(const QString & name)
{
  if (d->state_ < Authenticated) {
    qDebug("AsyncClient::selectMailbox(): state < Authenticated");
    return;
  }

  // Don't re-select.
  if (d->state_ == Selected && d->selected_ == name)
    return;

  runCommand("SELECT " + name);
}

  void
AsyncClient::examineMailbox(const QString & name)
{
  if (d->state_ < Authenticated) {
    qDebug("AsyncClient::examineMailbox(): state < Authenticated");
    return;
  }

  runCommand("EXAMINE " + name);
}

  void
AsyncClient::createMailbox(const QString & name)
{
  if (d->state_ < Authenticated) {
    qDebug("AsyncClient::createMailbox(): state < Authenticated");
    return;
  }

  runCommand("CREATE " + name);
}

  void
AsyncClient::removeMailbox(const QString & name)
{
  if (d->state_ < Authenticated) {
    qDebug("AsyncClient::removeMailbox(): state < Authenticated");
    return;
  }

  runCommand("DELETE " + name);
}

  void
AsyncClient::renameMailbox(const QString & from, const QString & to)
{
  if (d->state_ < Authenticated) {
    qDebug("AsyncClient::renameMailbox(): state < Authenticated");
    return;
  }

  runCommand("RENAME " + from + " " + to);
}

  void
AsyncClient::subscribeMailbox(const QString & name)
{
  if (d->state_ < Authenticated) {
    qDebug("AsyncClient::subscribeMailbox(): state < Authenticated");
    return;
  }

  runCommand("SUBSCRIBE " + name);
}

  void
AsyncClient::unsubscribeMailbox(const QString & name)
{
  if (d->state_ < Authenticated) {
    qDebug("AsyncClient::unsubscribeMailbox(): state < Authenticated");
    return;
  }

  runCommand("UNSUBSCRIBE " + name);
}

  void
AsyncClient::list(
    const QString & ref,
    const QString & wild,
    bool subscribedOnly
)
{
  if (d->state_ < Authenticated) {
    qDebug("AsyncClient::list(): state < Authenticated");
    return;
  }

  QString cmd = subscribedOnly ? "LSUB" : "LIST";

  runCommand(cmd + " \"" + ref + "\" " + wild);
}

  void
AsyncClient::status(
    const QString & mailboxName,
    ulong items
)
{
  if (d->state_ < Authenticated) {
    qDebug("AsyncClient::status(): state < Authenticated");
    return;
  }

  QString s;

  if (items & StatusInfo::MessageCount)
    s += "MESSAGES ";
  if (items & StatusInfo::RecentCount)
    s += "RECENT ";
  if (items & StatusInfo::NextUID)
    s += "UIDNEXT ";
  if (items & StatusInfo::UIDValidity)
    s += "UIDVALIDITY ";
  if (items & StatusInfo::Unseen)
    s += "UNSEEN ";

  if (s[s.length() - 1] == ' ')
    s.truncate(s.length() - 1);

  runCommand("STATUS " + mailboxName + " (" + s + ')');
}

  void
AsyncClient::appendMessage(
    const QString & mailboxName,
    const QString & messageData,
    ulong           flags,
    const QString & date
)
{
  if (d->state_ < Authenticated) {
    qDebug("AsyncClient::appendMessage(): state < Authenticated");
    return;
  }

  QString s("APPEND " + mailboxName);

  if (0 != flags)
    s += " (" + flagsString(flags) + ")";

  if ("" != date)
    s += " " + date;

  s += "\r\n" + messageData;

  runCommand(s);
}

  void
AsyncClient::checkpoint()
{
  if (d->state_ != Selected) {
    qDebug("AsyncClient::checkpoint(): state != Selected");
    return;
  }

  runCommand("CHECKPOINT");
}

  void
AsyncClient::close()
{
  if (d->state_ != Selected) {
    qDebug("AsyncClient::close(): state != Selected");
    return;
  }

  runCommand("CLOSE");
}

  void
AsyncClient::expunge()
{
  if (d->state_ != Selected) {
    qDebug("AsyncClient::expunge(): state != Selected");
    return;
  }

  runCommand("EXPUNGE");
}

    void
AsyncClient::search(
    const QString & spec,
    const QString & charSet,
    bool            usingUID
)
{
  QValueList<ulong> retval;

  if (d->state_ != Selected) {
    qDebug("AsyncClient::search(): state != Selected");
    return;
  }

  QString s("SEARCH " + charSet + " " + spec);

  if (usingUID)
    s.prepend("UID ");

  runCommand(s);
}

  void
AsyncClient::fetch(
    ulong           start,
    ulong           end,
    const QString & spec,
    bool            usingUID
)
{
  QString retval;

  if (d->state_ != Selected) {
    qDebug("AsyncClient::fetch(): state != Selected");
    return;
  }

  QString s(
      "FETCH " + QString::number(start) + ':' + QString::number(end) +
      ' ' + spec
  );

  if (usingUID)
    s.prepend("UID ");

  runCommand(s);
}

  void
AsyncClient::setFlags(
  ulong         start,
  ulong         end,
  FlagSetStyle  style,
  ulong         flags,
  bool          usingUID
)
{
  if (d->state_ != Selected) {
    qDebug("AsyncClient::setFlags(): state != Selected");
    return;
  }

  QString styleString("FLAGS.SILENT");

  switch (style) {

    case Add:
      styleString.prepend('+');
      break;

    case Remove:
      styleString.prepend('-');
      break;

    case Set:
    default:
      break;
  }

  QString s(
      "STORE " +
      QString::number(start) + ":" + QString::number(end) +
      ' ' + styleString + " (" + flagsString(flags) + ')'
  );

  if (usingUID)
    s.prepend("UID ");

  runCommand(s);
}

  void
AsyncClient::copy(ulong start, ulong end, const QString & to, bool usingUID)
{
  if (d->state_ != Selected) {
    qDebug("AsyncClient::copy(): state != Selected");
    return;
  }

  QString s(
      "COPY " +
      QString::number(start) + ":" + QString::number(end) +
      " " + to
  );

  if (usingUID)
    s.prepend("UID ");

  runCommand(s);
}

  void
AsyncClient::runCommand(const QString & cmd)
{
  if (d->socket_->state() != QSocket::Connection) {
    qDebug("AsyncClient::runCommand(): Socket is not connected");
    return;
  }

  QString id;
  id.sprintf("EMPATH_%08ld", d->commandCount_++);

  d->endIndicator_ = id;

  QString command(id + " " + cmd + "\r\n");
  d->socket_->writeBlock(command.ascii(), command.length());

  log("> " + command);
}

  void
AsyncClient::slotDataReady()
{
  if (!d->socket_->canReadLine())
    return;

  QString s = d->socket_->readLine();

  log("< " + s);

  if (!d->endIndicator_)
    d->greeting_ = s;
  else
    d->buffer_ += s + "\r\n";

  if (s.left(d->endIndicator_.length()) == d->endIndicator_)
  {
    Response r(d->buffer_);
    d->buffer_ = QString::null;

    qDebug("Got a response.");
  }
}

  void
AsyncClient::connectToHost(const QString & host, uint port)
{
  d->socket_->connectToHost(host, port);
}

