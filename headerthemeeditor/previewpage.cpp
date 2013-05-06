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

#include "previewpage.h"
#include "themeeditorutil.h"
#include "messageviewer/viewer.h"
#include <KMime/Message>
#include <KPushButton>
#include <KLocale>
#include <KConfigGroup>
#include <QVBoxLayout>

PreviewPage::PreviewPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    mViewer = new MessageViewer::Viewer(this);
    lay->addWidget(mViewer);
    KPushButton *update = new KPushButton(i18n("Update view"));
    connect(update, SIGNAL(clicked(bool)),SLOT(slotUpdateViewer()));
    lay->addWidget(update);
    setLayout(lay);
    loadConfig();
}

PreviewPage::~PreviewPage()
{
}

void PreviewPage::loadConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    if (config->hasGroup(QLatin1String("Global"))) {
        KConfigGroup group = config->group(QLatin1String("Global"));
        mDefaultEmail = group.readEntry("defaultEmail", themeeditorutil::defaultMail()).toLatin1();
    } else {
        mDefaultEmail = themeeditorutil::defaultMail().toLatin1();
    }
    slotUpdateViewer();
}

void PreviewPage::slotUpdateViewer()
{
    KMime::Message *msg = new KMime::Message;
    msg->setContent( mDefaultEmail );
    msg->parse();
    mViewer->setMessage(KMime::Message::Ptr(msg));
}

void PreviewPage::createScreenShot(const QString &fileName)
{
    mViewer->saveMainFrameScreenshotInFile(fileName);
}

#include "previewpage.moc"
