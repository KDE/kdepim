/*
 * mailreaderview.cpp
 *
 * Copyright (C) 2009 Andras Mantia <amantia@kde.org>
 */
#include "mailreaderview.h"
#include "settings.h"
#include "kmreaderwin.h"
#include <KXmlGuiWindow>
#include <KConfigDialog>

#include <klocale.h>
#include <QtGui/QLabel>
#include <QHBoxLayout>
#include <KDebug>
#include <KApplication>

#include <akonadi/item.h>
#include "messagelist_settings.h"
#include "ui_prefs_messagelist.h"

mailreaderView::mailreaderView(QWidget *parent)
{
    ui_mailreaderview.setupUi(this);
    QHBoxLayout *layout = new QHBoxLayout;
    m_readerWin = new KMReaderWin( this, KSharedConfig::openConfig("mailreaderrc"), parent, dynamic_cast<KXmlGuiWindow*>(parent)->actionCollection());
    layout->addWidget(m_readerWin);
    setLayout(layout);
    setAutoFillBackground(true);
    displayAboutPage();
    connect( m_readerWin, SIGNAL(urlClicked(const KUrl&, int)), this, SLOT(urlClicked(const KUrl&, int)));
}

mailreaderView::~mailreaderView()
{

}

void mailreaderView::switchColors()
{
    // switch the foreground/background colors of the label
    QColor color = Settings::col_background();
    Settings::setCol_background( Settings::col_foreground() );
    Settings::setCol_foreground( color );

    settingsChanged();
}

void mailreaderView::settingsChanged()
{
}

void mailreaderView::showItem(const Akonadi::Item& item)
{
  kDebug() << "Show item with ID: " << item.id();
  m_readerWin->enableMessageDisplay();
  m_readerWin->setDecryptMessageOverwrite( false );
  m_readerWin->setMessageItem(item, KMReaderWin::Force);
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
    .subs( "0.1" ); // Akonadi Mail Reader version

   m_readerWin->displaySplashPage( info.toString() );
}

void mailreaderView::urlClicked( const KUrl& url, int button)
{
  kDebug() << url << " clicked in the mail viewer";
}

void mailreaderView::slotConfigure()
{
  if(KConfigDialog::showDialog("mailviewersettings"))
     return;
  KConfigDialog *dialog = new KConfigDialog( this, "mailviewersettings", m_readerWin->configObject() );
  QWidget* widget = m_readerWin->configWidget();
  dialog->addPage( widget, i18n("Viewer"), "kmail");

  QWidget *messageListConfig = new QWidget(dialog);
  Ui::MessageListConfig ui;
  ui.setupUi(messageListConfig);
  dialog->addPage(messageListConfig,
                  MessageList::Settings::self(),
                  i18n("Message List"),
                  "kmail");

  connect( dialog, SIGNAL(settingsChanged(const QString& )),
         widget, SLOT(slotSettingsChanged()) );
  dialog->setAttribute( Qt::WA_DeleteOnClose );
  dialog->show();
}

#include "mailreaderview.moc"
