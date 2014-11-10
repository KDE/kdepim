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

#include "adblockblockableitemdialog_gui.h"
#include "adblock/adblockblockableitemswidget.h"
#include "libkdepim/widgets/progressindicatorlabel.h"
#include <qdebug.h>

#include <KLocalizedString>
#include <QUrl>

#include <QDebug>
#include <QPointer>
#include <QVBoxLayout>
#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include <QPushButton>
#include <QFileDialog>
#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>
#include <QCommandLineOption>

AdBlockBlockableItemTestDialog::AdBlockBlockableItemTestDialog(const QString &filename, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;

    mWidget = new MessageViewer::AdBlockBlockableItemsWidget;
    lay->addWidget(mWidget);

    mProgress = new KPIM::ProgressIndicatorLabel(i18n("Rendering page..."));
    lay->addWidget(mProgress);

    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *openFile = new QPushButton(i18n("Open html..."));
    connect(openFile, &QPushButton::clicked, this, &AdBlockBlockableItemTestDialog::slotOpenHtml);
    hbox->addWidget(openFile);
    lay->addLayout(hbox);

    setLayout(lay);
    connect(&page, &QWebPage::loadFinished, this, &AdBlockBlockableItemTestDialog::slotLoadFinished);
    mProgress->start();
    page.mainFrame()->load(QUrl::fromLocalFile(filename));
}

AdBlockBlockableItemTestDialog::~AdBlockBlockableItemTestDialog()
{
}

void AdBlockBlockableItemTestDialog::slotLoadFinished()
{
    mProgress->stop();
    mWidget->setWebFrame(page.mainFrame());
}

void AdBlockBlockableItemTestDialog::slotOpenHtml()
{
    const QString fileName = QFileDialog::getOpenFileName(0, QString(), QString(), QLatin1String("*.html"));
    if (!fileName.isEmpty()) {
        mProgress->start();
        page.mainFrame()->load(QUrl::fromLocalFile(fileName));
    }
}

int main(int argc, char **argv)
{
    KAboutData aboutData(QLatin1String("adblockblockableitemtest_gui"), i18n("adblockblockableitemtest_Gui"), QLatin1String("1.0"));
    aboutData.setShortDescription(i18n("Test for adblockblokabledialog"));
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

    AdBlockBlockableItemTestDialog *w = new AdBlockBlockableItemTestDialog(fileName);

    w->resize(800, 600);
    w->show();
    app.exec();
    delete w;
    return 0;
}

