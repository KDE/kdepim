/***************************************************************************
              kmsystemtray.h  -  description
               -------------------
  begin                : Fri Aug 31 22:38:44 EDT 2001
  copyright            : (C) 2001 by Ryan Breen
  email                : ryan@porivo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMSYSTEMTRAY_H
#define KMSYSTEMTRAY_H

#include <ksystemtray.h>

#include <tqmap.h>
#include <tqguardedptr.h>
#include <tqvaluevector.h>
#include <tqpixmap.h>
#include <tqimage.h>

#include <time.h>

class KMFolder;
class KMMainWidget;
class TQMouseEvent;
class KPopupMenu;
class TQPoint;

/**
 * KMSystemTray extends KSystemTray and handles system
 * tray notification for KMail
 */
class KMSystemTray : public KSystemTray
{
  Q_OBJECT
public:
  /** construtor */
  KMSystemTray(TQWidget* parent=0, const char *name=0);
  /** destructor */
  ~KMSystemTray();

  void setMode(int mode);
  int mode() const;

  void hideKMail();
  bool hasUnreadMail() const;

public slots:
  void foldersChanged();

private slots:
  void updateNewMessageNotification(KMFolder * folder);
  void selectedAccount(int);
  void updateNewMessages();
  void tray_quit();

protected:
  void mousePressEvent(TQMouseEvent *);
  bool mainWindowIsOnCurrentDesktop();
  void showKMail();
  void buildPopupMenu();
  void updateCount();

  TQString prettyName(KMFolder *);

private:

  bool mParentVisible;
  TQPoint mPosOfMainWin;
  int mDesktopOfMainWin;

  int mMode;
  int mCount;
  int mNewMessagePopupId;

  KPopupMenu * mPopupMenu;
  TQPixmap mDefaultIcon;
  TQImage mLightIconImage;

  TQValueVector<KMFolder*> mPopupFolders;
  TQMap<TQGuardedPtr<KMFolder>, int> mFoldersWithUnread;
  TQMap<TQGuardedPtr<KMFolder>, bool> mPendingUpdates;
  TQTimer *mUpdateTimer;
  time_t mLastUpdate;
};

#endif
