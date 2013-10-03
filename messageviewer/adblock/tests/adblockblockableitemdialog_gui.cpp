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

#include "adblockblockableitemdialog_gui.h"
#include "adblock/adblockblockableitemswidget.h"
#include "KPIMUtils/ProgressIndicatorLabel"
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
#include <QWebFrame>
#include <QPushButton>

AdBlockBlockableItemTestDialog::AdBlockBlockableItemTestDialog(const QString &filename, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;

    mWidget = new MessageViewer::AdBlockBlockableItemsWidget;
    lay->addWidget(mWidget);

    mProgress = new KPIMUtils::ProgressIndicatorLabel(i18n("Rendering page..."));
    lay->addWidget(mProgress);

    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *openFile = new QPushButton(i18n("Open html..."));
    connect(openFile, SIGNAL(clicked()), SLOT(slotOpenHtml()));
    hbox->addWidget(openFile);
    lay->addLayout(hbox);

    setLayout(lay);
    connect(&page, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished()));
    mProgress->start();
    page.mainFrame()->load(QUrl(filename));
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
    const QString fileName = KFileDialog::getOpenFileName(KUrl(), QLatin1String("*.html"));
    if (!fileName.isEmpty()) {
        mProgress->start();
        page.mainFrame()->load(QUrl(fileName));
    }
}

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "adblockblockableitemtest_gui", 0, ki18n("adblockblockableitemtest_Gui"),
                       "1.0", ki18n("Test for adblockblokabledialog"));

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

    AdBlockBlockableItemTestDialog *w = new AdBlockBlockableItemTestDialog(fileName);

    w->resize(800, 600);
    w->show();
    app.exec();
    delete w;
    return 0;
}

