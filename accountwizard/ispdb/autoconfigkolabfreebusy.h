/*
 * Copyright (C) 2014  Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AUTOCONFIGKOLABFREEBUSY_H
#define AUTOCONFIGKOLABFREEBUSY_H

#include "autoconfigkolabmail.h"

struct freebusy;

class AutoconfigKolabFreebusy : public AutoconfigKolabMail
{
public:
    /** Constructor */
    explicit AutoconfigKolabFreebusy(QObject *parent = Q_NULLPTR);

    QHash<QString, freebusy> freebusyServers() const;

protected:
    void lookupInDb(bool auth, bool crypt) Q_DECL_OVERRIDE;
    void parseResult(const QDomDocument &document) Q_DECL_OVERRIDE;

private:
    freebusy createFreebusyServer(const QDomElement &n);

    QHash<QString, freebusy> mFreebusyServer;
};

struct freebusy {
    freebusy()
        : port(80)
        , socketType(Ispdb::None)
        , authentication(Ispdb::Plain)
    {
    }
    bool isValid() const
    {
        return (port != -1);
    }
    QString hostname;
    QString username;
    QString password;
    QString path;
    int port;
    Ispdb::socketType socketType;
    Ispdb::authType authentication;
};

#endif // AUTOCONFIGKOLABFREEBUSY_H
