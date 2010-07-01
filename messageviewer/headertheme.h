/*  -*- c++ -*-
    headertheme.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2010 Ronny Yabar Aizcorbe

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

class HeaderStrategy;
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

  static QString dateStr(const KDateTime &dateTime);
  static QByteArray dateShortStr(const KDateTime &dateTime);

  // method to set up the theme in the header Styles
  QString setTheming( const QString &styleName , KMime::Message *message ) const;

  static QString imgToDataUrl( const QImage & image );
  static QString drawSpamMeter( SpamError spamError, double percent, double confidence,
  const QString & filterHeader, const QString & confidenceHeader );

private:

  QString mMessagePath;
  const HeaderStrategy *mStrategy;
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