/*
    knconfigmanager.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2004 the KNode authors.
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


class KNConfigManager : QObject {

  Q_OBJECT

  public:
    KNConfigManager(QObject *p=0, const char *n=0);
    ~KNConfigManager();

    KNConfig::Identity*             identity() const           { return i_dentity; }
    KNConfig::Appearance*           appearance()const          { return a_ppearance; }
    KNConfig::ReadNewsGeneral*      readNewsGeneral()const     { return r_eadNewsGeneral; }
    KNConfig::ReadNewsNavigation*   readNewsNavigation()const  { return r_eadNewsNavigation; }
    KNConfig::ReadNewsViewer*       readNewsViewer()const      { return r_eadNewsViewer; }
    KNConfig::DisplayedHeaders*     displayedHeaders()const    { return d_isplayedHeaders; }
    KNConfig::Scoring*              scoring()const             { return s_coring; }
    KNConfig::PostNewsTechnical*    postNewsTechnical()const   { return p_ostNewsTechnical; }
    KNConfig::PostNewsComposer*     postNewsComposer() const   { return p_ostNewsCompose; }
    KNConfig::Cleanup*              cleanup()const             { return c_leanup; }
    //KNConfig::Cache*                cache()const               { return c_ache; }

    void configure();
    void syncConfig();

  protected:
    KNConfig::Identity             *i_dentity;
    KNConfig::Appearance           *a_ppearance;
    KNConfig::ReadNewsGeneral      *r_eadNewsGeneral;
    KNConfig::ReadNewsNavigation   *r_eadNewsNavigation;
    KNConfig::ReadNewsViewer       *r_eadNewsViewer;
    KNConfig::DisplayedHeaders     *d_isplayedHeaders;
    KNConfig::Scoring              *s_coring;
    KNConfig::PostNewsTechnical    *p_ostNewsTechnical;
    KNConfig::PostNewsComposer     *p_ostNewsCompose;
    KNConfig::Cleanup              *c_leanup;
    //KNConfig::Cache                *c_ache;

    KNConfigDialog  *d_ialog;

  protected slots:
    void slotDialogDone();

};


class KNConfigDialog : public KCMultiDialog {

  Q_OBJECT

  public:
    KNConfigDialog(QWidget *p=0, const char *n=0);

  protected slots:
    void slotConfigCommitted();

};

#endif //KNCONFIGMANAGER_H
