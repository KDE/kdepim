#ifndef EMPATH_INTERFACE_H
#define EMPATH_INTERFACE_H

#include <dcopobject.h>

#include <qcstring.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdict.h>

class EmpathInterface : virtual public DCOPObject
{
    K_DCOP

    public:
        
        EmpathInterface();

    k_dcop:

        virtual QString inboxURL() = 0;
        virtual QString outboxURL() = 0;
        virtual QString sentURL() = 0;
        virtual QString draftsURL() = 0;
        virtual QString trashURL() = 0;
        virtual QByteArray message(QString url) = 0;
        virtual void queue(QByteArray message) = 0;
        virtual void send(QByteArray message) = 0;
        virtual void sendQueued() = 0;
        virtual void checkMail() = 0;
        virtual void compose(QString recipient) = 0;
        virtual void reply(QString url) = 0;
        virtual void replyAll(QString url) = 0;
        virtual void forward(QString url) = 0;
        virtual void bounce(QString url) = 0;
        virtual void createFolder(QString path) = 0;
        virtual void removeFolder(QString path) = 0;
        virtual void copy(QString urlFrom, QString urlTo) = 0;
        virtual void move(QString urlFrom, QString urlTo) = 0;
        virtual void retrieve(QString urlFrom) = 0;
        virtual void write(QString urlFolder, QByteArray message) = 0;
        virtual void remove(QString urlID) = 0;
        virtual void remove(QString urlFolder, QStringList idList) = 0;
        virtual void mark(QString urlID, int status) = 0;
        virtual void mark(QString urlFolder, QStringList idList, int status) = 0;

    private:

        QDict<int> fnDict_;
};

#endif // Included this file.
// vim:ts=4:sw=4:tw=78
