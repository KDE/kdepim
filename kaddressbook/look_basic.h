/* -*- C++ -*-
   This file implements the base class for kab´s looks.

   the KDE addressbook

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2001, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org $
   $ License: GPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Revision$
*/

#ifndef LOOK_KABBASIC_H
#define LOOK_KABBASIC_H

#include <kabc/addressbook.h>
#include <qwidget.h>

class KConfig;

/** This is a pure virtual base class that defines the
 *  interface for how to display and change entries of
 *  the KDE addressbook.
 *
 *  This basic widget does not show anything in its client space.
 *  Derive it and implement its look and how the user may edit the
 *  entry.
 *
 *  The paintEvent() has to paint the whole widget, since repaint()
 *  calls will not delete the widgets background. */
class KABBasicLook : public QWidget
{
    Q_OBJECT
public:
    /** The constructor. */
    KABBasicLook(QWidget* parent=0, const char* name=0);
    /** Set the entry. It will be displayed automatically. */
    virtual void setEntry(const KABC::Addressee& addressee);
    /** Get the current entry. */
    virtual KABC::Addressee entry();
    /** Configure the view from the configuration file. */
    virtual void configure(KConfig* config);
    /** Retrieve read-write state. */
    bool readonly() const;
signals:
    /** This signal is emitted when the user changed the entry. */
    void entryChanged();
    /** This signal indicates that the entry needs to be changed
        immidiately in the database. This might be due to changes in
        values that are available in menus. */
    void saveMe();
    /** The user acticated the email address displayed. This may happen
        by, for example, clicking on the displayed mailto-URL. */
    void sendEmail(const QString&);
    /** The user activated one of the displayed HTTP URLs. For example
        by clicking on the displayed homepage address. */
    void browse(const QString&);
public slots:
    /** Set read-write state. */
    virtual void setReadonly(bool state);
protected:
    /** The displayed entry. */
    KABC::Addressee current;
    /** Read-Only? */
    bool m_ro;

};

class KABLookFactory
{
public:
    KABLookFactory(QWidget* parent=0, const char* name=0);
    virtual ~KABLookFactory();
    virtual KABBasicLook *create()=0;
    /** Overload this method to provide a one-liner description
        for your look. */
    virtual QString description()=0;
protected:
    QWidget *parent;
    const char* name;
};

#endif // LOOK_KABBASIC_H
