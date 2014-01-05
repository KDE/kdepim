/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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
#include <kdebug.h>
#include <kapplication.h>
#include <KFileDialog>
#include <KCmdLineArgs>
#include <KLocalizedString>

#include <QDebug>
#include <QPointer>
#include <QVBoxLayout>
#include <QWebView>
#include <QWebPage>
#include <QPushButton>

ScamDetectionTestWidget::ScamDetectionTestWidget(const QString &filename, QWidget *parent)
    : QWidget(parent)
{
    mScamDetection = new MessageViewer::ScamDetection;

    QVBoxLayout *lay = new QVBoxLayout;
    mScamWarningWidget = new MessageViewer::ScamDetectionWarningWidget();
    mScamWarningWidget->setUseInTestApps(true);
    lay->addWidget(mScamWarningWidget);

    mWebView = new QWebView;
    connect(mWebView, SIGNAL(loadFinished(bool)), SLOT(slotLoadFinished()));
    lay->addWidget(mWebView);

    connect(mScamDetection, SIGNAL(messageMayBeAScam()), mScamWarningWidget, SLOT(slotShowWarning()));
    connect(mScamWarningWidget, SIGNAL(showDetails()), mScamDetection, SLOT(showDetails()));


    mWebView->load(QUrl(filename));

    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *openFile = new QPushButton(i18n("Open html..."));
    connect(openFile, SIGNAL(clicked()), SLOT(slotOpenHtml()));
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
    const QString fileName = KFileDialog::getOpenFileName(KUrl(), QLatin1String("*.html"));
    if (!fileName.isEmpty()) {
        mScamWarningWidget->setVisible(false);
        mWebView->load(QUrl(fileName));
    }
}

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "scamdetection_gui", 0, ki18n("ScamDetectionTest_Gui"),
                       "1.0", ki18n("Test for scamdetection widget"));

    KCmdLineOptions option;
    option.add("+[url]", ki18n("URL of an html file to be opened"));
    KCmdLineArgs::addCmdLineOptions(option);


    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QString fileName;
    if (args->count()) {
        fileName = args->url(0).path();
    } else {
        fileName = KFileDialog::getOpenFileName(KUrl(), QLatin1String("*.html"));
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

