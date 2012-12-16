/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNCONFIGMANAGER_H
#define KNCONFIGMANAGER_H

#include <kcmultidialog.h>

#include "knconfig.h"

class KNConfigDialog;


/** Manages config objects.
 * @todo Move to the KConfigXT generated KNode::Settings class.
 */
class KNConfigManager : QObject {

  Q_OBJECT

  public:
    explicit KNConfigManager( QObject *parent = 0 );
    ~KNConfigManager();

    KNode::Appearance*           appearance()const          { return a_ppearance; }
    KNode::DisplayedHeaders*     displayedHeaders()const    { return d_isplayedHeaders; }
    KNode::Cleanup*              cleanup()const             { return c_leanup; }

    void configure();
    void syncConfig();

  protected:
    KNode::Appearance           *a_ppearance;
    KNode::DisplayedHeaders     *d_isplayedHeaders;
    KNode::Cleanup              *c_leanup;

    KNConfigDialog  *d_ialog;

  protected slots:
    void slotDialogDone();

};


/** The configuration dialog. */
class KNConfigDialog : public KCMultiDialog
{
  Q_OBJECT
  public:
    /** Create a new configuration dialog.
     * @param parent The parent widget.
     */
    explicit KNConfigDialog( QWidget *parent = 0 );

  protected slots:
    /** Update and reload configuration settings. */
    void slotConfigCommitted();

};

#endif //KNCONFIGMANAGER_H
