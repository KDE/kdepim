/*  -*- c++ -*-
    headerstyle.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

    Copyright (c) 2013 Laurent Montel <montel@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __MESSAGEVIEWER_HEADERSTYLE_H__
#define __MESSAGEVIEWER_HEADERSTYLE_H__

#include "messageviewer_export.h"

#include "grantleetheme/grantleetheme.h"

#include <Akonadi/KMime/messagestatus.h>
#include <KMime/Message>

class QString;

namespace MessageViewer {

class HeaderStrategy;
class NodeHelper;

/** This class encapsulates the visual appearance of message
    headers. Together with HeaderStrategy, which determines
    which of the headers present in the message be shown, it is
    responsible for the formatting of message headers.

    @short Encapsulates visual appearance of message headers.
    @author Marc Mutz <mutz@kde.org>
    @see HeaderStrategy
**/
class MESSAGEVIEWER_EXPORT HeaderStyle {
protected:
    HeaderStyle();
    virtual ~HeaderStyle();

public:
    //
    // Factory methods:
    //
    enum Type {
        Brief,
        Plain,
        Fancy,
        Enterprise,
        Mobile,
        MobileExtended,
        Custom,
        Grantlee
    };

    static HeaderStyle * create( Type type );
    static HeaderStyle * create( const QString & type );

    static HeaderStyle * brief();
    static HeaderStyle * plain();
    static HeaderStyle * fancy();
    static HeaderStyle * enterprise();
    static HeaderStyle * mobile();
    static HeaderStyle * mobileExtended();
    static HeaderStyle * custom();
    static HeaderStyle * grantlee();

    //
    // Methods for handling the styles:
    //
    virtual const char * name() const = 0;

    //
    // HeaderStyle interface:
    //
    virtual QString format( KMime::Message *message ) const = 0;

    // Should return true if this style has an attachment quick list
    virtual bool hasAttachmentQuickList() const;

    void setMessagePath( const QString &path ) { mMessagePath = path; }
    QString messagePath() const { return mMessagePath; }

    void setHeaderStrategy( const HeaderStrategy *strategy ) { mStrategy = strategy; }
    const HeaderStrategy* headerStrategy() const { return mStrategy; }

    void setVCardName( const QString &vCardName ) { mVCardName = vCardName; }
    QString vCardName() const { return mVCardName; }

    void setPrinting( bool printing ) { mPrinting = printing; }
    bool isPrinting() const { return mPrinting; }

    void setTopLevel( bool topLevel ) { mTopLevel = topLevel; }
    bool isTopLevel() const { return mTopLevel; }

    void setNodeHelper( NodeHelper *nodeHelper ) { mNodeHelper = nodeHelper; }
    NodeHelper* nodeHelper() const { return mNodeHelper; }

    void setAllowAsync( bool allowAsync ) { mAllowAsync = allowAsync; }
    bool allowAsync() const { return mAllowAsync; }

    void setSourceObject( QObject *sourceObject ) { mSourceObject = sourceObject; }
    QObject* sourceObject() const { return mSourceObject; }

    void setMessageStatus( const Akonadi::MessageStatus &status ) { mMessageStatus = status; }
    Akonadi::MessageStatus messageStatus() const { return mMessageStatus; }

    void setTheme(const GrantleeTheme::Theme &theme) { mTheme = theme; }
    GrantleeTheme::Theme theme() const { return mTheme; }

private:
    GrantleeTheme::Theme mTheme;
    QString mMessagePath;
    const HeaderStrategy *mStrategy;
    QString mVCardName;
    bool mPrinting;
    bool mTopLevel;
    NodeHelper *mNodeHelper;
    bool mAllowAsync;
    QObject *mSourceObject;
    Akonadi::MessageStatus mMessageStatus;
};
}

#endif // __MESSAGEVIEWER_HEADERSTYLE_H__
