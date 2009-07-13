/*
 * mailreaderview.cpp
 *
 * Copyright (C) 2009 Andras Mantia <amantia@kde.org>
 */
#include "mailreaderview.h"
#include "settings.h"
#include "kmreaderwin.h"
#include <KXmlGuiWindow>

#include <klocale.h>
#include <QtGui/QLabel>
#include <QHBoxLayout>
#include <KDebug>
#include <KApplication>

#include <akonadi/item.h>

mailreaderView::mailreaderView(QWidget *parent)
{
    ui_mailreaderview.setupUi(this);
    QHBoxLayout *layout = new QHBoxLayout;
    m_readerWin = new KMReaderWin( this, KSharedConfig::openConfig(KApplication::kApplication()->sessionConfig()->componentData(), KApplication::kApplication()->sessionConfig()->name()), parent, dynamic_cast<KXmlGuiWindow*>(parent)->actionCollection());
    layout->addWidget(m_readerWin);
    setLayout(layout);
    setAutoFillBackground(true);
    displayAboutPage();
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

void mailreaderView::showItem(Akonadi::Item& item)
{
  kDebug() << "Show item with ID: " << item.id();
  m_readerWin->enableMsgDisplay();
  m_readerWin->setDecryptMessageOverwrite( false );
  m_readerWin->setMessageItem(item, true);
}

void mailreaderView::showAboutPage()
{
  displayAboutPage();
}

void mailreaderView::displayAboutPage()
{
  KLocalizedString info =
    ki18nc("%1: Mailreader version; %2: help:// URL; %3: homepage URL; "
         "%4: generated list of new features; "
         "%5: First-time user text (only shown on first start); "
         "%6: generated list of important changes; "
         "--- end of comment ---",
         "<h2 style='margin-top: 0px;'>Welcome to Mailreader %1</h2><p>Mailread is the proof of concept reader for the K "
         "Desktop Environment using the Akonadi/KMime framework."
         "</p>\n"
         "<p style='margin-bottom: 0px'>&nbsp; &nbsp; The Akonadi Team</p>")
           .subs( "0.1" /*FIXME KMAIL_VERSION*/ ) // KMail version
           .subs( "help:/kmail/index.html" ) // KMail help:// URL
           .subs( "http://kontact.kde.org/kmail/" ); // KMail homepage URL

  info = info.subs( QString() ); // remove the place holder

   m_readerWin->displaySplashPage( info.toString() );
}


#include "mailreaderview.moc"
