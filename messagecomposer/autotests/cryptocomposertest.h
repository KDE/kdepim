/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
  Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>

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

#ifndef CRYPTOCOMPOSERTEST_H
#define CRYPTOCOMPOSERTEST_H

#include <QtCore/QObject>

namespace MessageComposer
{
class Composer;
}

class CryptoComposerTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();

private Q_SLOTS:
    // openpgp
    void testOpenPGPMime();
    void testOpenPGPMime_data();

    // the following will do for s-mime as well, as the same sign/enc jobs are used
    void testEncryptSameAttachments();
    void testEncryptSameAttachments_data();
    void testSignEncryptLateAttachments();
    void testSignEncryptLateAttachments_data();

    // secondary recipients
    void testBCCEncrypt();
    void testBCCEncrypt_data();

    // inline pgp
    void testOpenPGPInline_data();
    void testOpenPGPInline();

    // s-mime
    void testSMIME_data();
    void testSMIME();
    void testSMIMEOpaque_data();
    void testSMIMEOpaque();

    // contentTransferEncoding
    void testCTEquPr_data();
    void testCTEquPr();
    void testCTEbase64_data();
    void testCTEbase64();

    // TODO test the code for autodetecting the charset of a text attachment.
private:
    void fillComposerData(MessageComposer::Composer *composer, QString data);
    void fillComposerCryptoData(MessageComposer::Composer *composer);

    // convenience, shared code
    void runSMIMETest(bool sign, bool enc, bool opaque);
};

#endif
