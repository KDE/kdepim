/*
 * mailreader.h
 *
 * Copyright (C) 2008 Andras Mantia <amantia@kde.org>
 */
#ifndef MAILREADER_H
#define MAILREADER_H


#include <kxmlguiwindow.h>

#include "ui_prefs_base.h"

class mailreaderView;
class QPrinter;
class KToggleAction;
class KUrl;

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
    void fileNew();
    void optionsPreferences();

private:
    void setupActions();

private:
    Ui::prefs_base ui_prefs_base ;
    mailreaderView *m_view;

    QPrinter   *m_printer;
    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;
};

#endif // _MAILREADER_H_
