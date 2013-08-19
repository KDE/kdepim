/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef GRANTLEEHEADERTESTSTYLE_H
#define GRANTLEEHEADERTESTSTYLE_H

#include "header/headerstyle.h"
#include "messageviewer_export.h"

namespace MessageViewer {
class GrantleeHeaderFormatter;
class MESSAGEVIEWER_EXPORT GrantleeHeaderTestStyle : public HeaderStyle {
    friend class GrantleeHeaderStyle;
public:
    GrantleeHeaderTestStyle();
    ~GrantleeHeaderTestStyle();

public:
    const char * name() const { return "grantleetest"; }

    QString format(KMime::Message *message) const;

    void setAbsolutePath(const QString &);
    void setMainFilename(const QString &);
    void setExtraDisplayHeaders(const QStringList &);

    bool hasAttachmentQuickList() const {
        return true;
    }

private:
    QStringList mExtraDisplay;
    QString mAbsolutePath;
    QString mMainFilename;
    GrantleeHeaderFormatter *mGrantleeFormatter;
};
}

#endif // GRANTLEEHEADERTESTSTYLE_H
