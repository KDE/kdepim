/*
 * mailreader.cpp
 *
 * Copyright (C) 2008 Andras Mantia <amantia@kde.org>
 */
#include "mailreader.h"
#include "mailreaderview.h"
#include "settings.h"

#include <kconfigdialog.h>
#include <kstatusbar.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <KComboBox>
#include <KToolBar>

#include <KDE/KLocale>

#include <akonadi/collectionfetchjob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>

mailreader::mailreader()
    : KXmlGuiWindow(),
      m_view(new mailreaderView(this)),
      m_itemIndex(0)
{
    // accept dnd
    setAcceptDrops(true);

    // tell the KXmlGuiWindow that this is indeed the main widget
    setCentralWidget(m_view);

    // then, setup our actions
    setupActions();

    // add a status bar
    statusBar()->show();

    // a call to KXmlGuiWindow::setupGUI() populates the GUI
    // with actions, using KXMLGUI.
    // It also applies the saved mainwindow settings, if any, and ask the
    // mainwindow to automatically save settings if changed: window size,
    // toolbar position, icon size, etc.
    setupGUI();

    m_collectionCombo = new KComboBox(this);
    m_collectionCombo->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_collectionCombo->addItem("Select mailbox", -1);
    toolBar("naviToolbar")->insertWidget(m_previousMessage, m_collectionCombo);
    connect(m_collectionCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotCollectionSelected(int)));

    Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive );
    connect(job, SIGNAL(collectionsReceived(const Akonadi::Collection::List &)), SLOT(slotCollectionsReceived(const Akonadi::Collection::List &)));

}

mailreader::~mailreader()
{
}

void mailreader::setupActions()
{
    KStandardAction::quit(qApp, SLOT(closeAllWindows()), actionCollection());
    KStandardAction::preferences(m_view, SLOT(slotConfigure()), actionCollection());

    m_previousMessage = new KAction("Previous Message", this);
    actionCollection()->addAction("previous_message", m_previousMessage);
    connect(m_previousMessage, SIGNAL(triggered( bool )), SLOT(slotPreviousMessage()));
    m_nextMessage = new KAction("Next Message", this);
    actionCollection()->addAction("next_message", m_nextMessage);
    connect(m_nextMessage, SIGNAL(triggered( bool )), SLOT(slotNextMessage()));
    m_nextMessage->setEnabled(false);
    m_previousMessage->setEnabled(false);
}

void mailreader::slotCollectionsReceived(const Akonadi::Collection::List &collections)
{
 Q_FOREACH(Akonadi::Collection collection, collections) {
    if (collection.contentMimeTypes().contains("message/rfc822")) {
      m_collectionCombo->addItem(collection.name(), collection.id());
    }
  }
}

void mailreader::slotCollectionSelected(int index)
{
  m_nextMessage->setEnabled(false);
  m_previousMessage->setEnabled(false);
  qint64 id = m_collectionCombo->itemData(index).toLongLong();
  if ( id == -1 ) {
    m_view->showAboutPage();
  }
  m_items.clear();
  Akonadi::Collection c(id);
  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(c);
  connect(job, SIGNAL(itemsReceived(const Akonadi::Item::List &)), SLOT(slotItemsReceived(const Akonadi::Item::List &)));
  connect(job, SIGNAL(result(KJob*)), SLOT(slotCollectionFecthDone()));
}

void mailreader::slotItemsReceived(const Akonadi::Item::List &items)
{
  m_items = items;
  m_itemIndex = 0;
  if (!m_items.isEmpty()) {
    if (m_items.size() > 1)
      m_nextMessage->setEnabled(true);
    Akonadi::Item item = m_items.at(0);
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(item);
    job->fetchScope().fetchFullPayload();
    connect(job, SIGNAL(itemsReceived(const Akonadi::Item::List &)), SLOT(slotItemReceived(const Akonadi::Item::List &)));
  }
}

void mailreader::slotCollectionFecthDone()
{
  if (m_items.isEmpty()) {
    m_view->showAboutPage();
  }
}

void mailreader::slotItemReceived(const Akonadi::Item::List &items)
{
  if (!items.isEmpty()) {
    Akonadi::Item item = items.at(0);
    m_view->showItem(item);
  }
}


void mailreader::slotPreviousMessage()
{
  if (m_itemIndex > 0 && !m_items.isEmpty()) {
    m_nextMessage->setEnabled(true);
    m_itemIndex--;
    if (m_itemIndex < 1)
      m_previousMessage->setEnabled(false);
    Akonadi::Item item = m_items.at(m_itemIndex);
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(item);
    job->fetchScope().fetchFullPayload();
    connect(job, SIGNAL(itemsReceived(const Akonadi::Item::List &)), SLOT(slotItemReceived(const Akonadi::Item::List &)));
  }

}

void mailreader::slotNextMessage()
{
  if (m_itemIndex + 1 < m_items.size() && !m_items.isEmpty()) {
    m_previousMessage->setEnabled(true);
    m_itemIndex++;
    if (m_itemIndex + 1 >= m_items.size())
      m_nextMessage->setEnabled(false);
    Akonadi::Item item = m_items.at(m_itemIndex);
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(item);
    job->fetchScope().fetchFullPayload();
    connect(job, SIGNAL(itemsReceived(const Akonadi::Item::List &)), SLOT(slotItemReceived(const Akonadi::Item::List &)));
  }

}

#include "mailreader.moc"
