/*  -*- c++ -*-
    header/headerstrategy.h

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

#ifndef HEADERSTRATEGY_P_H
#define HEADERSTRATEGY_P_H
#include "header/headerstrategy.h"
#include <QStringList>
//
// AllHeaderStrategy:
//   show everything
//
//
// Header tables:
//
namespace MessageViewer
{
static const char *const briefHeaders[] = {
    "subject", "from", "cc", "bcc", "date"
};
static const int numBriefHeaders = sizeof briefHeaders / sizeof *briefHeaders;

static const char *const standardHeaders[] = {
    "subject", "from", "cc", "bcc", "to"
};
static const int numStandardHeaders = sizeof standardHeaders / sizeof *standardHeaders;

static const char *const richHeaders[] = {
    "subject", "date", "from", "cc", "bcc", "to",
    "organization", "organisation", "reply-to",
    "user-agent", "x-mailer", "x-bugzilla-url"
};
static const int numRichHeaders = sizeof richHeaders / sizeof *richHeaders;

//
// Convenience function
//

static QStringList stringList(const char *const headers[], int numHeaders)
{
    QStringList sl;
    for (int i = 0 ; i < numHeaders ; ++i) {
        sl.push_back(QLatin1String(headers[i]));
    }
    return sl;
}

class AllHeaderStrategy : public HeaderStrategy
{
    friend class HeaderStrategy;
protected:
    AllHeaderStrategy() : HeaderStrategy() {}
    ~AllHeaderStrategy() {}

public:
    const char *name() const
    {
        return "all";
    }

    DefaultPolicy defaultPolicy() const
    {
        return Display;
    }

    bool showHeader(const QString &) const
    {
        return true; // more efficient than default impl
    }
};

//
// RichHeaderStrategy:
//   Date, Subject, From, To, CC, ### what exactly?
//

class RichHeaderStrategy : public HeaderStrategy
{
    friend class HeaderStrategy;
protected:
    RichHeaderStrategy()
        : HeaderStrategy(),
          mHeadersToDisplay(stringList(richHeaders, numRichHeaders)) {}
    ~RichHeaderStrategy() {}

public:
    const char *name() const
    {
        return "rich";
    }

    QStringList headersToDisplay() const
    {
        return mHeadersToDisplay;
    }
    DefaultPolicy defaultPolicy() const
    {
        return Hide;
    }

private:
    const QStringList mHeadersToDisplay;
};

//
// StandardHeaderStrategy:
//   BCC, CC, Date, From, Subject, To
//

class StandardHeaderStrategy : public HeaderStrategy
{
    friend class HeaderStrategy;
protected:
    StandardHeaderStrategy()
        : HeaderStrategy(),
          mHeadersToDisplay(stringList(standardHeaders, numStandardHeaders)) {}
    ~StandardHeaderStrategy() {}

public:
    const char *name() const
    {
        return "standard";
    }

    QStringList headersToDisplay() const
    {
        return mHeadersToDisplay;
    }
    DefaultPolicy defaultPolicy() const
    {
        return Hide;
    }

private:
    const QStringList mHeadersToDisplay;
};

//
// BriefHeaderStrategy
//   From, Subject, Date
//

class BriefHeaderStrategy : public HeaderStrategy
{
    friend class HeaderStrategy;
protected:
    BriefHeaderStrategy()
        : HeaderStrategy(),
          mHeadersToDisplay(stringList(briefHeaders, numBriefHeaders)) {}
    ~BriefHeaderStrategy() {}

public:
    const char *name() const
    {
        return "brief";
    }

    QStringList headersToDisplay() const
    {
        return mHeadersToDisplay;
    }
    DefaultPolicy defaultPolicy() const
    {
        return Hide;
    }

private:
    const QStringList mHeadersToDisplay;
};

//
// CustomHeaderStrategy
//   Determined by user
//

class CustomHeaderStrategy : public HeaderStrategy
{
    friend class HeaderStrategy;
protected:
    CustomHeaderStrategy();
    ~CustomHeaderStrategy() {}

public:
    const char *name() const
    {
        return "custom";
    }

    void loadConfig();
    QStringList headersToDisplay() const
    {
        return mHeadersToDisplay;
    }
    QStringList headersToHide() const
    {
        return mHeadersToHide;
    }
    DefaultPolicy defaultPolicy() const
    {
        return mDefaultPolicy;
    }

private:
    QStringList mHeadersToDisplay;
    QStringList mHeadersToHide;
    DefaultPolicy mDefaultPolicy;
};

//
// CustomHeaderStrategy
//   Determined by grantlee theme.
//
class GrantleeHeaderStrategy : public HeaderStrategy
{
    friend class HeaderStrategy;
protected:
    GrantleeHeaderStrategy() {}
    ~GrantleeHeaderStrategy() {}

public:
    const char *name() const
    {
        return "grantlee";
    }
    DefaultPolicy defaultPolicy() const
    {
        return Hide;
    }
};

}

#endif // HEADERSTRATEGY_P_H
