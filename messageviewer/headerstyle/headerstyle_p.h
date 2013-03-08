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


#ifndef HEADERSTYLE_P_H
#define HEADERSTYLE_P_H
#include "headerstyle.h"
#include "spamheaderanalyzer.h"

namespace MessageViewer {

class BriefHeaderStyle : public HeaderStyle {
  friend class HeaderStyle;
protected:
  BriefHeaderStyle() : HeaderStyle() {}
  virtual ~BriefHeaderStyle() {}

public:
  const char * name() const { return "brief"; }
  HeaderStyle * next() const { return plain(); }
  HeaderStyle * prev() const { return fancy(); }

  QString format( KMime::Message *message ) const;
};

class PlainHeaderStyle : public HeaderStyle {
  friend class HeaderStyle;
protected:
  PlainHeaderStyle() : HeaderStyle() {}
  virtual ~PlainHeaderStyle() {}

public:
  const char * name() const { return "plain"; }
  HeaderStyle * next() const { return fancy(); }
  HeaderStyle * prev() const { return brief(); }

  QString format( KMime::Message *message ) const;

private:
  QString formatAllMessageHeaders( KMime::Message *message ) const;
};

class FancyHeaderStyle : public HeaderStyle {
  friend class HeaderStyle;
protected:
  FancyHeaderStyle() : HeaderStyle() {}
  virtual ~FancyHeaderStyle() {}

public:
  const char * name() const { return "fancy"; }
  HeaderStyle * next() const { return enterprise(); }
  HeaderStyle * prev() const { return plain(); }

  QString format( KMime::Message *message ) const;

  virtual bool hasAttachmentQuickList() const {
    return true;
  }

  static QString imgToDataUrl( const QImage & image );

private:
  static QString drawSpamMeter( SpamError spamError, double percent, double confidence,
  const QString & filterHeader, const QString & confidenceHeader );
};

class EnterpriseHeaderStyle : public HeaderStyle {
  friend class HeaderStyle;
protected:
  EnterpriseHeaderStyle() : HeaderStyle() {}
  virtual ~EnterpriseHeaderStyle() {}

public:
  const char * name() const { return "enterprise"; }
  HeaderStyle * next() const {
#if defined KDEPIM_MOBILE_UI
    return mobile();
#else
    return brief();
#endif
  }
  HeaderStyle * prev() const { return fancy(); }

  QString format( KMime::Message *message ) const;

  virtual bool hasAttachmentQuickList() const {
    return true;
  }
};

class MobileHeaderStyle : public HeaderStyle {
  friend class HeaderStyle;
protected:
  MobileHeaderStyle() : HeaderStyle() {}
  virtual ~MobileHeaderStyle() {}

public:
  const char * name() const { return "mobile"; }
  HeaderStyle * next() const { return mobileExtended(); }
  HeaderStyle * prev() const { return enterprise(); }

  QString format( KMime::Message *message ) const;
};

class MobileExtendedHeaderStyle : public HeaderStyle {
  friend class HeaderStyle;
protected:
  MobileExtendedHeaderStyle() : HeaderStyle() {}
  virtual ~MobileExtendedHeaderStyle() {}

public:
  const char * name() const { return "mobileExtended"; }
  HeaderStyle * next() const { return brief(); }
  HeaderStyle * prev() const { return mobile(); }

  QString format( KMime::Message *message ) const;
};

class CustomHeaderStyle : public HeaderStyle {
  friend class HeaderStyle;
protected:
  CustomHeaderStyle() : HeaderStyle() {}
  virtual ~CustomHeaderStyle() {}

public:
  const char * name() const { return "custom"; }
  HeaderStyle * next() const { return fancy(); }
  HeaderStyle * prev() const { return brief(); }

  QString format( KMime::Message *message ) const;

private:
  QString formatAllMessageHeaders(KMime::Message *message , const QStringList &headersToHide) const;
};

class GrantleeHeaderStyle : public HeaderStyle {
  friend class HeaderStyle;
protected:
  GrantleeHeaderStyle() : HeaderStyle() {}
  virtual ~GrantleeHeaderStyle() {}

public:
  const char * name() const { return "grantlee"; }
  HeaderStyle * next() const { return fancy(); }
  HeaderStyle * prev() const { return brief(); }

  QString format( KMime::Message *message ) const;

private:
  QString formatAllMessageHeaders(KMime::Message *message , const QStringList &headersToHide) const;
};


}
#endif // HEADERSTYLE_P_H
