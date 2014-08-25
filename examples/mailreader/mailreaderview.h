/*
 * mailreaderview.h
 *
 * Copyright (C) 2009 Andras Mantia <amantia@kde.org>
 */
#ifndef MAILREADERVIEW_H
#define MAILREADERVIEW_H

#include <QWidget>

#include "ui_mailreaderview.h"

class QPainter;
class KUrl;

namespace MessageViewer
{
class Viewer;
}

namespace Akonadi
{
class Item;
}

/**
 * This is the main view class for mailreader.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * @short Main view
 * @author Andras Mantia <amantia@kde.org>
 * @version 0.1
 */

class mailreaderView : public QWidget, public Ui::mailreaderview
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    mailreaderView(QWidget *parent);

    /**
     * Destructor
     */
    virtual ~mailreaderView();

    void showItem(const Akonadi::Item &item);
    void showAboutPage();

private:
    void displayAboutPage();

    Ui::mailreaderview ui_mailreaderview;
    MessageViewer::Viewer *m_readerWin;

public slots:
    void slotConfigure();

signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const QString &text);

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption(const QString &text);

private slots:
    void switchColors();
    void settingsChanged();
    void urlClicked(const Akonadi::Item &, const KUrl &);
};

#endif // mailreaderVIEW_H
