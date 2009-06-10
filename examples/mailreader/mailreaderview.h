/*
 * mailreaderview.h
 *
 * Copyright (C) 2007 Andras Mantia <amantia@kde.org>
 */
#ifndef MAILREADERVIEW_H
#define MAILREADERVIEW_H

#include <QtGui/QWidget>

#include "ui_mailreaderview.h"

class QPainter;
class KUrl;
class KMReaderWin;

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

private:
    Ui::mailreaderview ui_mailreaderview;
    KMReaderWin *m_readerWin;

signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const QString& text);

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption(const QString& text);

private slots:
    void switchColors();
    void settingsChanged();
};

#endif // mailreaderVIEW_H
