/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef MESSAGECOMPOSER_TEXTPART_H
#define MESSAGECOMPOSER_TEXTPART_H

#include <QtCore/QString>

#include "messagecomposer_export.h"
#include "messagepart.h"

#include <KPIMTextEdit/textedit.h>

namespace MessageComposer {

class MESSAGECOMPOSER_EXPORT TextPart : public MessagePart
{
    Q_OBJECT

public:
    explicit TextPart( QObject *parent = 0 );
    virtual ~TextPart();

    // default true
    bool isWordWrappingEnabled() const;
    void setWordWrappingEnabled( bool enabled );
    // default true
    bool warnBadCharset() const;
    void setWarnBadCharset( bool warn );

    QString cleanPlainText() const;
    void setCleanPlainText( const QString &text );
    QString wrappedPlainText() const;
    void setWrappedPlainText( const QString &text );

    bool isHtmlUsed() const;
    QString cleanHtml() const;
    void setCleanHtml( const QString &text );

    bool hasEmbeddedImages() const;
    KPIMTextEdit::ImageList embeddedImages() const;
    void setEmbeddedImages( const KPIMTextEdit::ImageList &images );

private:
    class Private;
    Private *const d;
};

} // namespace MessageComposer

#endif // MESSAGECOMPOSER_TEXTPART_H
