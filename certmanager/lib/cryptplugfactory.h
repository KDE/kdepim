/*
    cryptplugfactory.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klar�lvdalens Datakonsult AB

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

#ifndef __KLEO_CRYPTPLUGFACTORY_H__
#define __KLEO_CRYPTPLUGFACTORY_H__

#include <qobject.h>
#include <qvaluevector.h>

#include <kleo/cryptobackend.h>

namespace Kleo {
  class BackendConfigWidget;
}

class QWidget;

namespace Kleo {

  class CryptPlugFactory : public QObject {
    Q_OBJECT
  protected:
    CryptPlugFactory();
    ~CryptPlugFactory();
  public:
    static CryptPlugFactory * instance();

    const CryptoBackend::Protocol * smime() const;
    const CryptoBackend::Protocol * openpgp() const;
    CryptoConfig * config() const;

    const CryptoBackend * backend( unsigned int idx ) const;

    bool hasBackends() const;

    Kleo::BackendConfigWidget * configWidget( QWidget * parent=0, const char * name=0 ) const;

    void scanForBackends( QStringList * reasons=0 );

  protected:
    QValueVector<CryptoBackend*> mBackendList;

  private:
    CryptPlugFactory( const CryptPlugFactory & );
    void operator=( const CryptPlugFactory & );

    static CryptPlugFactory * mSelf;
  };

} // namespace Kleo

#ifndef LIBKLEOPATRA_NO_COMPAT
class CryptPlugWrapper;
class CryptPlugWrapperList;

namespace KMail {

  class CryptPlugFactory : public Kleo::CryptPlugFactory {
    Q_OBJECT
  protected:
    CryptPlugFactory();
    ~CryptPlugFactory();

  public:
    static CryptPlugFactory * instance();

    CryptPlugWrapper * active() const;
    CryptPlugWrapper * smime() const;
    CryptPlugWrapper * openpgp() const;

    CryptPlugWrapperList & list() const { return *mCryptPlugWrapperList; }

    CryptPlugWrapper * createForProtocol( const QString & proto ) const;

    void scanForBackends( QStringList * reason );

  private:
    void updateCryptPlugWrapperList();

  private:
    CryptPlugFactory( const CryptPlugFactory & );
    void operator=( const CryptPlugFactory & );
    CryptPlugWrapperList * mCryptPlugWrapperList;

    static CryptPlugFactory * mSelf;
  };

}
#endif

#endif // __KLEO_CRYPTPLUGFACTORY_H__
