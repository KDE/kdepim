#ifndef EMPATH_MAILBOX_IMAP4_H
#define EMPATH_MAILBOX_IMAP4_H

// Qt includes
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qsocket.h>
#include <qasciidict.h>

class EmpathMailboxIMAP4
{
  public:

    class MailboxInfo
    {
      public:

        MailboxInfo()
          : count_(0),
            recent_(0),
            unseen_(0),
            uidValidity_(0),
            flags_(0),
            permanentFlags_(0),
            readWrite_(false),
            countAvailable_(false),
            recentAvailable_(false),
            unseenAvailable_(false),
            uidValidityAvailable_(false),
            flagsAvailable_(false),
            permanentFlagsAvailable_(false),
            readWriteAvailable_(false)
        {
        }

        void setCount(ulong l)
        { countAvailable_ = true; count_ = l; }

        void setRecent(ulong l)
        { recentAvailable_ = true; recent_ = l; }

        void setUnseen(ulong l)
        { unseenAvailable_ = true; unseen_ = l; }

        void setUidValidity(ulong l)
        { uidValidityAvailable_ = true; uidValidity_ = l; }

        void setFlags(ulong l)
        { flagsAvailable_ = true; flags_ = l; }

        void setPermanentFlags(ulong l)
        { permanentFlagsAvailable_ = true; permanentFlags_ = l; }

        void setReadWrite(bool b)
        { readWriteAvailable_ = true; readWrite_ = b; }

        ulong count() const
        { return count_; }

        ulong recent() const
        { return recent_; }

        ulong unseen() const
        { return unseen_; }

        ulong uidValidity() const
        { return uidValidity_; }

        ulong flags() const
        { return flags_; }

        ulong permanentFlags() const
        { return permanentFlags_; }

        bool readWrite() const
        { return readWrite_; }

        ulong countAvailable() const
        { return countAvailable_; }

        ulong recentAvailable() const
        { return recentAvailable_; }

        ulong unseenAvailable() const
        { return unseenAvailable_; }

        ulong uidValidityAvailable() const
        { return uidValidityAvailable_; }

        ulong flagsAvailable() const
        { return flagsAvailable_; }

        ulong permanentFlagsAvailable() const
        { return permanentFlagsAvailable_; }

        bool readWriteAvailable() const
        { return readWriteAvailable_; }

      private:

        ulong count_;
        ulong recent_;
        ulong unseen_;
        ulong uidValidity_;
        ulong flags_;
        ulong permanentFlags_;
        bool  readWrite_;

        bool countAvailable_;
        bool recentAvailable_;
        bool unseenAvailable_;
        bool uidValidityAvailable_;
        bool flagsAvailable_;
        bool permanentFlagsAvailable_;
        bool readWriteAvailable_;
    };
 
    enum MessageAttribute
    {
      Seen      = 1 << 0,
      Answered  = 1 << 1,
      Flagged   = 1 << 2,
      Deleted   = 1 << 3,
      Draft     = 1 << 4,
      Recent    = 1 << 5
    };

    enum State
    {
      Closed,
      Init,
      Logout,
      NotAuthenticated,
      Authenticated,
      Selected
    };

    EmpathMailboxIMAP4();
    ~EmpathMailboxIMAP4();

    bool connect();
    void disconnect();

    void setServerDetails(const QString & hostname, uint port);
    void setLoginDetails(const QString & username, const QString & password);

    // Any state
    QString capability();
    bool noop();
    bool logout();

    // Not authenticated
    bool authenticate();
    bool login();

    // Authenticated
    bool selectMailbox(const QString & name, MailboxInfo & ret);
    bool examineMailbox(const QString & name, MailboxInfo & ret);
    bool createMailbox(const QString & name);
    bool removeMailbox(const QString & name);
    bool renameMailbox(const QString & from, const QString & to);
    bool subscribeMailbox(const QString & name);
    bool unsubscribeMailbox(const QString & name);
    QStringList list(const QString & ref, const QString & wild);
    QStringList listSubscribed(const QString & ref, const QString & wild);

    bool status(
        ulong & messageCount,
        ulong & recentCount,
        ulong & nextUID,
        ulong & uidValidity,
        ulong & unseenCount
    );

    bool appendMessage(
        const QString & mailboxName,
        ulong flags,
        const QString & messageData,
        const QString & date = ""
    );

    // Selected
    bool checkpoint();

    bool close();

    bool expunge(QValueList<ulong> & ret);

    QValueList<ulong> search(
        const QString & spec,
        const QString & charSet = "",
        bool usingUID = false
    );

    QStringList fetch(
        ulong start,
        ulong end,
        const QString & spec,
        bool usingUID = false
    );

    enum FlagSetStyle { Set, Add, Remove };

    bool setFlags(
        ulong start,
        ulong end,
        FlagSetStyle style,
        ulong flags,
        bool usingUID = false
    );

    bool copy(ulong start, ulong end, const QString & to);

  protected:

    class Response {

      public:

        enum ResponseType
        {
          ResponseTypeUnknown,
          ResponseTypeStatus,
          ResponseTypeContinuationRequest,
          ResponseTypeServerData
        };

        Response(const QStringList & data);

        ResponseType type() const;

        static void cleanup();

        enum StatusCode
        {
          StatusCodeUnknown,
          StatusCodeAlert,
          StatusCodeNewName,
          StatusCodeParse,
          StatusCodePermanentFlags,
          StatusCodeReadOnly,
          StatusCodeReadWrite,
          StatusCodeTryCreate,
          StatusCodeUIDValidity,
          StatusCodeUnseen,
          StatusCodeOk,
          StatusCodeNo,
          StatusCodeBad,
          StatusCodePreAuth,
          StatusCodeBye,
          StatusCodeCapability,
          StatusCodeList,
          StatusCodeLsub,
          StatusCodeStatus,
          StatusCodeSearch,
          StatusCodeFlags,
          StatusCodeExists,
          StatusCodeRecent,
          StatusCodeExpunge,
          StatusCodeFetch
        };

        StatusCode statusCode() const;

        QStringList data() const;

        QStringList storedData() const;

      private:

        StatusCode _statusCode(const QString & key);

        QStringList data_;
        static QAsciiDict<ulong> * statusCodeDict_;
        
        // Order dependency
        QStringList storedData_;
        ResponseType responseType_;
        StatusCode statusCode_;
        // End order dependency
    };

    Response runCommand(const QString & cmd);
    QStringList response(const QString & endIndicator);

    ulong _flags(const QString &);

  private:

    void _setUsername(const QString &);
    void _setPassword(const QString &);

    QString _username() const;
    QString _password() const;

    QString hostname_;
    QString username_;
    QString password_;

    QSocket socket_;

    // Order dependency
    State state_;
    ulong commandCount_;
    uint port_;
    // End order dependency
};

#endif // Included this file
