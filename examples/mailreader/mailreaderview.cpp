/*
 * mailreaderview.cpp
 *
 * Copyright (C) 2008 Andras Mantia <amantia@kde.org>
 */
#include "mailreaderview.h"
#include "settings.h"
#include "kmreaderwin.h"
#include <KXmlGuiWindow>

#include <klocale.h>
#include <QtGui/QLabel>
#include <QHBoxLayout>

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

#include "mailreaderview.moc"
