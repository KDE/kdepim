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

#include <akonadi/item.h>

mailreaderView::mailreaderView(QWidget *parent)
{
    ui_mailreaderview.setupUi(this);
    QHBoxLayout *layout = new QHBoxLayout;
    m_readerWin = new KMReaderWin(this, parent, dynamic_cast<KXmlGuiWindow*>(parent)->actionCollection());
    layout->addWidget(m_readerWin);
    setLayout(layout);
    setAutoFillBackground(true);
    m_readerWin->displayAboutPage();
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
  m_readerWin->displayAboutPage();
}

#include "mailreaderview.moc"
