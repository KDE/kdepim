/*
    KNode, the KDE newsreader
    Copyright (c) 2004-2005 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNCONFIGPAGES_H
#define KNCONFIGPAGES_H


#include <kcmodule.h>

class BaseWidget;
class IdentityWidget;

namespace KNode {

/**
 * A tab-based KCModule container.
 * It simply forwards load and save operations to all tabs.
 * Code mostly taken from kmail.
 */
class KDE_EXPORT KCMTabContainer : public KCModule
{
  public:
    /** Create a new tab-based KCModule container.
     * @param parent The parent widget.
     */
    KCMTabContainer( KInstance *inst,QWidget * parent = 0 );

    /** Reimplemented to forward load() to all tabs. */
    virtual void load();
    /** Reimplemented to forward save() to all tabs. */
    virtual void save();
    /** Reimplemented to forward defaults() to the current tab. */
    virtual void defaults();

  protected:
    /** Add a new tab.
     * @param tab A KCModule to add.
     * @param title The tab title.
     */
    void addTab( KCModule* tab, const QString & title );

  private:
    QTabWidget *mTabWidget;

};


/** Accounts config page. */
class AccountsPage : public KCMTabContainer {
  Q_OBJECT

  public:
    AccountsPage( KInstance *inst, QWidget *parent = 0 );
};


/** Read news page. */
class KDE_EXPORT ReadNewsPage : public KCMTabContainer {
  Q_OBJECT

  public:
    ReadNewsPage( KInstance *inst,QWidget *parent = 0 );
};

/** Post news page. */
class KDE_EXPORT PostNewsPage : public KCMTabContainer {
  Q_OBJECT

  public:
    PostNewsPage( KInstance *inst, QWidget *parent = 0 );
};


} //KNode

#endif //KNCONFIGPAGES_H
