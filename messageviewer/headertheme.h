/*  -*- c++ -*-
    headertheme.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2010 Ronny Yabar Aizcorbe

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __MESSAGEVIEWER_HEADERTHEME_H__
#define __MESSAGEVIEWER_HEADERTHEME_H__

#include "messageviewer_export.h"
#include "spamheaderanalyzer.h"

#include <KMime/Message>
#include <QStringList>

class QByteArray;
class QString;
class QImage;
class KDateTime;
class SpamScore;

namespace Grantlee
{
    class Engine;
}

namespace MessageViewer {

class NodeHelper;

class MESSAGEVIEWER_EXPORT HeaderTheme {
 
public:

  HeaderTheme();
  virtual ~HeaderTheme();


    static HeaderTheme * create();
  //
  // HeaderTheme interface:
  //
  QString formatAllMessageHeaders( KMime::Message *message ) const;

  void setMessagePath( const QString &path ) { mMessagePath = path; }
  QString messagePath() const { return mMessagePath; }

  void setHeaderTheme( const HeaderTheme *theme ) { mTheme = theme; }

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

  static QString dateStr(const KDateTime &dateTime);
  static QByteArray dateShortStr(const KDateTime &dateTime);

  // method to set up the theme in the header Styles
  QString setTheme( const QString &styleName , KMime::Message *message ) const;

  static QString imgToDataUrl( const QImage & image );

private:

  static QString drawSpamMeter( SpamError spamError, double percent, double confidence,
  const QString & filterHeader, const QString & confidenceHeader );

  QString mMessagePath;
  const HeaderTheme *mTheme;
  QString mVCardName;
  bool mPrinting;
  bool mTopLevel;
  NodeHelper *mNodeHelper;
  bool mAllowAsync;
  QObject *mSourceObject;

  Grantlee::Engine *mEngine;


};
}

#endif // __MESSAGEVIEWER_HEADERTHEME_H__