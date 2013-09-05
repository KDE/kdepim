/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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
#include <KLocale>

#include <QDebug>
#include <QPointer>
#include <QVBoxLayout>
#include <QWebView>
#include <QWebPage>

ScamDetectionTestWidget::ScamDetectionTestWidget(const QString &filename, QWidget *parent)
    : QWidget(parent)
{
    mScamDetection = new MessageViewer::ScamDetection;

    QVBoxLayout *lay = new QVBoxLayout;
    mScamWarningWidget = new MessageViewer::ScamDetectionWarningWidget();
    lay->addWidget(mScamWarningWidget);

    QWebView *mWebView = new QWebView;
    mWebView->load(QUrl(filename));
    lay->addWidget(mWebView);

    QWebFrame *mainFrame = mWebView->page()->mainFrame();
    mScamDetection->scanPage(mainFrame);
    connect(mScamDetection, SIGNAL(messageMayBeAScam()), mScamWarningWidget, SLOT(slotShowWarning()));

    setLayout(lay);
}

ScamDetectionTestWidget::~ScamDetectionTestWidget()
{
    delete mScamDetection;
}

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "scamdetection_gui", 0, ki18n("ScamDetectionTest_Gui"),
                       "1.0", ki18n("Test for scamdetection widget"));

    KApplication app;

    const QString fileName = KFileDialog::getOpenFileName();
    if (fileName.isEmpty()) {
        return 0;
    }

    ScamDetectionTestWidget *w = new ScamDetectionTestWidget(fileName);

    w->show();
    app.exec();
    delete w;
    return 0;
}

