/*
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>

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

#include "messagefactorytest.h"

#include "cryptofunctions.h"

#include <messagecore/utils/stringutil.h>
#include <messagecore/helpers/nodehelper.h>

#include "messagecomposer/composer/composer.h"
#include "messagecomposer/helper/messagefactory.h"
#include "messagecomposer/part/globalpart.h"
#include "messagecomposer/settings/messagecomposersettings.h"

#include "messagecomposer/part/infopart.h"
#include "messagecomposer/part/textpart.h"

#include "testhtmlwriter.h"
#include "testcsshelper.h"
#include <messageviewer/viewer/nodehelper.h>
#include <messagecore/autotests/util.h>

#include <messageviewer/viewer/objecttreeparser.h>

#include "qtest_messagecomposer.h"
#include <kmime/kmime_dateformatter.h>

#include <KIdentityManagement/kidentitymanagement/identitymanager.h>
#include <KIdentityManagement/kidentitymanagement/identity.h>
#include <qtest.h>
#include <QDateTime>
#include <KCharsets>
#include <QtCore/QDir>
#include <KLocale>
#include "templateparser/globalsettings_base.h"
#include "templateparser/templateparser_export.h"

using namespace MessageComposer;
using namespace MessageComposer;

namespace
{
template <typename String>
String very_simplistic_diff(const String &a, const String &b)
{
    const QList<String> al = a.split('\n');
    const QList<String> bl = b.split('\n');
    String result;
    int ai = 0, bi = 0;
    while (ai < al.size() && bi < bl.size())
        if (al[ai] == bl[bi]) {
            //qDebug( "found   equal line a@%d x b@%d", ai, bi );
            result += "  " + al[ai] + '\n';
            ++ai;
            ++bi;
        } else {
            //qDebug( "found unequal line a@%d x b@%d", ai, bi );
            const int b_in_a = al.indexOf(bl[bi], ai);
            const int a_in_b = bl.indexOf(al[ai], bi);
            //qDebug( "   b_in_a == %d", b_in_a );
            //qDebug( "   a_in_b == %d", a_in_b );
            if (b_in_a == -1) {
                if (a_in_b == -1)
                    // (at least) one line changed:
                    result += "- " + al[ai++] + '\n'
                              +  "+ " + bl[bi++] + '\n';
                else
                    // some lines added:
                    while (bi < a_in_b) {
                        result += "+ " + bl[bi++] + '\n';
                    }
            } else {
                // some lines removed:
                while (ai < b_in_a) {
                    result += "- " + al[ai++] + '\n';
                }
                // some lines added:
                while (bi < a_in_b) {
                    result += "+ " + bl[bi++] + '\n';
                }
            }
            //qDebug( "result ( a@%d b@%d ):\n%s\n--end", ai, bi, result.constData() );
        }

    for (int i = ai ; i < al.size() ; ++i) {
        result += "- " + al[i] + '\n';
    }
    for (int i = bi ; i < bl.size() ; ++i) {
        result += "+ " + bl[i] + '\n';
    }
    return result;
}
}

#define QCOMPARE_OR_DIFF( a, b )                                        \
    if ( a != b )                                                       \
        qDebug( "diff:\n--begin--\n%s\n--end--", very_simplistic_diff( a, b ).constData() ); \
    QVERIFY( a == b )

QTEST_MAIN(MessageFactoryTest)

void MessageFactoryTest::testCreateReply()
{
    KMime::Message::Ptr msg = createPlainTestMessage();
    KIdentityManagement::IdentityManager *identMan = new KIdentityManagement::IdentityManager;

    MessageFactory factory(msg, 0);
    factory.setIdentityManager(identMan);

    MessageFactory::MessageReply reply =  factory.createReply();
    QVERIFY(reply.replyAll = true);
    qDebug() << reply.msg->body();

    QDateTime date = msg->date()->dateTime();
    QString datetime = KLocale::global()->formatDate(date.date(), KLocale::LongDate);
    datetime += QLatin1String(" ") + KLocale::global()->formatTime(date.time(), true);
    QString replyStr = QString::fromLatin1(QByteArray(QByteArray("On ") + datetime.toLatin1() + QByteArray(" you wrote:\n> All happy families are alike; each unhappy family is unhappy in its own way.\n\n")));
    QVERIFY(reply.msg->subject()->asUnicodeString() == QLatin1String("Re: Test Email Subject"));
    QCOMPARE_OR_DIFF(reply.msg->body(), replyStr.toLatin1());

}

void MessageFactoryTest::testCreateReplyHtml()
{
    KMime::Message::Ptr msg = loadMessageFromFile(QLatin1String("html_utf8_encoded.mbox"));
    KIdentityManagement::IdentityManager *identMan = new KIdentityManagement::IdentityManager;

    qDebug() << "html message:" << msg->encodedContent();

    MessageFactory factory(msg, 0);
    factory.setIdentityManager(identMan);

    MessageFactory::MessageReply reply =  factory.createReply();
    QVERIFY(reply.replyAll = true);
    qDebug() << "html reply" << reply.msg->encodedContent();

    QDateTime date = msg->date()->dateTime();
    QString datetime = KLocale::global()->formatDate(date.date(), KLocale::LongDate);
    datetime += QLatin1String(" ") + KLocale::global()->formatTime(date.time(), true);
    QString replyStr = QString::fromLatin1(QByteArray(QByteArray("On ") + datetime.toLatin1() + QByteArray(" you wrote:\n> encoded?\n")));
    QSKIP("This test has been failing for a long time, please someone fix it", SkipSingle);
    QVERIFY(reply.msg->contentType()->mimeType() == "multipart/alternative");
    QVERIFY(reply.msg->subject()->asUnicodeString() == QLatin1String("Re: reply to please"));
    QCOMPARE_OR_DIFF(reply.msg->contents().at(0)->body(), replyStr.toLatin1());

}

void MessageFactoryTest::testCreateReplyUTF16Base64()
{
    KMime::Message::Ptr msg = loadMessageFromFile(QLatin1String("plain_utf16.mbox"));
    KIdentityManagement::IdentityManager *identMan = new KIdentityManagement::IdentityManager;

//   qDebug() << "plain base64 msg message:" << msg->encodedContent();

    MessageFactory factory(msg, 0);
    factory.setIdentityManager(identMan);

    MessageFactory::MessageReply reply =  factory.createReply();
    QVERIFY(reply.replyAll = true);
//   qDebug() << "html reply" << reply.msg->encodedContent();

    QDateTime date = msg->date()->dateTime();
    QString datetime = KLocale::global()->formatDate(date.date(), KLocale::LongDate);
    datetime += QLatin1String(" ") + KLocale::global()->formatTime(date.time(), true);
    QString replyStr = QString::fromLatin1(QByteArray(QByteArray("On ") + datetime.toLatin1() + QByteArray(" you wrote:\n> quote me please.\n")));
    QSKIP("This test has been failing for a long time, please someone fix it", SkipSingle);
    QVERIFY(reply.msg->contentType()->mimeType() == "multipart/alternative");
    QVERIFY(reply.msg->subject()->asUnicodeString() == QLatin1String("Re: asking for reply"));
    QCOMPARE_OR_DIFF(reply.msg->contents().at(0)->body(), replyStr.toLatin1());

}

void MessageFactoryTest::testCreateForward()
{
    KMime::Message::Ptr msg = createPlainTestMessage();
    KIdentityManagement::IdentityManager *identMan = new KIdentityManagement::IdentityManager;
    KIdentityManagement::Identity &ident = identMan->modifyIdentityForUoid(identMan->identityForUoidOrDefault(0).uoid());
    ident.setFullName(QLatin1String("another"));
    ident.setPrimaryEmailAddress(QLatin1String("another@another.com"));
    identMan->commit();

    MessageFactory factory(msg, 0);
    factory.setIdentityManager(identMan);

    KMime::Message::Ptr fw =  factory.createForward();

    QDateTime date = msg->date()->dateTime();
    QString datetime = KLocale::global()->formatDate(date.date(), KLocale::LongDate);
    datetime += QLatin1String(", ") + KLocale::global()->formatTime(date.time(), true);

    QString fwdMsg = QString::fromLatin1(
                         "From: another <another@another.com>\n"
                         "Subject: Fwd: Test Email Subject\n"
                         "Date: %2\n"
                         "User-Agent: %3\n"
                         "X-KMail-Transport: 0\n"
                         "MIME-Version: 1.0\n"
                         "Content-Type: text/plain; charset=\"ISO-8859-1\"\n"
                         "Content-Transfer-Encoding: 8Bit\n"
                         "X-KMail-Link-Message: 0\n"
                         "X-KMail-Link-Type: forward\n"
                         "\n"
                         "\n"
                         "----------  Forwarded Message  ----------\n"
                         "\n"
                         "Subject: Test Email Subject\n"
                         "Date: %1\n"
                         "From: me@me.me\n"
                         "To: you@you.you\n"
                         "CC: cc@cc.cc\n"
                         "\n"
                         "All happy families are alike; each unhappy family is unhappy in its own way.\n"
                         "-----------------------------------------");
    fwdMsg = fwdMsg.arg(datetime).arg(fw->date()->asUnicodeString()).arg(fw->userAgent()->asUnicodeString());

//   qDebug() << "got:" << fw->encodedContent() << "against" << fwdMsg.toLatin1();
    QCOMPARE(fw->subject()->asUnicodeString(), QLatin1String("Fwd: Test Email Subject"));
    QCOMPARE_OR_DIFF(fw->encodedContent(), fwdMsg.toLatin1());
}

void MessageFactoryTest::testCreateRedirect()
{
    KMime::Message::Ptr msg = createPlainTestMessage();
    KIdentityManagement::IdentityManager *identMan = new KIdentityManagement::IdentityManager;
    KIdentityManagement::Identity &ident = identMan->modifyIdentityForUoid(identMan->identityForUoidOrDefault(0).uoid());
    ident.setFullName(QLatin1String("another"));
    ident.setPrimaryEmailAddress(QLatin1String("another@another.com"));
    identMan->commit();

    MessageFactory factory(msg, 0);
    factory.setIdentityManager(identMan);

    QString redirectTo = QLatin1String("redir@redir.com");
    KMime::Message::Ptr rdir =  factory.createRedirect(redirectTo);

    QDateTime date = rdir->date()->dateTime();
    QString datetime = KLocale::global()->formatDate(date.date(), KLocale::LongDate);
    datetime = rdir->date()->asUnicodeString();

//   qDebug() << rdir->encodedContent();

    QString msgId = MessageCore::StringUtil::generateMessageId(msg->sender()->asUnicodeString(), QString());

    QRegExp rx(QString::fromLatin1("Resent-Message-ID: ([^\n]*)"));
    rx.indexIn(QString::fromLatin1(rdir->head()));

    QRegExp rxmessageid(QString::fromLatin1("Message-ID: ([^\n]+)"));
    rxmessageid.indexIn(QString::fromLatin1(rdir->head()));
    qWarning() << "messageid:" << rxmessageid.cap(1) << "(" << rdir->head() << ")";
    QString baseline = QString::fromLatin1("From: me@me.me\n"
                                           "Cc: cc@cc.cc\n"
                                           "Bcc: bcc@bcc.bcc\n"
                                           "Subject: Test Email Subject\n"
                                           "Date: %1\n"
                                           "X-KMail-Transport: 0\n"
                                           "Message-ID: %2\n"
                                           "Disposition-Notification-To: me@me.me\n"
                                           "MIME-Version: 1.0\n"
                                           "Content-Transfer-Encoding: 7Bit\n"
                                           "Content-Type: text/plain; charset=\"us-ascii\"\n"
                                           "Resent-Message-ID: %3\n"
                                           "Resent-Date: %4\n"
                                           "Resent-From: %5\n"
                                           "To: you@you.you\n"
                                           "Resent-To: redir@redir.com\n"
                                           "X-KMail-Redirect-From: me@me.me (by way of another <another@another.com>)\n"
                                           "\n"
                                           "All happy families are alike; each unhappy family is unhappy in its own way.");
    baseline = baseline.arg(datetime).arg(rxmessageid.cap(1)).arg(rx.cap(1)).arg(datetime).arg(QLatin1String("another <another@another.com>"));

//   qDebug() << baseline.toLatin1();
//   qDebug() << "instead:" << rdir->encodedContent();

//   QString fwdStr = QString::fromLatin1( "On " + datetime.toLatin1() + " you wrote:\n> All happy families are alike; each unhappy family is unhappy in its own way.\n" );
    QCOMPARE(rdir->subject()->asUnicodeString(), QLatin1String("Test Email Subject"));
    QCOMPARE_OR_DIFF(rdir->encodedContent(), baseline.toLatin1());
}

void MessageFactoryTest::testCreateResend()
{
    KMime::Message::Ptr msg = createPlainTestMessage();
    KIdentityManagement::IdentityManager *identMan = new KIdentityManagement::IdentityManager;
    KIdentityManagement::Identity &ident = identMan->modifyIdentityForUoid(identMan->identityForUoidOrDefault(0).uoid());
    ident.setFullName(QLatin1String("another"));
    ident.setPrimaryEmailAddress(QLatin1String("another@another.com"));
    identMan->commit();

    MessageFactory factory(msg, 0);
    factory.setIdentityManager(identMan);

    KMime::Message::Ptr rdir =  factory.createResend();

    QDateTime date = rdir->date()->dateTime();
    QString datetime = KLocale::global()->formatDate(date.date(), KLocale::LongDate);
    datetime = rdir->date()->asUnicodeString();

//   qDebug() << msg->encodedContent();

    QString msgId = MessageCore::StringUtil::generateMessageId(msg->sender()->asUnicodeString(), QString());

    QRegExp rx(QString::fromLatin1("Resent-Message-ID: ([^\n]*)"));
    rx.indexIn(QString::fromLatin1(rdir->head()));

    QRegExp rxmessageid(QString::fromLatin1("Message-ID: ([^\n]+)"));
    rxmessageid.indexIn(QString::fromLatin1(rdir->head()));

    QString baseline = QString::fromLatin1("From: me@me.me\n"
                                           "To: %1\n"
                                           "Cc: cc@cc.cc\n"
                                           "Bcc: bcc@bcc.bcc\n"
                                           "Subject: Test Email Subject\n"
                                           "Date: %2\n"
                                           "X-KMail-Transport: 0\n"
                                           "Message-ID: %3\n"
                                           "Disposition-Notification-To: me@me.me\n"
                                           "MIME-Version: 1.0\n"
                                           "Content-Transfer-Encoding: 7Bit\n"
                                           "Content-Type: text/plain; charset=\"us-ascii\"\n"
                                           "\n"
                                           "All happy families are alike; each unhappy family is unhappy in its own way.");
    baseline = baseline.arg(msg->to()->asUnicodeString()).arg(datetime).arg(rxmessageid.cap(1));

    qDebug() << baseline.toLatin1();
    qDebug() << "instead:" << rdir->encodedContent();

//   QString fwdStr = QString::fromLatin1( "On " + datetime.toLatin1() + " you wrote:\n> All happy families are alike; each unhappy family is unhappy in its own way.\n" );
    QCOMPARE(rdir->subject()->asUnicodeString(), QLatin1String("Test Email Subject"));
    QCOMPARE_OR_DIFF(rdir->encodedContent(), baseline.toLatin1());
}

void MessageFactoryTest::testCreateMDN()
{
    KMime::Message::Ptr msg = createPlainTestMessage();
    KIdentityManagement::IdentityManager *identMan = new KIdentityManagement::IdentityManager;

    MessageFactory factory(msg, 0);

    factory.setIdentityManager(identMan);

    KMime::Message::Ptr mdn = factory.createMDN(KMime::MDN::AutomaticAction, KMime::MDN::Displayed, KMime::MDN::SentAutomatically);

    QVERIFY(mdn.get());
    qDebug() << "mdn" << mdn->encodedContent();
    /*
      // parse the result and make sure it is valid in various ways
      TestHtmlWriter testWriter;
      TestCSSHelper testCSSHelper;
      TestObjectTreeSource testSource( &testWriter, &testCSSHelper );
      MessageViewer::NodeHelper* nh = new MessageViewer::NodeHelper;
      MessageViewer::ObjectTreeParser otp( &testSource, nh, 0, false, true, 0 );
      MessageViewer::ProcessResult pResult( nh ); */

//   qDebug() << MessageCore::NodeHelper::firstChild( mdn->mainBodyPart() )->encodedContent();
//   qDebug() << MessageCore::NodeHelper::next(  MessageViewer::ObjectTreeParser::findType( mdn.get(), "multipart", "report", true, true ) )->body();

    QString mdnContent = QString::fromLatin1("The message sent on %1 to %2 with subject \"%3\" has been displayed. "
                         "This is no guarantee that the message has been read or understood.");
    mdnContent = mdnContent.arg(KMime::DateFormatter::formatDate(KMime::DateFormatter::Localized, msg->date()->dateTime().toTime_t()))
                 .arg(msg->to()->asUnicodeString()).arg(msg->subject()->asUnicodeString());

    qDebug() << "comparing with:" << mdnContent;

    QCOMPARE_OR_DIFF(MessageCore::NodeHelper::next(MessageViewer::ObjectTreeParser::findType(mdn.get(), "multipart", "report", true, true))->body(),
                     mdnContent.toLatin1());
}

KMime::Message::Ptr MessageFactoryTest::createPlainTestMessage()
{
    Composer *composer = new Composer;
    composer->globalPart()->setFallbackCharsetEnabled(true);
    composer->infoPart()->setFrom(QString::fromLatin1("me@me.me"));
    composer->infoPart()->setTo(QStringList(QString::fromLatin1("you@you.you")));
    composer->infoPart()->setCc(QStringList(QString::fromLatin1("cc@cc.cc")));
    composer->infoPart()->setBcc(QStringList(QString::fromLatin1("bcc@bcc.bcc")));
    composer->textPart()->setWrappedPlainText(QString::fromLatin1("All happy families are alike; each unhappy family is unhappy in its own way."));
    composer->infoPart()->setSubject(QLatin1String("Test Email Subject"));
    composer->globalPart()->setMDNRequested(true);
    composer->exec();

    KMime::Message::Ptr message = KMime::Message::Ptr(composer->resultMessages().first());
    delete composer;

    MessageComposerSettings::self()->setPreferredCharsets(QStringList() << QLatin1String("us-ascii") << QLatin1String("iso-8859-1") << QLatin1String("utf-8"));

    return message;
}

KMime::Message::Ptr MessageFactoryTest::loadMessageFromFile(QString filename)
{
    QFile file(QLatin1String(QByteArray(MAIL_DATA_DIR "/" + filename.toLatin1())));
    const bool opened = file.open(QIODevice::ReadOnly);
    Q_ASSERT(opened);
    Q_UNUSED(opened);
    const QByteArray data = KMime::CRLFtoLF(file.readAll());
    Q_ASSERT(!data.isEmpty());
    KMime::Message::Ptr msg(new KMime::Message);
    msg->setContent(data);
    msg->parse();
    return msg;
}

void MessageFactoryTest::test_multipartAlternative_data()
{
    QTest::addColumn<QString>("mailFileName");
    QTest::addColumn<int>("contentAt");
    QTest::addColumn<QString>("selection");
    QTest::addColumn<QString>("expected");

    QDir dir(QLatin1String(MAIL_DATA_DIR));
    foreach (const QString &file, dir.entryList(QStringList(QLatin1String("plain_message.mbox")), QDir::Files | QDir::Readable | QDir::NoSymLinks)) {
        QTest::newRow(file.toLatin1()) << QString(dir.path() + QLatin1Char('/') + file) << 0 << "" <<
                                       "> This *is* the *message* text *from* Sudhendu Kumar<dontspamme@yoohoo.com>\n"
                                       "> \n"
                                       "> --\n"
                                       "> Thanks & Regards\n"
                                       "> Sudhendu Kumar";
        QTest::newRow(file.toLatin1()) << QString(dir.path() + QLatin1Char('/') + file) << 1 << "" << "<html><head></head><body>"
                                       "<blockquote>This <i>is</i> the <b>message</b> text <u>from</u> Sudhendu Kumar&lt;dontspamme@yoohoo.com&gt;<br>"
                                       "<br>-- <br>Thanks &amp; Regards<br>Sudhendu Kumar<br>\n</blockquote><br/></body></html>";
    }
}

void MessageFactoryTest::test_multipartAlternative()
{
    QFETCH(QString, mailFileName);
    QFETCH(int, contentAt);
    QFETCH(QString, selection);
    QFETCH(QString, expected);

    QFile mailFile(mailFileName);
    QVERIFY(mailFile.open(QIODevice::ReadOnly));
    const QByteArray mailData = KMime::CRLFtoLF(mailFile.readAll());
    QVERIFY(!mailData.isEmpty());
    KMime::Message::Ptr origMsg(new KMime::Message);
    origMsg->setContent(mailData);
    origMsg->parse();

    KIdentityManagement::IdentityManager *identMan = new KIdentityManagement::IdentityManager;

    MessageFactory factory(origMsg, 0);
    factory.setIdentityManager(identMan);
    factory.setSelection(selection);
    factory.setQuote(true);
    factory.setReplyStrategy(ReplyAll);
    TemplateParser::GlobalSettings::self()->setTemplateReplyAll(QLatin1String("%QUOTE"));

    QString str;
    str = TemplateParser::GlobalSettings::self()->templateReplyAll();
    factory.setTemplate(str);

    MessageFactory::MessageReply reply =  factory.createReply();
    QVERIFY(reply.replyAll = true);
    QSKIP("This tests has been failing for a long time, please someone fix it", SkipSingle);
    QVERIFY(reply.msg->contentType()->mimeType() == "multipart/alternative");
    QVERIFY(reply.msg->subject()->asUnicodeString() == QLatin1String("Re: Plain Message Test"));
    QCOMPARE(reply.msg->contents().at(contentAt)->encodedBody().data(), expected.toLatin1().data());
}

