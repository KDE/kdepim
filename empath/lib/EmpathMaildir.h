/*
    Empath - Mailer for KDE

    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>

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

#ifndef EMPATH_MAILDIR_H
#define EMPATH_MAILDIR_H 

// Qt includes
#include <qdir.h>
#include <qdict.h>
#include <qstring.h>
#include <qdir.h>
#include <qlist.h>
#include <qtimer.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qmap.h>

// Local includes
#include "EmpathURL.h"
#include "EmpathDefines.h"
#include "EmpathIndexRecord.h"
#include "rmm/Message.h"

class EmpathFolder;
class EmpathIndex;

/**
 * @internal
 * @author Rikkus
 */
class EmpathMaildir : public QObject
{
    Q_OBJECT

    public:

        EmpathMaildir(const QString & basePath, const EmpathURL & url);

        bool createdOK() const { return createdOK_; }

        virtual ~EmpathMaildir();

        void init();

        const QString & basePath() const    { return basePath_; }
        const EmpathURL & url() const       { return url_; }
        const QString & path() const        { return path_; }

        EmpathSuccessMap mark(const QStringList &, EmpathIndexRecord::Status);

        QString writeMessage(RMM::Message);

        RMM::Message message(const QString &);

        EmpathSuccessMap removeMessage (const QStringList &);

        void sync();

        EmpathIndex * index();

    protected slots:

        void s_timerBeeped();

    private:

        EmpathMaildir();

        QString _cur() { return path_ + "/cur/"; }
        QString _new() { return path_ + "/new/"; }
        QString _tmp() { return path_ + "/tmp/"; }

        EmpathIndex * index_;

        void        _markNewMailAsSeen();
        void        _tagAsDisappearedOrAddToIndex();
        void        _removeDisappeared();

        bool        _mark(const QString & id, EmpathIndexRecord::Status);
        void        _markAsSeen(const QString &);

        bool        _removeMessage(const QString & id);
        QString     _write(RMM::Message);
        QCString    _messageData(const QString &, bool isFullName = false);
        QString     _generateFlagsString(EmpathIndexRecord::Status);

        void        _clearTmp();

        bool        _checkDirs();
        bool        _touched();

        QDateTime    mtime_;
        QDict<bool>  disappeared_;

        QTimer       timer_;

        QStringList cachedEntryList_;
        QStringList & _entryList();

        bool createdOK_;

        // Order dependency
        QString      path_;
        EmpathURL    url_;
        QString      basePath_;
        // End order dependency
};

typedef QList<EmpathMaildir> EmpathMaildirList;
typedef QListIterator<EmpathMaildir> EmpathMaildirListIterator;

#endif

// vim:ts=4:sw=4:tw=78
