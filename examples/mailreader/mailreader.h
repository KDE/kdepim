/*
 * mailreader.h
 *
 * Copyright (C) 2008 Andras Mantia <amantia@kde.org>
 */
#ifndef MAILREADER_H
#define MAILREADER_H


#include <kxmlguiwindow.h>

#include <akonadi/collection.h>
#include <akonadi/item.h>

class mailreaderView;
class KToggleAction;
class KUrl;
class KComboBox;
class KAction;

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

private:
    void setupDocks();
    void setupActions();

private:
    mailreaderView *m_view;
    MessageList::Pane *m_messagePane;

    KAction *m_nextMessage;
    KAction *m_previousMessage;
};

#endif // _MAILREADER_H_
