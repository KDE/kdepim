/*  -*- c++ -*-
    partmetadata.h

    KMail, the KDE mail client.
    Copyright (c) 2002-2003 Karl-Heinz Zimmer <khz@kde.org>
    Copyright (c) 2003      Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/


#ifndef _KMAIL_PARTMETADATA_H_
#define _KMAIL_PARTMETADATA_H_

#include <gpgmepp/verificationresult.h>
#include <gpgmepp/context.h>

#include <kpgp.h>
#include <tqstring.h>
#include <tqcstring.h>
#include <tqdatetime.h>

namespace KMail {

  class PartMetaData {
  public:
    PartMetaData()
      : sigSummary( GpgME::Signature::None ),
        isSigned( false ),
        isGoodSignature( false ),
        isEncrypted( false ),
        isDecryptable( false ),
        inProgress( false ),
        technicalProblem( false ),
        isEncapsulatedRfc822Message( false )
    {
    }
    GpgME::Signature::Summary sigSummary;
    TQString signClass;
    TQString signer;
    TQStringList signerMailAddresses;
    TQCString keyId;
    Kpgp::Validity keyTrust;
    TQString status;  // to be used for unknown plug-ins
    int status_code; // to be used for i18n of OpenPGP and S/MIME CryptPlugs
    TQString errorText;
    TQDateTime creationTime;
    TQString decryptionError;
    TQString auditLog;
    GpgME::Error auditLogError;
    bool isSigned : 1;
    bool isGoodSignature : 1;
    bool isEncrypted : 1;
    bool isDecryptable : 1;
    bool inProgress : 1;
    bool technicalProblem : 1;
    bool isEncapsulatedRfc822Message : 1;
  };

} // namespace KMail

#endif // _KMAIL_PARTMETADATA_H_

