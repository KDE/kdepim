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

#ifndef __KLEO_KCONFIGBASEDKEYFILTER_H__
#define __KLEO_KCONFIGBASEDKEYFILTER_H__

#include "keyfilter.h"

#include <QFont>
#include <QString>
#include <QColor>

#include <gpgme++/key.h>

class KConfigGroup;

namespace Kleo {

  class KeyFilterImplBase : public KeyFilter {
  public:
    KeyFilterImplBase();
    ~KeyFilterImplBase();

    bool matches( const GpgME::Key & key, MatchContexts ctx ) const;

    unsigned int specificity() const { return mSpecificity; }
    QString id() const { return mId; }
    MatchContexts availableMatchContexts() const { return mMatchContexts; }

    QColor fgColor() const { return mFgColor; }
    QColor bgColor() const { return mBgColor; }
    FontDescription  fontDesription() const;
    QString name() const { return mName; }
    QString icon() const { return mIcon; }

  protected:
    QColor mFgColor, mBgColor;
    QString mName;
    QString mIcon;
    QString mId;
    MatchContexts mMatchContexts;
    unsigned int mSpecificity;
    bool mItalic;
    bool mBold;
    bool mStrikeOut;
    bool mUseFullFont;
    QFont mFont;

    enum TriState {
      DoesNotMatter = 0,
      Set = 1,
      NotSet = 2
    };
    TriState mRevoked;
    TriState mExpired;
    TriState mDisabled;
    TriState mRoot;
    TriState mCanEncrypt;
    TriState mCanSign;
    TriState mCanCertify;
    TriState mCanAuthenticate;
    TriState mQualified;
    TriState mCardKey;
    TriState mHasSecret;
    TriState mIsOpenPGP;
    TriState mWasValidated;
    enum LevelState {
      LevelDoesNotMatter = 0,
      Is = 1,
      IsNot = 2,
      IsAtLeast = 3,
      IsAtMost = 4
    };
    LevelState mOwnerTrust;
    GpgME::Key::OwnerTrust mOwnerTrustReferenceLevel;
    LevelState mValidity;
    GpgME::UserID::Validity mValidityReferenceLevel;
  };

  class KConfigBasedKeyFilter : public KeyFilterImplBase {
  public:
     explicit KConfigBasedKeyFilter( const KConfigGroup & group );
  };

}

#endif // __KLEO_KCONFIGBASEDKEYFILTER_H__
