#ifndef EMPATH_INTERFACE_H
#define EMPATH_INTERFACE_H

#include <dcopobject.h>

#include <qcstring.h>
#include <qstring.h>
#include <qstringlist.h>

class EmpathInterface : virtual public DCOPObject
{
    K_DCOP

    k_dcop:

        virtual QString inboxURL() = 0;
        virtual QString outboxURL() = 0;
        virtual QString sentURL() = 0;
        virtual QString draftsURL() = 0;
        virtual QString trashURL() = 0;
        virtual QByteArray message(QString url) = 0;
        virtual unsigned int queue(QByteArray message) = 0;
        virtual unsigned int send(QByteArray message) = 0;
        virtual unsigned int sendQueued() = 0;
        virtual unsigned int checkMail() = 0;
        virtual unsigned int compose(QString recipient) = 0;
        virtual unsigned int reply(QString url) = 0;
        virtual unsigned int replyAll(QString url) = 0;
        virtual unsigned int forward(QString url) = 0;
        virtual unsigned int bounce(QString url) = 0;
        virtual unsigned int createFolder(QString path) = 0;
        virtual unsigned int removeFolder(QString path) = 0;
        virtual unsigned int copy(QString urlFrom, QString urlTo) = 0;
        virtual unsigned int move(QString urlFrom, QString urlTo) = 0;
        virtual unsigned int retrieve(QString urlFrom) = 0;
        virtual unsigned int write(QByteArray message, QString urlFolder) = 0;
        virtual unsigned int remove(QString urlID) = 0;
        virtual unsigned int remove(QString urlFolder, QStringList idList) = 0;
        virtual unsigned int mark(QString urlID, int status) = 0;
        virtual unsigned int mark(QString urlFolder, QStringList idList, int status) = 0;
};

#endif // Included this file.
// vim:ts=4:sw=4:tw=78
