/*  -*- mode: C++; c-file-style: "gnu" -*-
    objecttreeparser_p.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

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
    the TQt library by Trolltech AS, Norway (or with modified versions
    of TQt that use the same license as TQt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    TQt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef _KMAIL_OBJECTTREEPARSER_P_H_
#define _KMAIL_OBJECTTREEPARSER_P_H_

#include <gpgmepp/verificationresult.h>
#include <gpgmepp/decryptionresult.h>
#include <gpgmepp/key.h>

#include <tqobject.h>
#include <tqcstring.h>
#include <tqstring.h>
#include <tqguardedptr.h>

#include "isubject.h"
#include "interfaces/bodypart.h"

namespace Kleo {
  class DecryptVerifyJob;
  class VerifyDetachedJob;
  class VerifyOpaqueJob;
  class KeyListJob;
}

class TQStringList;

namespace KMail {

  class CryptoBodyPartMemento
    : public TQObject,
      public KMail::Interface::BodyPartMemento,
      public KMail::ISubject
  {
    Q_OBJECT
  public:
    CryptoBodyPartMemento();
    ~CryptoBodyPartMemento();

    /* reimp */ Interface::Observer   * asObserver()   { return 0;    }
    /* reimp */ Interface::Observable * asObservable() { return this; }

    bool isRunning() const { return m_running; }

    const TQString & auditLogAsHtml() const { return m_auditLog; }
    GpgME::Error auditLogError() const { return m_auditLogError; }

  protected:
    void setAuditLog( const GpgME::Error & err, const TQString & log );
    void setRunning( bool running );

  private:
    bool m_running;
    TQString m_auditLog;
    GpgME::Error m_auditLogError;
  };

  class DecryptVerifyBodyPartMemento
    : public CryptoBodyPartMemento
  {
    Q_OBJECT
  public:
    DecryptVerifyBodyPartMemento( Kleo::DecryptVerifyJob * job, const TQByteArray & cipherText );
    ~DecryptVerifyBodyPartMemento();

    bool start();
    void exec();

    const TQByteArray & plainText() const { return m_plainText; }    
    const GpgME::DecryptionResult & decryptResult() const { return m_dr; }
    const GpgME::VerificationResult & verifyResult() const { return m_vr; }

  private slots:
    void slotResult( const GpgME::DecryptionResult & dr,
                     const GpgME::VerificationResult & vr,
                     const TQByteArray & plainText );

  private:
    void saveResult( const GpgME::DecryptionResult &,
                     const GpgME::VerificationResult &,
                     const TQByteArray & );
  private:
    // input:
    const TQByteArray m_cipherText;
    TQGuardedPtr<Kleo::DecryptVerifyJob> m_job;
    // output:
    GpgME::DecryptionResult m_dr;
    GpgME::VerificationResult m_vr;
    TQByteArray m_plainText;
  };


  class VerifyDetachedBodyPartMemento
    : public CryptoBodyPartMemento
  {
    Q_OBJECT
  public:
    VerifyDetachedBodyPartMemento( Kleo::VerifyDetachedJob * job,
                                   Kleo::KeyListJob * klj,
                                   const TQByteArray & signature,
                                   const TQByteArray & plainText );
    ~VerifyDetachedBodyPartMemento();

    bool start();
    void exec();

    const GpgME::VerificationResult & verifyResult() const { return m_vr; }
    const GpgME::Key & signingKey() const { return m_key; }

  private slots:
    void slotResult( const GpgME::VerificationResult & vr );
    void slotKeyListJobDone();
    void slotNextKey( const GpgME::Key & );

  private:
    void saveResult( const GpgME::VerificationResult & );
    bool canStartKeyListJob() const;
    TQStringList keyListPattern() const;
    bool startKeyListJob();
  private:
    // input:
    const TQByteArray m_signature;
    const TQByteArray m_plainText;
    TQGuardedPtr<Kleo::VerifyDetachedJob> m_job;
    TQGuardedPtr<Kleo::KeyListJob> m_keylistjob;
    // output:
    GpgME::VerificationResult m_vr;
    GpgME::Key m_key;
  };


  class VerifyOpaqueBodyPartMemento
    : public CryptoBodyPartMemento
  {
    Q_OBJECT
  public:
    VerifyOpaqueBodyPartMemento( Kleo::VerifyOpaqueJob * job,
                                 Kleo::KeyListJob * klj,
                                 const TQByteArray & signature );
    ~VerifyOpaqueBodyPartMemento();

    bool start();
    void exec();

    const TQByteArray & plainText() const { return m_plainText; }    
    const GpgME::VerificationResult & verifyResult() const { return m_vr; }
    const GpgME::Key & signingKey() const { return m_key; }

  private slots:
    void slotResult( const GpgME::VerificationResult & vr,
                     const TQByteArray & plainText );
    void slotKeyListJobDone();
    void slotNextKey( const GpgME::Key & );

  private:
    void saveResult( const GpgME::VerificationResult &,
                     const TQByteArray & );
    bool canStartKeyListJob() const;
    TQStringList keyListPattern() const;
    bool startKeyListJob();
  private:
    // input:
    const TQByteArray m_signature;
    TQGuardedPtr<Kleo::VerifyOpaqueJob> m_job;
    TQGuardedPtr<Kleo::KeyListJob> m_keylistjob;
    // output:
    GpgME::VerificationResult m_vr;
    TQByteArray m_plainText;
    GpgME::Key m_key;
  };


} // namespace KMail

#endif // _KMAIL_OBJECTTREEPARSER_H_
