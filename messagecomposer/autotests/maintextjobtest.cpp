/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "maintextjobtest.h"

#include <QTextCodec>

#include <QDebug>
#include <KIconLoader>
#include <qtest.h>

#include <kmime/kmime_content.h>
using namespace KMime;

#include <MessageComposer/Composer>
#include <MessageComposer/GlobalPart>
#include <MessageComposer/MainTextJob>
#include <MessageComposer/TextPart>
#include <MessageComposer/RichTextComposerNg>
#include <KPIMTextEdit/RichTextComposerControler>
#include <KPIMTextEdit/RichTextComposerImages>

#include <KActionCollection>

//#include <kpimtextedit/textedit.h>

using namespace MessageComposer;

QTEST_MAIN(MainTextJobTest)

void MainTextJobTest::testPlainText()
{
    Composer *composer = new Composer;
    composer->globalPart()->setGuiEnabled(false);
    QList<QByteArray> charsets;
    charsets << "us-ascii" << "utf-8";
    composer->globalPart()->setCharsets(charsets);
    TextPart *textPart = new TextPart;
    QString data = QStringLiteral("they said their nevers they slept their dream");
    textPart->setWrappedPlainText(data);
    MainTextJob *mjob = new MainTextJob(textPart, composer);
    QVERIFY(mjob->exec());
    Content *result = mjob->content();
    result->assemble();
    qDebug() << result->encodedContent();
    QVERIFY(result->contentType(false));
    QCOMPARE(result->contentType()->mimeType(), QByteArray("text/plain"));
    QCOMPARE(result->contentType()->charset(), QByteArray("us-ascii"));
    QCOMPARE(QString::fromLatin1(result->body()), data);
}

void MainTextJobTest::testWrappingErrors()
{
    {
        Composer *composer = new Composer;
        composer->globalPart()->setGuiEnabled(false);
        composer->globalPart()->setFallbackCharsetEnabled(true);
        TextPart *textPart = new TextPart;
        QString data = QStringLiteral("they said their nevers they slept their dream");
        textPart->setWordWrappingEnabled(false);
        textPart->setWrappedPlainText(data);
        MainTextJob *mjob = new MainTextJob(textPart, composer);
        QVERIFY(!mjob->exec());   // error: not UseWrapping but given only wrapped text
        QCOMPARE(mjob->error(), int(JobBase::BugError));
    }
    {
        Composer *composer = new Composer;
        composer->globalPart()->setGuiEnabled(false);
        composer->globalPart()->setFallbackCharsetEnabled(true);
        TextPart *textPart = new TextPart;
        textPart->setWordWrappingEnabled(true);
        QString data = QStringLiteral("they said their nevers they slept their dream");
        textPart->setCleanPlainText(data);
        MainTextJob *mjob = new MainTextJob(textPart, composer);
        QVERIFY(!mjob->exec());   // error: UseWrapping but given only clean text
        QCOMPARE(mjob->error(), int(JobBase::BugError));
    }
}

void MainTextJobTest::testCustomCharset()
{
    Composer *composer = new Composer;
    composer->globalPart()->setGuiEnabled(false);
    QByteArray charset("iso-8859-2");
    composer->globalPart()->setCharsets(QList<QByteArray>() << charset);
    TextPart *textPart = new TextPart;
    QString data = QStringLiteral("şi el o să se-nchidă cu o frunză de pelin");
    textPart->setWrappedPlainText(data);
    MainTextJob *mjob = new MainTextJob(textPart, composer);
    QVERIFY(mjob->exec());
    Content *result = mjob->content();
    result->assemble();
    qDebug() << result->encodedContent();
    QVERIFY(result->contentType(false));
    QCOMPARE(result->contentType()->mimeType(), QByteArray("text/plain"));
    QCOMPARE(result->contentType()->charset(), charset);
    QByteArray outData = result->body();
    QTextCodec *codec = QTextCodec::codecForName(charset);
    QVERIFY(codec);
    QCOMPARE(codec->toUnicode(outData), data);
}

void MainTextJobTest::testNoCharset()
{
    Composer *composer = new Composer;
    QVERIFY(!composer->globalPart()->isFallbackCharsetEnabled());
    composer->globalPart()->setGuiEnabled(false);
    TextPart *textPart = new TextPart;
    QString data = QStringLiteral("do you still play the accordion?");
    textPart->setWrappedPlainText(data);
    MainTextJob *mjob = new MainTextJob(textPart, composer);
    QSKIP("This tests has been failing for a long time, please someone fix it", SkipSingle);
    QVERIFY(!mjob->exec());   // Error.
    QCOMPARE(mjob->error(), int(JobBase::BugError));
    qDebug() << mjob->errorString();
}

void MainTextJobTest::testBadCharset()
{
    Composer *composer = new Composer;
    composer->globalPart()->setGuiEnabled(false);
    QByteArray charset("us-ascii");   // Cannot handle Romanian chars.
    composer->globalPart()->setCharsets(QList<QByteArray>() << charset);
    TextPart *textPart = new TextPart;
    QString data = QStringLiteral("el a plâns peste ţară cu lacrima limbii noastre");
    textPart->setWrappedPlainText(data);
    MainTextJob *mjob = new MainTextJob(textPart, composer);
    QSKIP("This tests has been failing for a long time, please someone fix it", SkipSingle);
    QVERIFY(!mjob->exec());   // Error.
    QCOMPARE(mjob->error(), int(JobBase::UserError));
    qDebug() << mjob->errorString();
}

void MainTextJobTest::testFallbackCharset()
{
    Composer *composer = new Composer;
    composer->globalPart()->setGuiEnabled(false);
    composer->globalPart()->setFallbackCharsetEnabled(true);
    TextPart *textPart = new TextPart;
    QString data = QStringLiteral("and when he falleth...");
    textPart->setWrappedPlainText(data);
    MainTextJob *mjob = new MainTextJob(textPart, composer);
    QVERIFY(mjob->exec());
    Content *result = mjob->content();
    result->assemble();
    qDebug() << result->encodedContent();
    QVERIFY(result->contentType(false));
    QCOMPARE(result->contentType()->mimeType(), QByteArray("text/plain"));
    QCOMPARE(result->contentType()->charset(), QByteArray("us-ascii"));     // Fallback is us-ascii or utf8.
    QCOMPARE(QString::fromLatin1(result->body()), data);
}

void MainTextJobTest::testHtml()
{
    QLatin1String originalHtml("<html><head></head><body>Test <em>with</em> formatting...<br>The end.</body></html>");
    MessageComposer::RichTextComposerNg editor;
    editor.createActions(new KActionCollection(this));
    editor.setTextOrHtml(originalHtml);
    QVERIFY(editor.composerControler()->isFormattingUsed());

    Composer *composer = new Composer;
    composer->globalPart()->setGuiEnabled(false);
    composer->globalPart()->setFallbackCharsetEnabled(true);
    TextPart *textPart = new TextPart;
    textPart->setWordWrappingEnabled(false);
    textPart->setCleanPlainText(editor.composerControler()->toCleanPlainText());
    textPart->setCleanHtml(editor.toCleanHtml());
    MainTextJob *mjob = new MainTextJob(textPart, composer);
    QVERIFY(mjob->exec());
    Content *result = mjob->content();
    result->assemble();
    qDebug() << result->encodedContent();

    // multipart/alternative
    {
        QVERIFY(result->contentType(false));
        QCOMPARE(result->contentType()->mimeType(), QByteArray("multipart/alternative"));
        QCOMPARE(result->contents().count(), 2);
        // text/plain
        {
            Content *plain = result->contents().at(0);
            QVERIFY(plain->contentType(false));
            QCOMPARE(plain->contentType()->mimeType(), QByteArray("text/plain"));
            QCOMPARE(QString::fromLatin1(plain->body()), editor.composerControler()->toCleanPlainText());
        }
        // text/html
        {
            Content *html = result->contents().at(1);
            QVERIFY(html->contentType(false));
            QCOMPARE(html->contentType()->mimeType(), QByteArray("text/html"));
            // The editor adds extra Html stuff, so we can't compare to originalHtml.
            QCOMPARE(QLatin1String(html->body()), editor.toCleanHtml());
        }
    }
}

void MainTextJobTest::testHtmlWithImages()
{
    KActionCollection ac(this);
    MessageComposer::RichTextComposerNg editor;
    editor.createActions(new KActionCollection(this));

    QString image1 = KIconLoader::global()->iconPath(QStringLiteral("folder-new"), KIconLoader::Small, false);
    QString image2 = KIconLoader::global()->iconPath(QStringLiteral("message"), KIconLoader::Small, false);
    QString data = QStringLiteral("dust in the wind");
    editor.setTextOrHtml(data);
    editor.composerControler()->composerImages()->addImage(QUrl::fromLocalFile(image1));
    editor.composerControler()->composerImages()->addImage(QUrl::fromLocalFile(image1));
    editor.composerControler()->composerImages()->addImage(QUrl::fromLocalFile(image2));
    editor.composerControler()->composerImages()->addImage(QUrl::fromLocalFile(image2));
    KPIMTextEdit::ImageList images = editor.composerControler()->composerImages()->embeddedImages();
    QCOMPARE(images.count(), 2);
    QString cid1 = images[0]->contentID;
    QString cid2 = images[1]->contentID;
    QString name1 = images[0]->imageName;
    QString name2 = images[1]->imageName;

    Composer *composer = new Composer;
    composer->globalPart()->setGuiEnabled(false);
    composer->globalPart()->setFallbackCharsetEnabled(true);
    TextPart *textPart = new TextPart;
    textPart->setWordWrappingEnabled(false);
    textPart->setCleanPlainText(editor.composerControler()->toCleanPlainText());
    textPart->setCleanHtml(editor.composerControler()->toCleanHtml());
    textPart->setEmbeddedImages(editor.composerControler()->composerImages()->embeddedImages());
    MainTextJob *mjob = new MainTextJob(textPart, composer);
    QVERIFY(mjob->exec());
    Content *result = mjob->content();
    result->assemble();
    qDebug() << result->encodedContent();

    // multipart/related
    {
        QVERIFY(result->contentType(false));
        QCOMPARE(result->contentType()->mimeType(), QByteArray("multipart/related"));
        QCOMPARE(result->contents().count(), 3);
        // multipart/alternative
        {
            Content *alternative = result->contents().at(0);
            QVERIFY(alternative->contentType(false));
            QCOMPARE(alternative->contentType()->mimeType(), QByteArray("multipart/alternative"));
            QCOMPARE(alternative->contents().count(), 2);
            // text/plain
            {
                Content *plain = alternative->contents().at(0);
                QCOMPARE(plain->contentType()->mimeType(), QByteArray("text/plain"));
                QCOMPARE(QString::fromLatin1(plain->body()), data);
            }
            // text/html
            {
                Content *html = alternative->contents().at(1);
                QCOMPARE(html->contentType()->mimeType(), QByteArray("text/html"));
                QString data = QString::fromLatin1(html->body());
                int idx1 = data.indexOf(QStringLiteral("cid:%1").arg(cid1));
                int idx2 = data.indexOf(QStringLiteral("cid:%1").arg(cid2));
                QVERIFY(idx1 > 0);
                QVERIFY(idx2 > 0);
                QVERIFY(idx1 < idx2);
            }
        }
        // First image/png
        {
            Content *image = result->contents().at(1);
            QVERIFY(image->contentType(false));
            QCOMPARE(image->contentType()->mimeType(), QByteArray("image/png"));
            QCOMPARE(image->contentType()->name(), name1);
            const Headers::ContentID *cid = image->header<Headers::ContentID>();
            QVERIFY(cid);
            QCOMPARE(cid->identifier(), cid1.toLatin1());
        }
        // Second image/png
        {
            Content *image = result->contents().at(2);
            QVERIFY(image->contentType(false));
            QCOMPARE(image->contentType()->mimeType(), QByteArray("image/png"));
            QCOMPARE(image->contentType()->name(), name2);
            const Headers::ContentID *cid = image->header<Headers::ContentID>();
            QVERIFY(cid);
            QCOMPARE(cid->identifier(), cid2.toLatin1());
        }
    }
}

