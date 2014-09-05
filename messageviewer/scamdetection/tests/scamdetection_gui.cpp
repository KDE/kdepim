/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "scamdetection_gui.h"
#include "scamdetection/scamdetectionwarningwidget.h"
#include "scamdetection/scamdetection.h"
#include <qdebug.h>

#include <KLocalizedString>

#include <QUrl>
#include <QDebug>
#include <QPointer>
#include <QVBoxLayout>
#include <QWebView>
#include <QWebPage>
#include <QPushButton>
#include <QFileDialog>
#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>
#include <QCommandLineOption>

ScamDetectionTestWidget::ScamDetectionTestWidget(const QString &filename, QWidget *parent)
    : QWidget(parent)
{
    mScamDetection = new MessageViewer::ScamDetection;

    QVBoxLayout *lay = new QVBoxLayout;
    mScamWarningWidget = new MessageViewer::ScamDetectionWarningWidget();
    mScamWarningWidget->setUseInTestApps(true);
    lay->addWidget(mScamWarningWidget);

    mWebView = new QWebView;
    connect(mWebView, &QWebView::loadFinished, this, &ScamDetectionTestWidget::slotLoadFinished);
    lay->addWidget(mWebView);

    connect(mScamDetection, &MessageViewer::ScamDetection::messageMayBeAScam, mScamWarningWidget, &MessageViewer::ScamDetectionWarningWidget::slotShowWarning);
    connect(mScamWarningWidget, &MessageViewer::ScamDetectionWarningWidget::showDetails, mScamDetection, &MessageViewer::ScamDetection::showDetails);


    mWebView->load(QUrl::fromLocalFile(filename));

    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *openFile = new QPushButton(i18n("Open html..."));
    connect(openFile, &QPushButton::clicked, this, &ScamDetectionTestWidget::slotOpenHtml);
    hbox->addWidget(openFile);
    lay->addLayout(hbox);

    setLayout(lay);
}

ScamDetectionTestWidget::~ScamDetectionTestWidget()
{
    delete mScamDetection;
}

void ScamDetectionTestWidget::slotLoadFinished()
{
    QWebFrame *mainFrame = mWebView->page()->mainFrame();
    mScamDetection->scanPage(mainFrame);
}

void ScamDetectionTestWidget::slotOpenHtml()
{
    const QString fileName = QFileDialog::getOpenFileName(0, QString(), QString(), QLatin1String("*.html"));
    if (!fileName.isEmpty()) {
        mScamWarningWidget->setVisible(false);
        mWebView->load(QUrl::fromLocalFile(fileName));
    }
}

int main (int argc, char **argv)
{
    KAboutData aboutData( QLatin1String("scamdetection_gui"), i18n("ScamDetectionTest_Gui"), QLatin1String("1.0"));
    aboutData.setShortDescription(i18n("Test for scamdetection widget"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("+[url]"), i18n("URL of an html file to be opened")));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    QString fileName;
    if (parser.positionalArguments().count()) {
        fileName = parser.positionalArguments().at(0);
    } else {
        fileName = QFileDialog::getOpenFileName(0, QString(), QString(), QLatin1String("*.html"));
    }
    if (fileName.isEmpty()) {
        return 0;
    }

    ScamDetectionTestWidget *w = new ScamDetectionTestWidget(fileName);

    w->show();
    app.exec();
    delete w;
    return 0;
}

