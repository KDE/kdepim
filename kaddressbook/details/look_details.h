/* -*- C++ -*-
   This file implements the detailed look.

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

#ifndef LOOK_DETAILS_H
#define LOOK_DETAILS_H

#include <kaction.h>
// the common interface for looks:
#include "look_basic.h"
#include <qpixmap.h>
#include <qptrlist.h>
#include <qrect.h>
#include <qmap.h>
#include <klocale.h>
#include <kabc/addressbook.h>

class QComboBox;
class KABEntryPainter;

/** This class implements kab´s detailed view.
 *  Currently, there is no possibility to change the entry in this
 *  view. */

class KABDetailedView : public KABBasicLook
{
    Q_OBJECT
public:
    /** Enum to select how the background is drawn. */
    enum BackgroundStyle {
        /** The is no background, we use the default background
            color. */
        None,
        /** The background is displayed as a tile. */
        Tiled,
        /** This is a bordered background, paint it from top left and
            down, but do not repeat it to the right.
        */
        Bordered
    };
    /** The constructor. */
    KABDetailedView(QWidget* parent=0, const char* name=0);
    /** The virtual destructor. */
    virtual ~KABDetailedView();
    /** Set the entry.
     */
    void setEntry(const KABC::Addressee&);
    /** Overloaded from KABBasicLook */
    void setReadonly(bool);
    /** Overloaded from KABBasicLook. */
    void configure(KConfig* config);
protected:
    /** Paint it. */
    void paintEvent(QPaintEvent *);
    /** Handle mouse events. */
    void mousePressEvent(QMouseEvent*);
    /** Handle mouse movement. */
    void mouseMoveEvent(QMouseEvent*);
    /** Store locations of the URLs. */
    QPtrList<QRect> locURLs;
    /** Store locations of the email addresses. */
    QPtrList<QRect> locEmails;
    /** Store locations of the phone numbers. */
    QPtrList<QRect> locPhones;
    /** The settings for painting. */
    KABEntryPainter *epainter;
    /** A method to retrieve a background image according to the path
        stored in the entry. It is either loaded
        from backgrounds, that acts as a cache, or from the file
        and added to @see backgrounds.
    */
    bool getBackground(QString path, QPixmap& image);
    /** Map of QImages to save loaded background images into it. */
    QMap<QString, QPixmap> backgrounds;
    /** The background image used in that entry. */
    QPixmap background;
    /** The background style. */
    BackgroundStyle bgStyle;
    /** Setting: default background is a color (defaultBGColor). */
    bool useDefaultBGImage;
    /** The default background color. */
    QColor defaultBGColor;
    /** Colored headline background and text? */
    bool useHeadlineBGColor;
    /** The headline background color. */
    QColor headlineBGColor;
    /** The headline color. */
    QColor headlineTextColor;
    /** The default background image. */
    QPixmap defaultBGImage;
    /** Show addresses? */
    KToggleAction *actionShowAddresses;
    /** Show emails? */
    KToggleAction *actionShowEmails;
    /** Show telephones? */
    KToggleAction *actionShowTelephones;
    /** Show URLs? */
    KToggleAction *actionShowURLs;
    /** Used for constant distances. */
    const int Grid;
    /** Stores a list of the contents of the bordered backgrounds directory. */
    QStringList borders;
    /** Stores a list of the contents of the tiled backgrounds directory. */
    QStringList tiles;
    /** The bordered backgrounds menu. Only valid when not zero (e.g.,
        when handling a mouse click event. */
    QPopupMenu *menuBorderedBG;
    /** The tiled backgrounds menu. Only valid when not zero (e.g.,
        when handling a mouse click event. */
    QPopupMenu *menuTiledBG;
public slots:
    void slotBorderedBGSelected(int index);
    void slotTiledBGSelected(int index);
protected: // statics:
    static const QString BorderedBGDir;
    static const QString TiledBGDir;
};

class KABDetailedViewFactory : public KABLookFactory
{
public:
    KABDetailedViewFactory(QWidget* parent=0, const char* name=0)
        : KABLookFactory(parent, name) {}
    KABBasicLook *create()
        { return new KABDetailedView(parent, name); }
    QString description()
        { return i18n("Detailed Style: Display all details, no modifications."); }
};

#endif // LOOK_DETAILS_H
