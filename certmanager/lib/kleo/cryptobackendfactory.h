/*
    kleo/cryptobackendfactory.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004,2005 Klarälvdalens Datakonsult AB

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

#ifndef __KLEO_CRYPTOBACKENDFACTORY_H__
#define __KLEO_CRYPTOBACKENDFACTORY_H__

#include <qobject.h>

#include "cryptobackend.h"

#include <vector>
#include <map>

namespace Kleo {
  class BackendConfigWidget;
}

class QString;
class KConfig;

namespace Kleo {

  struct lt_i_str {
    bool operator()( const char * one, const char * two ) const {
      return qstricmp( one, two ) < 0;
    }
  };

  class CryptoBackendFactory : public QObject {
    Q_OBJECT
  protected:
    CryptoBackendFactory();
    ~CryptoBackendFactory();
  public:
    static CryptoBackendFactory * instance();

    const CryptoBackend::Protocol * smime() const;
    const CryptoBackend::Protocol * openpgp() const;
    const CryptoBackend::Protocol * protocol( const char * name ) const;
    CryptoConfig * config() const;

    const CryptoBackend * backend( unsigned int idx ) const;

    bool hasBackends() const;

    Kleo::BackendConfigWidget * configWidget( QWidget * parent=0, const char * name=0 ) const;

    KConfig* configObject() const;

    // The preferred backend for smime (can be 0) - currently unused
    //const CryptoBackend* smimeBackend() const;
    // The preferred backend for openpgp (can be 0) - currently unused
    //const CryptoBackend* openpgpBackend() const;

    // For BackendConfigWidget to save the configuration
    // 0 means no backend selected.
    void setSMIMEBackend( const CryptoBackend* backend );
    void setOpenPGPBackend( const CryptoBackend* backend );
    void setProtocolBackend( const char * name, const CryptoBackend * backend );

    void scanForBackends( QStringList * reasons=0 );

    const char * enumerateProtocols( int i ) const;

  protected:
    std::vector<CryptoBackend*> mBackendList;
    mutable KConfig* mConfigObject;
    typedef std::map<const char *, const CryptoBackend*, lt_i_str> BackendMap;
    BackendMap mBackends;
    typedef std::vector<const char *> ProtocolSet;
    ProtocolSet mAvailableProtocols;

  private:
    const CryptoBackend * backendByName( const QString& name ) const;
    void readConfig();
    CryptoBackendFactory( const CryptoBackendFactory & );
    void operator=( const CryptoBackendFactory & );

    static CryptoBackendFactory * mSelf;
  };


}

#endif // __KLEO_CRYPTOBACKENDFACTORY_H__
