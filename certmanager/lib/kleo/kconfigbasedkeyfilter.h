/*
    kconfigbasedkeyfilter.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#ifndef __KLEO_KCONFIGBASEDKEYFILTER_H__
#define __KLEO_KCONFIGBASEDKEYFILTER_H__

#include "keyfilter.h"

#include <qfont.h>
#include <qstring.h>
#include <qcolor.h>

#include <gpgmepp/key.h>

class KConfigBase;

namespace Kleo {

  class KConfigBasedKeyFilter : public KeyFilter {
  public:
    explicit KConfigBasedKeyFilter( const KConfigBase & config );
    ~KConfigBasedKeyFilter();
    bool matches( const GpgME::Key & key ) const;

    unsigned int specificity() const { return mSpecificity; }

    QColor fgColor() const { return mFgColor; }
    QColor bgColor() const { return mBgColor; }
    QFont  font( const QFont & ) const;
    QString name() const { return mName; }
    QString icon() const { return mIcon; }

  private:
    QColor mFgColor, mBgColor;
    QString mName;
    QString mIcon;
    unsigned int mSpecificity;
    bool mItalic : 1;
    bool mBold : 1;
    bool mStrikeOut : 1;
    bool mUseFullFont : 1;
    QFont mFont;

    enum TriState {
      DoesNotMatter = 0,
      Set = 1,
      NotSet = 2
    };
    TriState mRevoked : 2;
    TriState mExpired : 2;
    TriState mDisabled : 2;
    TriState mCanEncrypt : 2;
    TriState mCanSign : 2;
    TriState mCanCertify : 2;
    TriState mCanAuthenticate : 2;
    TriState mHasSecret : 2;
    TriState mIsOpenPGP : 2;
    TriState mWasValidated : 2;
    enum LevelState {
      LevelDoesNotMatter = 0,
      Is = 1,
      IsNot = 2,
      IsAtLeast = 3,
      IsAtMost = 4,
    };
    LevelState mOwnerTrust : 3;
    GpgME::Key::OwnerTrust mOwnerTrustReferenceLevel;
    LevelState mValidity : 3;
    GpgME::UserID::Validity mValidityReferenceLevel;
  };

}

#endif // __KLEO_KCONFIGBASEDKEYFILTER_H__
