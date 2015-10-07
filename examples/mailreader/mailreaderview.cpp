/*
 * mailreaderview.cpp
 *
 * Copyright (C) 2009 Andras Mantia <amantia@kde.org>
 */
#include "mailreaderview.h"
#include "settings.h"
#include "viewer/viewer.h"
#include "viewer/attachmentstrategy.h"
#include <settings/messageviewersettings.h>
#include <KXmlGuiWindow>
#include <KConfigDialog>

#include <KLocalizedString>
#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>
#include <QUrl>

#include <AkonadiCore/item.h>
#include "messagelistsettings.h"
#include "ui_prefs_messagelist.h"

mailreaderView::mailreaderView(QWidget *parent)
{
    ui_mailreaderview.setupUi(this);
    QHBoxLayout *layout = new QHBoxLayout;
    m_readerWin = new MessageViewer::Viewer(this, parent, dynamic_cast<KXmlGuiWindow *>(parent)->actionCollection());
    m_readerWin->setAttachmentStrategy(MessageViewer::AttachmentStrategy::inlined());
    layout->addWidget(m_readerWin);
    setLayout(layout);
    setAutoFillBackground(true);
    displayAboutPage();
    connect(m_readerWin, SIGNAL(urlClicked(Akonadi::Item,QUrl)), this,
            SLOT(urlClicked(Akonadi::Item,QUrl)));
}

mailreaderView::~mailreaderView()
{

}

void mailreaderView::switchColors()
{
    // switch the foreground/background colors of the label
    QColor color = Settings::col_background();
    Settings::setCol_background(Settings::col_foreground());
    Settings::setCol_foreground(color);

    settingsChanged();
}

void mailreaderView::settingsChanged()
{
}

void mailreaderView::showItem(const Akonadi::Item &item)
{
    qDebug() << "Show item with ID: " << item.id();
    m_readerWin->enableMessageDisplay();
    m_readerWin->setDecryptMessageOverwrite(false);
    m_readerWin->setMessageItem(item, MessageViewer::Viewer::Force);
}

void mailreaderView::showAboutPage()
{
    displayAboutPage();
}

void mailreaderView::displayAboutPage()
{
    KLocalizedString info =
        ki18nc("%1: Mailreader version;"
               "--- end of comment ---",
               "<h2 style='margin-top: 0px;'>Welcome to Mailreader %1</h2>"
               "<p>Mailread is a proof of concept reader for the Akonadi/KMime framework.</p>\n"
               "<p style='margin-bottom: 0px'>&nbsp; &nbsp; <a href='http://pim.kde.org/akonadi'>The Akonadi Team</a></p>")
        .subs(QLatin1String("0.1"));   // Akonadi Mail Reader version

    m_readerWin->displaySplashPage(info.toString());
}

void mailreaderView::urlClicked(const Akonadi::Item &item , const QUrl &url)
{
    Q_UNUSED(item);
    //TODO "Handle click"
    qDebug() << url << " clicked in the mail viewer";
}

void mailreaderView::slotConfigure()
{
    if (KConfigDialog::showDialog(QLatin1String("mailviewersettings"))) {
        return;
    }
    KConfigDialog *dialog = new KConfigDialog(this, QLatin1String("mailviewersettings"), MessageViewer::MessageViewerSettings::self());
    QWidget *widget = m_readerWin->configWidget();
    dialog->addPage(widget, i18n("Viewer"), QLatin1String("kmail"));

    QWidget *messageListConfig = new QWidget(dialog);
    Ui::MessageListConfig ui;
    ui.setupUi(messageListConfig);
    dialog->addPage(messageListConfig,
                    MessageList::MessageListSettings::self(),
                    i18n("Message List"),
                    QLatin1String("kmail"));

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

