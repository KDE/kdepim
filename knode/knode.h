/*
    knode.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNODE_H
#define KNODE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kmainwindow.h>
#include <kprogress.h>

#include "resource.h"

class QSize;
class KAccel;
class KToggleAction;

class KNodeView;


class KNProgress : public KProgress
{
  Q_OBJECT

  public:
    KNProgress (int desiredHeight, int minValue, int maxValue, int value, KProgress::Orientation orient, QWidget *parent=0, const char *name=0);
    ~KNProgress();

    void disableProgressBar();                                      // 0% and no text
    void setProgressBar(int value,const QString& = QString::null);  // manual operation
    virtual QSize sizeHint() const;

  protected:
    int desHeight;
};


class KNMainWindow : public KMainWindow
{
  Q_OBJECT

  public:
          
    KNMainWindow();
    ~KNMainWindow();

    //GUI
    void setStatusMsg(const QString& = QString::null, int id=SB_MAIN);
    void setStatusHelpMsg(const QString& text);
    void setCursorBusy(bool b=true);
    void blockUI(bool b=true);
    void secureProcessEvents();  // processEvents with some blocking

    virtual QSize sizeHint() const;   // useful default value


  protected:

    //checks if run for the first time, sets some global defaults (email configuration)
    bool firstStart();
      
    //exit
    bool queryClose();

    //update appearance
    virtual void fontChange( const QFont & );
    virtual void paletteChange ( const QPalette & );

    KAccel      *a_ccel;
    KNodeView   *v_iew;
    KNProgress  *p_rogBar;
    bool b_lockInput;

  //---------------------------------- <Actions> ----------------------------------

    KToggleAction *a_ctWinToggleToolbar,
                  *a_ctWinToggleStatusbar;


  protected slots:
    void slotWinToggleToolbar();
    void slotWinToggleStatusbar();
    void slotConfKeys();
    void slotConfToolbar();
    void slotSettings();

  //---------------------------------- </Actions> ---------------------------------
};

#endif // KNODE_H
