/* Copyright 2015 Sandro Knauß <bugs@sandroknauss.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "nodehelpertest.h"

#include "messageviewer/nodehelper.h"

#include <qtest.h>

using namespace MessageViewer;

NodeHelperTest::NodeHelperTest()
    : QObject()
{

}

void NodeHelperTest::testPersistentIndex()
{
    NodeHelper helper;

    KMime::Content *node = new KMime::Content();
    KMime::Content *subNode = new KMime::Content();
    KMime::Content *subsubNode = new KMime::Content(), *subsubNode2 = new KMime::Content();
    KMime::Content *extra = new KMime::Content(), *extra2 = new KMime::Content();
    KMime::Content *subExtra = new KMime::Content();
    KMime::Content *subsubExtra = new KMime::Content();
    KMime::Content *subsubExtraNode = new KMime::Content();

    subNode->addContent(subsubNode);
    subNode->addContent(subsubNode2);
    node->addContent(subNode);
    subsubExtra->addContent(subsubExtraNode);
    helper.attachExtraContent(node, extra);
    helper.attachExtraContent(node, extra2);
    helper.attachExtraContent(subNode, subExtra);
    helper.attachExtraContent(subsubNode2, subsubExtra);

    /*  all content has a internal first child, so indexes starts at 2
     * node                 ""
     * -> subNode           "2"
     *    -> subsubNode     "2.2"
     *    -> subsubNode2    "2.3"
     *
     * node                 ""
     * -> extra             "0:"
     * -> extra2            "1:"
     *
     * subNode              "2"
     * -> subExtra          "2:0:"
     *
     * subsubNode2          "2.3"
     * -> subsubExtra       "2.3:0:"
     *    -> subsubExtraNode    "2.3:0:2"
     *
     */

    QCOMPARE(helper.persistentIndex(node), QStringLiteral(""));
    QCOMPARE(helper.contentFromIndex(node, QStringLiteral("")), node);

    QCOMPARE(helper.persistentIndex(node->contents()[0]), QStringLiteral("1"));
    QCOMPARE(helper.contentFromIndex(node, QStringLiteral("1")), node->contents()[0]);

    QCOMPARE(helper.persistentIndex(subNode), QStringLiteral("2"));
    QCOMPARE(helper.contentFromIndex(node, QStringLiteral("2")), subNode);

    QCOMPARE(helper.persistentIndex(subsubNode), QStringLiteral("2.2"));
    QCOMPARE(helper.contentFromIndex(node, QStringLiteral("2.2")), subsubNode);

    QCOMPARE(helper.persistentIndex(subsubNode2), QStringLiteral("2.3"));
    QCOMPARE(helper.contentFromIndex(node, QStringLiteral("2.3")), subsubNode2);

    QCOMPARE(helper.persistentIndex(extra), QStringLiteral("0:"));
    QCOMPARE(helper.contentFromIndex(node, QStringLiteral("0:")), extra);

    QCOMPARE(helper.persistentIndex(extra2), QStringLiteral("1:"));
    QCOMPARE(helper.contentFromIndex(node, QStringLiteral("1:")), extra2);

    QCOMPARE(helper.persistentIndex(subExtra), QStringLiteral("2:0:"));
    QCOMPARE(helper.contentFromIndex(node, QStringLiteral("2:0:")), subExtra);

    QCOMPARE(helper.persistentIndex(subsubExtra), QStringLiteral("2.3:0:"));
    QCOMPARE(helper.contentFromIndex(node, QStringLiteral("2.3:0:")), subsubExtra);

    QCOMPARE(helper.persistentIndex(subsubExtraNode), QStringLiteral("2.3:0:2"));
    QCOMPARE(helper.contentFromIndex(node, QStringLiteral("2.3:0:2")), subsubExtraNode);

    delete node;
}

void NodeHelperTest::testHREF()
{
    NodeHelper helper;
    KMime::Message::Ptr msg(new KMime::Message);
    QUrl url;

    KMime::Content *node = msg->topLevel();
    KMime::Content *subNode = new KMime::Content();
    KMime::Content *subsubNode = new KMime::Content(), *subsubNode2 = new KMime::Content();
    KMime::Content *extra = new KMime::Content(), *extra2 = new KMime::Content();
    KMime::Content *subExtra = new KMime::Content();
    KMime::Content *subsubExtra = new KMime::Content();
    KMime::Content *subsubExtraNode = new KMime::Content();

    subNode->addContent(subsubNode);
    subNode->addContent(subsubNode2);
    node->addContent(subNode);
    subsubExtra->addContent(subsubExtraNode);
    helper.attachExtraContent(node, extra);
    helper.attachExtraContent(node, extra2);
    helper.attachExtraContent(subNode, subExtra);
    helper.attachExtraContent(subsubNode2, subsubExtra);

    url = QUrl(QStringLiteral(""));
    QCOMPARE(helper.fromHREF(msg, url), node);

    url = QUrl(QStringLiteral("attachment:0:?place=body"));
    QCOMPARE(helper.fromHREF(msg, url), extra);

    url = QUrl(QStringLiteral("attachment:2.2?place=body"));
    QCOMPARE(helper.fromHREF(msg, url), subsubNode);

    url = QUrl(QStringLiteral("attachment:2.3:0:2?place=body"));
    QCOMPARE(helper.fromHREF(msg, url), subsubExtraNode);

    QCOMPARE(helper.asHREF(node, QStringLiteral("body")), QStringLiteral("attachment:?place=body"));
    QCOMPARE(helper.asHREF(extra, QStringLiteral("body")), QStringLiteral("attachment:0:?place=body"));
    QCOMPARE(helper.asHREF(subsubNode, QStringLiteral("body")), QStringLiteral("attachment:2.2?place=body"));
    QCOMPARE(helper.asHREF(subsubExtraNode, QStringLiteral("body")), QStringLiteral("attachment:2.3:0:2?place=body"));
}

void NodeHelperTest::testLocalFiles()
{
    NodeHelper helper;
    KMime::Message::Ptr msg(new KMime::Message);

    KMime::Content *node = msg->topLevel();
    KMime::Content *subNode = new KMime::Content();
    KMime::Content *subsubNode = new KMime::Content(), *subsubNode2 = new KMime::Content();
    KMime::Content *extra = new KMime::Content(), *extra2 = new KMime::Content();
    KMime::Content *subExtra = new KMime::Content();
    KMime::Content *subsubExtra = new KMime::Content();
    KMime::Content *subsubExtraNode = new KMime::Content();

    subNode->addContent(subsubNode);
    subNode->addContent(subsubNode2);
    node->addContent(subNode);
    subsubExtra->addContent(subsubExtraNode);
    helper.attachExtraContent(node, extra);
    helper.attachExtraContent(node, extra2);
    helper.attachExtraContent(subNode, subExtra);
    helper.attachExtraContent(subsubNode2, subsubExtra);

    helper.writeNodeToTempFile(node);
    QCOMPARE(helper.fromHREF(msg, helper.tempFileUrlFromNode(node)), node);
    helper.writeNodeToTempFile(subNode);
    QCOMPARE(helper.fromHREF(msg, helper.tempFileUrlFromNode(subNode)), subNode);
    helper.writeNodeToTempFile(subsubNode);
    QCOMPARE(helper.fromHREF(msg, helper.tempFileUrlFromNode(subsubNode)), subsubNode);
    helper.writeNodeToTempFile(subsubNode2);
    QCOMPARE(helper.fromHREF(msg, helper.tempFileUrlFromNode(subsubNode2)), subsubNode2);
    helper.writeNodeToTempFile(extra);
    QCOMPARE(helper.fromHREF(msg, helper.tempFileUrlFromNode(extra)), extra);
    helper.writeNodeToTempFile(subExtra);
    QCOMPARE(helper.fromHREF(msg, helper.tempFileUrlFromNode(subExtra)), subExtra);
    helper.writeNodeToTempFile(subsubExtra);
    QCOMPARE(helper.fromHREF(msg, helper.tempFileUrlFromNode(subsubExtra)), subsubExtra);
    helper.writeNodeToTempFile(subsubExtraNode);
    QCOMPARE(helper.fromHREF(msg, helper.tempFileUrlFromNode(subsubExtraNode)), subsubExtraNode);

    helper.forceCleanTempFiles();
}

QTEST_MAIN(NodeHelperTest)

