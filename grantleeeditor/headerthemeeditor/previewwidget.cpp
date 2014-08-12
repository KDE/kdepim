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

#include "previewwidget.h"
#include "themeeditorutil.h"
#include "messageviewer/viewer/viewer.h"
#include "messageviewer/header/headerstrategy.h"
#include "messageviewer/header/grantleeheaderteststyle.h"

#include <QPushButton>
#include <KLocale>
#include <KConfigGroup>


#include <QVBoxLayout>
#include <KSharedConfig>

PreviewWidget::PreviewWidget(const QString &projectDirectory, QWidget *parent)
    : GrantleeThemeEditor::PreviewWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    mViewer = new MessageViewer::Viewer(this);
    mGrantleeHeaderStyle = new MessageViewer::GrantleeHeaderTestStyle;
    mGrantleeHeaderStyle->setAbsolutePath(projectDirectory);
    //Default
    mGrantleeHeaderStyle->setMainFilename(QLatin1String("header.html"));
    mViewer->setHeaderStyleAndStrategy(mGrantleeHeaderStyle,
                                       MessageViewer::HeaderStrategy::create(QString()));
    lay->addWidget(mViewer);
    QPushButton *update = new QPushButton(i18n("Update view"));
    connect(update, SIGNAL(clicked(bool)), this, SIGNAL(needUpdateViewer()));
    lay->addWidget(update);
    setLayout(lay);
    loadConfig();
}

PreviewWidget::~PreviewWidget()
{
    delete mGrantleeHeaderStyle;
}

void PreviewWidget::slotExtraHeaderDisplayChanged(const QStringList &headers)
{
    mGrantleeHeaderStyle->setExtraDisplayHeaders(headers);
    updateViewer();
}

void PreviewWidget::slotMainFileNameChanged(const QString &filename)
{
    mGrantleeHeaderStyle->setMainFilename(filename);
    updateViewer();
}

void PreviewWidget::loadConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    if (config->hasGroup(QLatin1String("Global"))) {
        KConfigGroup group = config->group(QLatin1String("Global"));
        mDefaultEmail = group.readEntry("defaultEmail", themeeditorutil::defaultMail()).toLatin1();
    } else {
        mDefaultEmail = themeeditorutil::defaultMail().toLatin1();
    }
    updateViewer();
}

void PreviewWidget::updateViewer()
{
    KMime::Message *msg = new KMime::Message;
    msg->setContent( mDefaultEmail );
    msg->parse();
    mViewer->setPrinting(mPrinting);
    mViewer->setMessage(KMime::Message::Ptr(msg));
}

void PreviewWidget::createScreenShot(const QStringList &fileList)
{
    mViewer->saveMainFrameScreenshotInFile(fileList.at(0));
}

void PreviewWidget::setThemePath(const QString &projectDirectory, const QString &mainPageFileName)
{
    mGrantleeHeaderStyle->setAbsolutePath(projectDirectory);
    mGrantleeHeaderStyle->setMainFilename(mainPageFileName);
    updateViewer();
}

