#include <qapplication.h>
#include <qtextstream.h>
#include <qfile.h>
#include <unistd.h>
#include "EmpathMailboxIMAP4.h"

// IMAP4rev1 (RFC2060) client implementation

static QFile logfile;

void log(const QString & s)
{
  if (!logfile.isOpen()) {
    logfile.setName("log");
    if (!logfile.open(IO_WriteOnly))
      abort();
  }

  QTextStream t(&logfile);
  t << s;
}

void printSocketState(QSocket & sock)
{
  switch (sock.state()) {

    case QSocket::Idle:
      qDebug("Socket idle");
      break;
    case QSocket::HostLookup:
      qDebug("Socket looking up host");
      break;
    case QSocket::Connecting:
      qDebug("Socket connecting");
      break;
    case QSocket::Listening:
      qDebug("Socket listening");
      break;
    case QSocket::Connection:
      qDebug("Socket connected");
      break;
    case QSocket::Closing:
      qDebug("Socket closing");
      break;

    default:
      qDebug("Socket in unknown state");
      break;
  }
}

EmpathMailboxIMAP4::EmpathMailboxIMAP4()
  : state_(Closed),
    commandCount_(0),
    port_(0)
{
}

EmpathMailboxIMAP4::~EmpathMailboxIMAP4()
{
  logout();
  disconnect();
  Response::cleanup();
}

  ulong
EmpathMailboxIMAP4::_flags(const QString & flagsString)
{
  ulong flags = 0;

  if (0 != flagsString.contains("\\Seen"))
    flags ^= Seen;
  if (0 != flagsString.contains("\\Answered"))
    flags ^= Answered;
  if (0 != flagsString.contains("\\Flagged"))
    flags ^= Flagged;
  if (0 != flagsString.contains("\\Deleted"))
    flags ^= Deleted;
  if (0 != flagsString.contains("\\Draft"))
    flags ^= Draft;
  if (0 != flagsString.contains("\\Recent"))
    flags ^= Recent;

  return flags;
}

  void
EmpathMailboxIMAP4::setServerDetails(const QString & hostname, uint port)
{
  hostname_ = hostname;
  port_     = port;
}

  void
EmpathMailboxIMAP4::setLoginDetails(
    const QString & username,
    const QString & password
)
{
  username_ = username;
  password_ = password;
}

  bool
EmpathMailboxIMAP4::connect()
{
  if (state_ != Closed) {
    qDebug("EmpathMailboxIMAP4::connect(): state_ != Idle");
    return false;
  }

  qDebug("Connecting to %s:%d ", hostname_.ascii(), port_);
  socket_.connectToHost(hostname_, port_);

  int timeout = 0;

  while (timeout < 5)
  {
    printSocketState(socket_);

    if (socket_.state() == QSocket::Connection)
      break;

    timeout++;

    sleep(1);
    qApp->processEvents();
  }

  printSocketState(socket_);

  if (socket_.state() != QSocket::Connection)
  {
    qDebug("Couldn't connect");
    return false;
  }

  qDebug("Connected");

  state_ = NotAuthenticated;

  QStringList greeting = response(QString::null);

  return true;
}

  void
EmpathMailboxIMAP4::disconnect()
{
  socket_.disconnect();
  state_ = Closed;
}

  QString
EmpathMailboxIMAP4::capability()
{
  if (socket_.state() != QSocket::Connection) {
    qDebug("EmpathMailboxIMAP4::capability(): Not connected to server");
    return QString();
  }

  Response r = runCommand("CAPABILITY");

  return r.data()[0];
}

  bool
EmpathMailboxIMAP4::noop()
{
  if (socket_.state() != QSocket::Connection) {
    qDebug("EmpathMailboxIMAP4::noop(): Not connected to server");
    return false;
  }

  Response r = runCommand("NOOP");

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
EmpathMailboxIMAP4::logout()
{
  if (socket_.state() != QSocket::Connection) {
    qDebug("EmpathMailboxIMAP4::logout(): Not connected to server");
    return false;
  }

  Response r = runCommand("LOGOUT");

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
EmpathMailboxIMAP4::authenticate()
{
  qDebug("%s: STUB", __FUNCTION__);

  if (state_ < NotAuthenticated) {
    qDebug("EmpathMailboxIMAP4::authenticate(): state < NotAuthenticated");
    return false;
  }

  return false;
}

  bool
EmpathMailboxIMAP4::login()
{
  if (state_ < NotAuthenticated) {
    qDebug("EmpathMailboxIMAP4::login(): state < NotAuthenticated");
    return false;
  }

  Response r = runCommand("LOGIN " + username_ + " " + password_);

  bool ok = (r.statusCode() == Response::StatusCodeOk);

  if (ok)
    state_ = Authenticated;

  return ok;
}

  bool
EmpathMailboxIMAP4::selectMailbox(const QString & name, MailboxInfo & info)
{
  if (state_ < Authenticated) {
    qDebug("EmpathMailboxIMAP4::selectMailbox(): state < Authenticated");
    return false;
  }

  Response r = runCommand("SELECT " + name);

  bool ok = (r.statusCode() == Response::StatusCodeOk);

  if (!ok)
    return false;

  QStringList lines(r.data());

  QStringList::ConstIterator it(lines.begin());

  for (; it != lines.end(); ++it) {

    QString line = *it;

    QStringList tokens(QStringList::split(' ', line));

    if (tokens[2] == "EXISTS")
      info.setCount(tokens[1].toULong());

    else if (tokens[2] == "RECENT")
      info.setRecent(tokens[1].toULong());

    else if (tokens[1] == "FLAGS")
    {
      int flagsStart  = line.find('(');
      int flagsEnd    = line.find(')');

      if ((-1 != flagsStart) && (-1 != flagsEnd) && flagsStart < flagsEnd)
        info.setFlags(_flags(line.mid(flagsStart, flagsEnd)));
    }

    else if (tokens[2] == "[UNSEEN")
      info.setUnseen(tokens[3].left(tokens[3].length() - 2).toULong());

    else if (tokens[2] == "[UIDVALIDITY") {
      qDebug(tokens[3].ascii());
      info.setUidValidity(tokens[3].left(tokens[3].length() - 2).toULong());
    }

    else if (tokens[2] == "[PERMANENTFLAGS")
    {
      int flagsStart  = line.find('(');
      int flagsEnd    = line.find(')');

      if ((-1 != flagsStart) && (-1 != flagsEnd) && flagsStart < flagsEnd)
        info.setPermanentFlags(_flags(line.mid(flagsStart, flagsEnd)));
    }
  }

  info.setReadWrite(0 != lines[lines.count() - 1].contains("[READ-WRITE]"));

  return true;
}

  bool
EmpathMailboxIMAP4::examineMailbox(const QString & name, MailboxInfo & info)
{
  if (state_ < Authenticated) {
    qDebug("EmpathMailboxIMAP4::examineMailbox(): state < Authenticated");
    return false;
  }

  // Is this really just the same thing ?
  return selectMailbox(name, info);
}

  bool
EmpathMailboxIMAP4::createMailbox(const QString & name)
{
  if (state_ < Authenticated) {
    qDebug("EmpathMailboxIMAP4::createMailbox(): state < Authenticated");
    return false;
  }

  Response r = runCommand("CREATE " + name);

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
EmpathMailboxIMAP4::removeMailbox(const QString & name)
{
  if (state_ < Authenticated) {
    qDebug("EmpathMailboxIMAP4::removeMailbox(): state < Authenticated");
    return false;
  }

  Response r = runCommand("DELETE " + name);

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
EmpathMailboxIMAP4::renameMailbox(const QString & from, const QString & to)
{
  if (state_ < Authenticated) {
    qDebug("EmpathMailboxIMAP4::renameMailbox(): state < Authenticated");
    return false;
  }

  Response r = runCommand("RENAME " + from + " " + to);

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
EmpathMailboxIMAP4::subscribeMailbox(const QString & name)
{
  if (state_ < Authenticated) {
    qDebug("EmpathMailboxIMAP4::subscribeMailbox(): state < Authenticated");
    return false;
  }

  Response r = runCommand("SUBSCRIBE " + name);

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
EmpathMailboxIMAP4::unsubscribeMailbox(const QString & name)
{
  if (state_ < Authenticated) {
    qDebug("EmpathMailboxIMAP4::unsubscribeMailbox(): state < Authenticated");
    return false;
  }

  Response r = runCommand("UNSUBSCRIBE " + name);

  return (r.statusCode() == Response::StatusCodeOk);
}

  QStringList
EmpathMailboxIMAP4::list(const QString & ref, const QString & wild)
{
  qDebug("%s: STUB", __FUNCTION__);
  if (state_ < Authenticated) {
    qDebug("EmpathMailboxIMAP4::list(): state < Authenticated");
    return QStringList();
  }

  Response r = runCommand("LIST " + ref + wild);

  bool ok = (r.statusCode() == Response::StatusCodeOk);

  if (!ok)
    return QStringList();

  QStringList l(r.data());

  return QStringList();
}

  QStringList
EmpathMailboxIMAP4::listSubscribed(const QString & /* ref */, const QString & /* wild */)
{
  qDebug("%s: STUB", __FUNCTION__);
  if (state_ < Authenticated) {
    qDebug("EmpathMailboxIMAP4::listSubscribed(): state < Authenticated");
    return QStringList();
  }

  return QStringList();
}

  bool
EmpathMailboxIMAP4::status(
    ulong & /* messageCount */,
    ulong & /* recentCount */,
    ulong & /* nextUID */,
    ulong & /* uidValidity */,
    ulong & /* unseenCount */
)
{
  qDebug("%s: STUB", __FUNCTION__);
  if (state_ < Authenticated) {
    qDebug("EmpathMailboxIMAP4::status(): state < Authenticated");
    return false;
  }

  return false;
}

  bool
EmpathMailboxIMAP4::appendMessage(
    const QString & /* mailboxName */,
    ulong           /* flags */,
    const QString & /* messageData */,
    const QString & /* date */
)
{
  qDebug("%s: STUB", __FUNCTION__);
  if (state_ < Authenticated) {
    qDebug("EmpathMailboxIMAP4::appendMessage(): state < Authenticated");
    return false;
  }

  return false;
}

  bool
EmpathMailboxIMAP4::checkpoint()
{
  if (state_ != Selected) {
    qDebug("EmpathMailboxIMAP4::checkpoint(): state != Selected");
    return false;
  }

  Response r = runCommand("CHECKPOINT");

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
EmpathMailboxIMAP4::close()
{
  if (state_ != Selected) {
    qDebug("EmpathMailboxIMAP4::close(): state != Selected");
    return false;
  }

  Response r = runCommand("CLOSE");

  return (r.statusCode() == Response::StatusCodeOk);
}

  bool
EmpathMailboxIMAP4::expunge(QValueList<ulong> & ret)
{
  if (state_ != Selected) {
    qDebug("EmpathMailboxIMAP4::expunge(): state != Selected");
    return false;
  }

  Response r = runCommand("EXPUNGE");

  bool ok = (r.statusCode() == Response::StatusCodeOk);

  if (!ok)
    return false;

  ret.clear();

  QStringList l(r.data());

  QStringList::ConstIterator it(l.begin());

  for (; it != l.end(); ++it) {

    QStringList tokens(QStringList::split(' ', *it));

    if (tokens[0] == '*' && tokens[2] == "EXPUNGE")
      ret << tokens[1].toULong();
  }

  return true;
}

    QValueList<ulong>
EmpathMailboxIMAP4::search(
    const QString & /* spec */,
    const QString & /* charSet */,
    bool            /* usingUID */
)
{
  qDebug("%s: STUB", __FUNCTION__);
  if (state_ != Selected) {
    qDebug("EmpathMailboxIMAP4::search(): state != Selected");
    return QValueList<ulong>();
  }

  return QValueList<ulong>();
}

  QStringList
EmpathMailboxIMAP4::fetch(
    ulong           /* start */,
    ulong           /* end */,
    const QString & /* spec */,
    bool            /* usingUID */
)
{
  qDebug("%s: STUB", __FUNCTION__);
  if (state_ != Selected) {
    qDebug("EmpathMailboxIMAP4::fetch(): state != Selected");
    return QStringList();
  }

  return QStringList();
}

  bool
EmpathMailboxIMAP4::setFlags(
  ulong         /* start */,
  ulong         /* end */,
  FlagSetStyle  /* style */,
  ulong         /* flags */,
  bool          /* usingUID */
)
{
  qDebug("%s: STUB", __FUNCTION__);
// enum FlagSetStyle { Set, Add, Remove };
  if (state_ != Selected) {
    qDebug("EmpathMailboxIMAP4::setFlags(): state != Selected");
    return false;
  }

  return false;
}

  bool
EmpathMailboxIMAP4::copy(ulong start, ulong end, const QString & to)
{
  qDebug("%s: STUB", __FUNCTION__);
  if (state_ != Selected) {
    qDebug("EmpathMailboxIMAP4::copy(): state != Selected");
    return false;
  }

  Response r = runCommand(
      "COPY " + QString::number(start) + ":" + QString::number(end) + " " + to
  );

  return (r.statusCode() == Response::StatusCodeOk);
}

  EmpathMailboxIMAP4::Response
EmpathMailboxIMAP4::runCommand(const QString & cmd)
{
  if (socket_.state() != QSocket::Connection) {
    qDebug("EmpathMailboxIMAP4::runCommand(): Socket is not connected");
    return Response("");
  }

  QString id("EMPATH_" + QString::number(commandCount_));
  QString command(id + " " + cmd + "\r\n");
  socket_.writeBlock(command.ascii(), command.length());
  log("> " + command);
  return Response(response(id));
}

  QStringList
EmpathMailboxIMAP4::response(const QString & endIndicator)
{
  QStringList output;

  int iterations = 0;
  int delay = 1000;
  int timeout = 100;

  while (true) 
  {
    if (!socket_.canReadLine()) {

      if (iterations * delay >= timeout)
        break;

      usleep(delay);
      qApp->processEvents();
      continue;
    }

    QString line = socket_.readLine();
    log("< " + line);
    output << line;

    QStringList tokens(QStringList::split(' ', line));

    if (tokens[0] == endIndicator || !endIndicator)
      return output;
  }

  qDebug("EmpathMailboxIMAP4::reponse(): Timeout");
  return output;
}

QAsciiDict<ulong> * EmpathMailboxIMAP4::Response::statusCodeDict_ = 0L;

EmpathMailboxIMAP4::Response::Response(const QStringList & data)
  : storedData_(data),
    responseType_(ResponseTypeUnknown),
    statusCode_(StatusCodeUnknown)
{
  QString lastLine = storedData_.last();

  QStringList tokens(QStringList::split(' ', lastLine));

  switch (lastLine[0].latin1()) {

    case '*':
      responseType_ = ResponseTypeStatus;
      break;

    case '+':
      responseType_ = ResponseTypeContinuationRequest;
      break;

    default:
      responseType_ = ResponseTypeServerData;
      break;
  }

  statusCode_ = _statusCode(tokens[1]);

  data_ = storedData_;

  if (storedData_.count() > 1)
    data_.remove(data_.fromLast());
}

  void
EmpathMailboxIMAP4::Response::cleanup()
{
  delete statusCodeDict_;
}

  EmpathMailboxIMAP4::Response::ResponseType
EmpathMailboxIMAP4::Response::type() const
{
  return responseType_;
}

  EmpathMailboxIMAP4::Response::StatusCode
EmpathMailboxIMAP4::Response::statusCode() const
{
  return statusCode_;
}

  QStringList
EmpathMailboxIMAP4::Response::data() const
{
  return data_;
}

  QStringList
EmpathMailboxIMAP4::Response::storedData() const
{
  return storedData_;
}

  EmpathMailboxIMAP4::Response::StatusCode
EmpathMailboxIMAP4::Response::_statusCode(const QString & key)
{
  if (0 == statusCodeDict_) {

    statusCodeDict_ = new QAsciiDict<ulong>(23);

    statusCodeDict_->insert("ALERT",          new ulong(StatusCodeAlert));
    statusCodeDict_->insert("NEWNAME",        new ulong(StatusCodeNewName));
    statusCodeDict_->insert("PARSE",          new ulong(StatusCodeParse));
    statusCodeDict_->insert("PERMANENTFLAGS", new ulong(StatusCodePermanentFlags));
    statusCodeDict_->insert("READ-ONLY",      new ulong(StatusCodeReadOnly));
    statusCodeDict_->insert("READ-WRITE",     new ulong(StatusCodeReadWrite));
    statusCodeDict_->insert("TRYCREATE",      new ulong(StatusCodeTryCreate));
    statusCodeDict_->insert("UIDVALIDITY",    new ulong(StatusCodeUIDValidity));
    statusCodeDict_->insert("UNSEEN",         new ulong(StatusCodeUnseen));
    statusCodeDict_->insert("OK",             new ulong(StatusCodeOk));
    statusCodeDict_->insert("NO",             new ulong(StatusCodeNo));
    statusCodeDict_->insert("BAD",            new ulong(StatusCodeBad));
    statusCodeDict_->insert("PREAUTH",        new ulong(StatusCodePreAuth));
    statusCodeDict_->insert("CAPABILITY",     new ulong(StatusCodeCapability));
    statusCodeDict_->insert("LIST",           new ulong(StatusCodeList));
    statusCodeDict_->insert("LSUB",           new ulong(StatusCodeLsub));
    statusCodeDict_->insert("STATUS",         new ulong(StatusCodeStatus));
    statusCodeDict_->insert("SEARCH",         new ulong(StatusCodeSearch));
    statusCodeDict_->insert("FLAGS",          new ulong(StatusCodeFlags));
    statusCodeDict_->insert("EXISTS",         new ulong(StatusCodeExists));
    statusCodeDict_->insert("RECENT",         new ulong(StatusCodeRecent));
    statusCodeDict_->insert("EXPUNGE",        new ulong(StatusCodeExpunge));
    statusCodeDict_->insert("FETCH",          new ulong(StatusCodeFetch));
  }

  if (!key)
    return StatusCodeUnknown;

  ulong * l = statusCodeDict_->find(key.ascii());

  if (0 == l)
    return StatusCodeUnknown;
  else
    return StatusCode(*l);
}
