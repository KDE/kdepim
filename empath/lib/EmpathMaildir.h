/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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

#ifdef __GNUG__
# pragma interface "EmpathMaildir.h"
#endif

#ifndef EMPATH_MAILDIR_H
#define EMPATH_MAILDIR_H 

// Qt includes
#include <qdir.h>
#include <qstring.h>
#include <qdir.h>
#include <qlist.h>
#include <qtimer.h>
#include <qobject.h>
#include <qstringlist.h>

// Local includes
#include "EmpathURL.h"
#include "EmpathDefines.h"
#include "RMM_Enum.h"
#include "RMM_Envelope.h"
#include "RMM_Message.h"
#include "RMM_MessageID.h"

class EmpathFolder;

/**
 * @internal
 * @author Rikkus
 */
class EmpathMaildir : public QObject
{
    Q_OBJECT
        
    public:
        
        EmpathMaildir()
            :    QObject()
        {
            empathDebug("default ctor");
        }

        EmpathMaildir(const QString & basePath, const EmpathURL & url);

        virtual ~EmpathMaildir();
        
        void init();
        
        const QString &        basePath()    const { return basePath_; }
        const EmpathURL &    url()        const { return url_; }
        const QString &        path()        const { return path_; }
        
        bool mark(const QString &, RMM::MessageStatus);
        bool mark(const QStringList &, RMM::MessageStatus);
        
        QString        writeMessage(RMM::RMessage &);
        
        Q_UINT32                    sizeOfMessage        (const QString &);
        QString                        plainBodyOfMessage    (const QString &);
        RMM::REnvelope *            envelopeOfMessage    (const QString &);
        RMM::RMessage *                message                (const QString &);
        
        bool                removeMessage    (const QString &, bool = false);
        bool                removeMessage    (const QStringList &);
        
        RMM::RBodyPart::PartType    typeOfMessage        (const QString &);
        
        void sync(const EmpathURL & url, bool ignoreMtime = false);
        
    protected slots:
        
        void s_timerBeeped();
        
    private:
        
        QString     _write(RMM::RMessage &);
        QCString    _messageData(const QString &);
        void        _markNewMailAsSeen();
        void        _markAsSeen(const QString &);
        void        _clearTmp();
        bool        _setupDirs();
        QString     _generateFlagsString(RMM::MessageStatus);
        void        _readIndex();
        void        _writeIndex();
        
        // Order dependency
        QString      path_;
        EmpathURL    url_;
        QString      basePath_;
        // End order dependency
        
        QDir d;
        
        QDateTime    mtime_;
        
        QTimer       timer_; // Check for modification every so often.
};

typedef QList<EmpathMaildir> EmpathMaildirList;
typedef QListIterator<EmpathMaildir> EmpathMaildirListIterator;

#endif

// vim:ts=4:sw=4:tw=78
