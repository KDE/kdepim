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

#include <config.h>
#include <sys/time.h>
#include <unistd.h> // For usleep
#include "IMAPClient.h"

using namespace IMAP;

class ClientPrivate
{
  public:

    QString greeting_;
    QIODevice * device_;
    IMAP::State state_;
    ulong commandCount_;
    QString selected_;
};

Client::Client(QIODevice * dev)
{
  d = new ClientPrivate;
  d->device_ = dev;
  d->state_ = NotAuthenticated;
  d->commandCount_ = 0;
  d->greeting_ = response(QString::null);
}

Client::~Client()
{
  logout();
  Response::cleanup();
  delete d;
}

  QString
Client::greeting() const
{
  return d->greeting_;
}

  QString
Client::capability()
{
  if (!d->device_->isOpen()) {
    qDebug("Client::capability(): Not connected to server");
    return QString();
  }

  Response r = runCommand("CAPABILITY");

  return r.data();
}

  bool
Client::noop()
{
  if (!d->device_->isOpen()) {
    qDebug("Client::noop(): Not connected to server");
    return false;
  }

  Response r = runCommand("NOOP");

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
Client::logout()
{
  if (!d->device_->isOpen()) {
    qDebug("Client::logout(): Not connected to server");
    return false;
  }

  Response r = runCommand("LOGOUT");

  bool ok = (r.statusCode() == Response::StatusCodeOk);

  if (!ok)
    return false;

  d->state_ = Logout;

  return true;
}

  bool
Client::authenticate(const QString & /* username */, const QString & /* password */, const QString & /* authType */)
{
  qDebug("%s: STUB", __FUNCTION__);

  if (d->state_ < NotAuthenticated) {
    qDebug("Client::authenticate(): state < NotAuthenticated");
    return false;
  }

  return false;
}

  bool
Client::login(const QString & username, const QString & password)
{
  if (d->state_ < NotAuthenticated) {
    qDebug("Client::login(): state < NotAuthenticated");
    return false;
  }

  Response r = runCommand("LOGIN " + username + " " + password);

  bool ok = (r.statusCode() == Response::StatusCodeOk);

  if (ok)
    d->state_ = Authenticated;

  return ok;
}

  bool
Client::selectMailbox(const QString & name, MailboxInfo & info)
{
  if (d->state_ < Authenticated) {
    qDebug("Client::selectMailbox(): state < Authenticated");
    return false;
  }

  // Don't re-select.
  if (d->state_ == Selected && d->selected_ == name)
    return true;

  Response r = runCommand("SELECT " + name);

  bool ok = (r.statusCode() == Response::StatusCodeOk);

  if (!ok)
    return false;

  info = MailboxInfo(r.allData());

  d->state_ = Selected;
  d->selected_ = name;

  return true;
}

  bool
Client::examineMailbox(const QString & name, MailboxInfo & info)
{
  if (d->state_ < Authenticated) {
    qDebug("Client::examineMailbox(): state < Authenticated");
    return false;
  }

  Response r = runCommand("EXAMINE " + name);

  bool ok = (r.statusCode() == Response::StatusCodeOk);

  if (!ok)
    return false;

  info = MailboxInfo(r.allData());

  return true;
}

  bool
Client::createMailbox(const QString & name)
{
  if (d->state_ < Authenticated) {
    qDebug("Client::createMailbox(): state < Authenticated");
    return false;
  }

  Response r = runCommand("CREATE " + name);

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
Client::removeMailbox(const QString & name)
{
  if (d->state_ < Authenticated) {
    qDebug("Client::removeMailbox(): state < Authenticated");
    return false;
  }

  Response r = runCommand("DELETE " + name);

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
Client::renameMailbox(const QString & from, const QString & to)
{
  if (d->state_ < Authenticated) {
    qDebug("Client::renameMailbox(): state < Authenticated");
    return false;
  }

  Response r = runCommand("RENAME " + from + " " + to);

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
Client::subscribeMailbox(const QString & name)
{
  if (d->state_ < Authenticated) {
    qDebug("Client::subscribeMailbox(): state < Authenticated");
    return false;
  }

  Response r = runCommand("SUBSCRIBE " + name);

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
Client::unsubscribeMailbox(const QString & name)
{
  if (d->state_ < Authenticated) {
    qDebug("Client::unsubscribeMailbox(): state < Authenticated");
    return false;
  }

  Response r = runCommand("UNSUBSCRIBE " + name);

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
Client::list(
    const QString & ref,
    const QString & wild,
    QValueList<ListResponse> & responseList,
    bool subscribedOnly
)
{
  if (d->state_ < Authenticated) {
    qDebug("Client::list(): state < Authenticated");
    return false;
  }

  QString cmd = subscribedOnly ? "LSUB" : "LIST";

  Response r = runCommand(cmd + " \"" + ref + "\" " + wild);

  bool ok = (r.statusCode() == Response::StatusCodeOk);

  if (!ok)
    return false;

  responseList.clear();

  QStringList l(QStringList::split("\r\n", (r.data())));

  QStringList::ConstIterator it(l.begin());

  for (; it != l.end(); ++it)
    responseList.append(ListResponse(*it));

  return true;
}

  bool
Client::status(
    const QString & mailboxName,
    ulong items,
    StatusInfo & retval
)
{
  if (d->state_ < Authenticated) {
    qDebug("Client::status(): state < Authenticated");
    return false;
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

  Response r = runCommand("STATUS " + mailboxName + " (" + s + ')');

  bool ok = (r.statusCode() == Response::StatusCodeOk);

  if (!ok)
    return false;

  QStringList l(QStringList::split("\r\n", r.data()));

  QString resp(l.last());

  retval = StatusInfo(resp);

  return true;
}

  bool
Client::appendMessage(
    const QString & mailboxName,
    const QString & messageData,
    ulong           flags,
    const QString & date
)
{
  if (d->state_ < Authenticated) {
    qDebug("Client::appendMessage(): state < Authenticated");
    return false;
  }

  QString s("APPEND " + mailboxName);

  if (0 != flags)
    s += " (" + flagsString(flags) + ")";

  if ("" != date)
    s += " " + date;

  s += "\r\n" + messageData;

  Response r = runCommand(s);

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
Client::checkpoint()
{
  if (d->state_ != Selected) {
    qDebug("Client::checkpoint(): state != Selected");
    return false;
  }

  Response r = runCommand("CHECKPOINT");

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
Client::close()
{
  if (d->state_ != Selected) {
    qDebug("Client::close(): state != Selected");
    return false;
  }

  Response r = runCommand("CLOSE");

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
Client::expunge(QValueList<ulong> & ret)
{
  if (d->state_ != Selected) {
    qDebug("Client::expunge(): state != Selected");
    return false;
  }

  Response r = runCommand("EXPUNGE");

  bool ok = (r.statusCode() == Response::StatusCodeOk);

  if (!ok)
    return false;

  ret.clear();

  QStringList l(QStringList::split("\r\n", r.data()));

  QStringList::ConstIterator it(l.begin());

  for (; it != l.end(); ++it) {

    QStringList tokens(QStringList::split(' ', *it));

    if (tokens[0] == "*" && tokens[2] == "EXPUNGE")
      ret << tokens[1].toULong();
  }

  return true;
}

    QValueList<ulong>
Client::search(
    const QString & spec,
    const QString & charSet,
    bool            usingUID
)
{
  QValueList<ulong> retval;

  if (d->state_ != Selected) {
    qDebug("Client::search(): state != Selected");
    return retval;
  }

  QString s("SEARCH " + charSet + " " + spec);

  if (usingUID)
    s.prepend("UID ");

  Response r = runCommand(s);

  bool ok = (r.statusCode() == Response::StatusCodeOk);

  if (!ok)
    return retval;

  QString resp(r.data());

  int endOfFirstLine = resp.find("\r\n");

  if (-1 == endOfFirstLine) {
    qDebug("No end of first line");
    return retval;
  }

  resp.truncate(endOfFirstLine);

  QStringList l(QStringList::split(' ', resp));

  if (l.count() < 3 || l[0] != "*" || l[1] != "SEARCH")
    return retval;

  l.remove(l.begin());
  l.remove(l.begin());

  QStringList::ConstIterator it(l.begin());

  for (; it != l.end(); ++it)
    retval << (*it).toULong();

  return retval;
}

  QString
Client::fetch(
    ulong           start,
    ulong           end,
    const QString & spec,
    bool            usingUID
)
{
  QString retval;

  if (d->state_ != Selected) {
    qDebug("Client::fetch(): state != Selected");
    return retval;
  }

  QString s(
      "FETCH " + QString::number(start) + ':' + QString::number(end) +
      ' ' + spec
  );

  if (usingUID)
    s.prepend("UID ");

  Response r = runCommand(s);

  bool ok = (r.statusCode() == Response::StatusCodeOk);

  if (!ok)
    return retval;

  retval = r.data();

  return retval;
}

  bool
Client::setFlags(
  ulong         start,
  ulong         end,
  FlagSetStyle  style,
  ulong         flags,
  bool          usingUID
)
{
  if (d->state_ != Selected) {
    qDebug("Client::setFlags(): state != Selected");
    return false;
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

  Response r = runCommand(s);

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
Client::copy(ulong start, ulong end, const QString & to, bool usingUID)
{
  if (d->state_ != Selected) {
    qDebug("Client::copy(): state != Selected");
    return false;
  }

  QString s(
      "COPY " +
      QString::number(start) + ":" + QString::number(end) +
      " " + to
  );

  if (usingUID)
    s.prepend("UID ");

  Response r = runCommand(s);

  return (r.statusCode() == Response::StatusCodeOk);
}

  Response
Client::runCommand(const QString & cmd)
{
  if (!d->device_->isOpen()) {
    qDebug("Client::runCommand(): Socket is not connected");
    return Response("");
  }

  QString id;
  id.sprintf("EMPATH_%08ld", d->commandCount_++);

  QString command(id + " " + cmd + "\r\n");
  d->device_->writeBlock(command.ascii(), command.length());

#ifdef IMAP_DEBUG
  log("> " + command);
#endif

  return Response(response(id));
}

  QString
Client::response(const QString & endIndicator)
{
  QString out;

  int max(4096);
  QByteArray buf(max);

  int pos(0);

#if 0
  timeval t, t2;
  gettimeofday(&t, NULL);
#endif

  while (true) 
  {
    int bytesRead = d->device_->readBlock(buf.data(), buf.size());

    if (bytesRead > 0) {

      out += QString::fromUtf8(buf.data(), bytesRead);

      if (!endIndicator && out.right(out.length() - pos).contains("\r\n"))
          break;

      if (out.right(2) == "\r\n")
      {
        int prevNewLine = out.findRev("\r\n", out.length() - 2);

        if (-1 == prevNewLine)
          break;

        qDebug("PREVNEWLINE == %d", prevNewLine);
        qDebug("IND LEN == %d", endIndicator.length());

        QString interestingStartOfLastLine =
            out.mid(prevNewLine + 2, endIndicator.length() + 2);

        qDebug("INTERESTING: `%s'", interestingStartOfLastLine.ascii());
        qDebug("END_INDICATOR: `%s'", endIndicator.ascii());

        if (interestingStartOfLastLine == endIndicator)
          break;
      }

      pos += bytesRead;

    } else {

      usleep(100);
    }
  }

#if 0
  gettimeofday(&t2, NULL);

  int uselapsed = 1000000 * (t2.tv_sec - t.tv_sec) + t2.tv_usec - t.tv_usec;
  qDebug("uselapsed = %d", uselapsed);
#endif

  log("< " + out);

  return out;
}

