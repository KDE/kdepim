/*
 *    Copyright (c) 2015 Sandro Knauß <bugs@sandroknauss.de>
 *
 *    This library is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU Library General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or (at your
 *    option) any later version.
 *
 *    This library is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *    License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to the
 *    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *    02110-1301, USA.
 */

#include "filteractionpipethroughtest.h"
#include "../filteractions/filteractionpipethrough.h"
#include <qtest.h>

using namespace MailCommon;

FilterActionPipeThroughTest::FilterActionPipeThroughTest()
{

}

void FilterActionPipeThroughTest::setOutput(FilterAction *filter, const QByteArray &output)
{
    QByteArray sendData = output;
    filter->argsFromString(QLatin1String("echo \"") + sendData.replace('"', "\\\"") + "\"");
}

void FilterActionPipeThroughTest::testWithNoCommand()
{
    /* No command to exceute -> no output -> error
     */

    FilterActionPipeThrough filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    ItemContext context(item, true);

    filter.argsFromString("");
    QCOMPARE(filter.process(context, false), FilterAction::ErrorButGoOn);
    QCOMPARE(context.needsPayloadStore(), false);
}

void FilterActionPipeThroughTest::testWithInvalidCommandPath()
{
    /* put a mail in the pipe and make sure we get the same output
     */
    QByteArray data =
        "From: Konqui <konqui@kde.org>\n"
        "To: Friends <friends@kde.org>\n"
        "Date: Sun, 21 Mar 1993 23:56:48 -0800 (PST)\n"
        "Subject: Sample message\n"
        "MIME-Version: 1.0\n"
        "Content-type: multipart/mixed; boundary=\"simple boundary\"\n"
        "\n"
        "\n"
        "--simple boundary\n"
        "Content-type: text/plain; charset=us-ascii\n"
        "\n"
        "This is explicitly typed plain US-ASCII text.\n"
        "It DOES end with a linebreak.\n"
        "\n"
        "--simple boundary--\n";

    FilterActionPipeThrough filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    msgPtr->setContent(data);
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    ItemContext context(item, true);

    filter.argsFromString(QStringLiteral("/home/cat "));
    QCOMPARE(filter.process(context, false), FilterAction::ErrorButGoOn);
    QCOMPARE(context.needsPayloadStore(), false);
    QCOMPARE(msgPtr->encodedContent(), data);
}

void FilterActionPipeThroughTest::testCommandWithoutOutput()
{
    /* Valid command but no output -> error
     */

    FilterActionPipeThrough filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    ItemContext context(item, true);

    filter.argsFromString("echo ''");
    QCOMPARE(filter.process(context, false), FilterAction::ErrorButGoOn);
    QCOMPARE(context.needsPayloadStore(), false);
}

void FilterActionPipeThroughTest::testWithMailOutput()
{
    /* Make sure that mail is not changed from output to KMIME::Message
     * and also no assemble changes the mail
     *
     * Very important for not breake signatures from mails.
     */
    QByteArray data =
        "From: Konqui <konqui@kde.org>\n"
        "To: Friends <friends@kde.org>\n"
        "Date: Sun, 21 Mar 1993 23:56:48 -0800 (PST)\n"
        "Subject: Sample message\n"
        "MIME-Version: 1.0\n"
        "Content-type: multipart/mixed;\n"
        "              boundary=\"simple boundary\"\n"
        "\n"
        "\n"
        "--simple boundary\n"
        "Content-type: text/plain;\n"
        "        charset=us-ascii\n"
        "\n"
        "This is explicitly typed plain US-ASCII text.\n"
        "It DOES end with a linebreak.\n"
        "\n"
        "--simple boundary--\n";

    FilterActionPipeThrough filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    ItemContext context(item, true);

    setOutput(&filter, data);
    QCOMPARE(filter.process(context, false), FilterAction::GoOn);
    QCOMPARE(context.needsPayloadStore(), true);
    QByteArray expected = data + '\n';
    QCOMPARE(msgPtr->encodedContent(), expected);
    msgPtr->assemble();                               //Make sure that the message isFrozen so no submimes do not change
    QCOMPARE(msgPtr->encodedContent(), expected);
}

void FilterActionPipeThroughTest::testCopyMail()
{
    /* put a mail in the pipe and make sure we get the same output
     */
    QByteArray data =
        "From: Konqui <konqui@kde.org>\n"
        "To: Friends <friends@kde.org>\n"
        "Date: Sun, 21 Mar 1993 23:56:48 -0800 (PST)\n"
        "Subject: Sample message\n"
        "MIME-Version: 1.0\n"
        "Content-type: multipart/mixed; boundary=\"simple boundary\"\n"
        "\n"
        "\n"
        "--simple boundary\n"
        "Content-type: text/plain; charset=us-ascii\n"
        "\n"
        "This is explicitly typed plain US-ASCII text.\n"
        "It DOES end with a linebreak.\n"
        "\n"
        "--simple boundary--\n";

    FilterActionPipeThrough filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    msgPtr->setContent(data);
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    ItemContext context(item, true);

    filter.argsFromString(QStringLiteral("cat "));
    QCOMPARE(filter.process(context, false), FilterAction::GoOn);
    QCOMPARE(context.needsPayloadStore(), true);
    QCOMPARE(msgPtr->encodedContent(), data);
}

void FilterActionPipeThroughTest::testXUidUnchange()
{
    // the X-UID header isn't changed -> mail isn't changed anyhow
    QByteArray data =
        "From: Konqui <konqui@kde.org>\n"
        "To: Friends <friends@kde.org>\n"
        "Date: Sun, 21 Mar 1993 23:56:48 -0800 (PST)\n"
        "Subject: Sample message\n"
        "MIME-Version: 1.0\n"
        "X-UID: XXXX1\n"
        "Content-type: multipart/mixed;\n"
        "              boundary=\"simple boundary\"\n"
        "\n"
        "\n"
        "--simple boundary\n"
        "Content-type: text/plain;\n"
        "              charset=us-ascii\n"
        "\n"
        "This is explicitly typed plain US-ASCII text.\n"
        "It DOES end with a linebreak.\n"
        "\n"
        "--simple boundary--\n";

    FilterActionPipeThrough filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    msgPtr->setContent(data);
    msgPtr->parse();
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    ItemContext context(item, true);

    filter.argsFromString(QStringLiteral("cat "));
    QCOMPARE(filter.process(context, false), FilterAction::GoOn);
    QCOMPARE(context.needsPayloadStore(), true);
    QCOMPARE(QString::fromAscii(msgPtr->encodedContent()), QString::fromAscii(data));
}

void FilterActionPipeThroughTest::testXUidRemoved()
{
    /* Make sure if the X-Uid is removed from pipe through, that we add it again
     * but we have to assemble the mail, so we create some changes in the header.
     * More important is, that the body isn't changed.
     */

    QByteArray data =
        "From: Konqui <konqui@kde.org>\n"
        "To: Friends <friends@kde.org>\n"
        "Date: Sun, 21 Mar 1993 23:56:48 -0800 (PST)\n"
        "Subject: Sample message\n"
        "MIME-Version: 1.0\n"
        "X-UID: XXXX1\n"
        "Content-type: multipart/mixed;\n"
        "              boundary=\"simple boundary\"\n"
        "\n"
        "\n"
        "--simple boundary\n"
        "Content-type: text/plain;\n"
        "              charset=us-ascii\n"
        "\n"
        "This is explicitly typed plain US-ASCII text.\n"
        "It DOES end with a linebreak.\n"
        "\n"
        "--simple boundary--\n";

    QByteArray send =
        "From: Konqui <konqui@kde.org>\n"
        "To: Friends <friends@kde.org>\n"
        "Date: Sun, 21 Mar 1993 23:56:48 -0800 (PST)\n"
        "Subject: Sample message\n"
        "MIME-Version: 1.0\n"
        "Content-type: multipart/mixed;\n"
        "              boundary=\"simple boundary\"\n"
        "\n"
        "\n"
        "--simple boundary\n"
        "Content-type: text/plain;\n"
        "              charset=us-ascii\n"
        "\n"
        "This is explicitly typed plain US-ASCII text.\n"
        "It DOES end with a linebreak.\n"
        "\n"
        "--simple boundary--\n";

    QByteArray output =
        "From: Konqui <konqui@kde.org>\n"
        "To: Friends <friends@kde.org>\n"
        "Date: Sun, 21 Mar 1993 23:56:48 -0800\n"                           // <- this is removed, because we assemble
        "Subject: Sample message\n"
        "MIME-Version: 1.0\n"
        "Content-Type: multipart/mixed; boundary=\"simple boundary\"\n"     // <- this nweline is removed, because we assemble
        "X-UID: XXXX1\n"
        "\n"
        "\n"
        "--simple boundary\n"                                               // <- body isn't changed
        "Content-type: text/plain;\n"
        "              charset=us-ascii\n"
        "\n"
        "This is explicitly typed plain US-ASCII text.\n"
        "It DOES end with a linebreak.\n"
        "\n"
        "--simple boundary--\n"
        "\n";

    FilterActionPipeThrough filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    msgPtr->setContent(data);
    msgPtr->parse();
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    ItemContext context(item, true);
    setOutput(&filter, send);
    QCOMPARE(filter.process(context, false), FilterAction::GoOn);
    QCOMPARE(context.needsPayloadStore(), true);
    QCOMPARE(msgPtr->encodedContent(), output);
}

void FilterActionPipeThroughTest::shouldRequiresPart()
{
    FilterActionPipeThrough filter(this);
    QCOMPARE(filter.requiredPart(), SearchRule::CompleteMessage);
}

void FilterActionPipeThroughTest::testXUidChange()
{
    /* Make sure if the X-Uid is changed from pipe through, that we put is to the original value again.
     * The mail is assembled again, so we create some changes in the header.
     * More important is, that the body isn't changed.
     */

    QByteArray data =
        "From: Konqui <konqui@kde.org>\n"
        "To: Friends <friends@kde.org>\n"
        "Date: Sun, 21 Mar 1993 23:56:48 -0800 (PST)\n"
        "Subject: Sample message\n"
        "MIME-Version: 1.0\n"
        "X-UID: XXXX1\n"
        "Content-type: multipart/mixed;\n"
        "              boundary=\"simple boundary\"\n"
        "\n"
        "\n"
        "--simple boundary\n"
        "Content-type: text/plain;\n"
        "              charset=us-ascii\n"
        "\n"
        "This is explicitly typed plain US-ASCII text.\n"
        "It DOES end with a linebreak.\n"
        "\n"
        "--simple boundary--\n";

    QByteArray send =
        "From: Konqui <konqui@kde.org>\n"
        "To: Friends <friends@kde.org>\n"
        "Date: Sun, 21 Mar 1993 23:56:48 -0800 (PST)\n"
        "Subject: Sample message\n"
        "MIME-Version: 1.0\n"
        "X-UID: XXXX2\n"
        "Content-type: multipart/mixed;\n"
        "              boundary=\"simple boundary\"\n"
        "\n"
        "\n"
        "--simple boundary\n"
        "Content-type: text/plain;\n"
        "              charset=us-ascii\n"
        "\n"
        "This is explicitly typed plain US-ASCII text.\n"
        "It DOES end with a linebreak.\n"
        "\n"
        "--simple boundary--\n";

    QByteArray output =
        "From: Konqui <konqui@kde.org>\n"
        "To: Friends <friends@kde.org>\n"
        "Date: Sun, 21 Mar 1993 23:56:48 -0800\n"                           // <- this is removed, because we assemble
        "Subject: Sample message\n"
        "MIME-Version: 1.0\n"
        "Content-Type: multipart/mixed; boundary=\"simple boundary\"\n"     // <- this nweline is removed, because we assemble
        "X-UID: XXXX1\n"
        "\n"
        "\n"
        "--simple boundary\n"                                               // <- body isn't changed
        "Content-type: text/plain;\n"
        "              charset=us-ascii\n"
        "\n"
        "This is explicitly typed plain US-ASCII text.\n"
        "It DOES end with a linebreak.\n"
        "\n"
        "--simple boundary--\n"
        "\n";

    FilterActionPipeThrough filter(this);
    KMime::Message::Ptr msgPtr = KMime::Message::Ptr(new KMime::Message());
    Akonadi::Item item;
    msgPtr->setContent(data);
    msgPtr->parse();
    item.setPayload<KMime::Message::Ptr>(msgPtr);
    ItemContext context(item, true);
    setOutput(&filter, send);
    QCOMPARE(filter.process(context, false), FilterAction::GoOn);
    QCOMPARE(context.needsPayloadStore(), true);
    QCOMPARE(msgPtr->encodedContent(), output);
}

QTEST_MAIN(FilterActionPipeThroughTest)
