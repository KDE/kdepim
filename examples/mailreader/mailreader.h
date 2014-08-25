/*
 * mailreader.h
 *
 * Copyright (C) 2008 Andras Mantia <amantia@kde.org>
 */
#ifndef MAILREADER_H
#define MAILREADER_H

#include <kxmlguiwindow.h>

#include <AkonadiCore/collection.h>
#include <AkonadiCore/item.h>

class mailreaderView;
class KToggleAction;
class KUrl;
class KComboBox;
class QAction;
class KJob;

namespace MessageList
{
class Pane;
}

/**
 * This class serves as the main window for mailreader.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Andras Mantia <amantia@kde.org>
 * @version 0.1
 */
class mailreader : public KXmlGuiWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    mailreader();

    /**
     * Default Destructor
     */
    virtual ~mailreader();

private slots:
    void slotMessageSelected(const Akonadi::Item &item);
    void slotPreviousMessage();
    void slotNextMessage();
    void itemsReceived(const Akonadi::Item::List &list);
    void itemFetchDone(KJob *job);

private:
    void setupDocks();
    void setupActions();

private:
    mailreaderView *m_view;
    MessageList::Pane *m_messagePane;

    QAction *m_nextMessage;
    QAction *m_previousMessage;
};

#endif // _MAILREADER_H_
