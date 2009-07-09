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
    void slotCollectionsReceived(const Akonadi::Collection::List &);
    void slotCollectionSelected(int index);
    void slotCollectionFecthDone();
    void slotItemsReceived(const Akonadi::Item::List &items);
    void slotItemReceived(const Akonadi::Item::List &items);
    void slotPreviousMessage();
    void slotNextMessage();

private:
    void setupActions();

private:
    mailreaderView *m_view;

    KComboBox *m_collectionCombo;
    KAction *m_nextMessage;
    KAction *m_previousMessage;
    Akonadi::Item::List m_items;
    int m_itemIndex;
};

#endif // _MAILREADER_H_
