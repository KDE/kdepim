/*
    knodeiface.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2003 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/
#ifndef KNODEIFACE_H
#define KNODEIFACE_H


// no forward declarations - dcopidl2cpp won't work
#include <dcopobject.h>
#include <dcopref.h>
#include <kurl.h>
#include <qstringlist.h>


class KNodeIface : virtual public DCOPObject
{
  K_DCOP

k_dcop:
  /**
   * Move to the next article
   */
  virtual void nextArticle() = 0;

  /**
   * Move to the previous article
   */
  virtual void previousArticle() = 0;

  /**
   * Move to the next unread article
   */
  virtual void nextUnreadArticle() = 0;

  /**
   * Move to the next unread thread
   */
  virtual void nextUnreadThread() = 0;

  /**
   * Move to the next group
   */
  virtual void nextGroup() = 0;

  /**
   * Move to the previous group
   */
  virtual void previousGroup() = 0;

  /* Group options */

  /**
   * Open the editor to post a new article in the selected group
   */
  virtual void postArticle() = 0;

  /**
   * Fetch the new headers in the selected groups
   */
  virtual void fetchHeadersInCurrentGroup() = 0;

  /**
   * Expire the articles in the current group
   */
  virtual void expireArticlesInCurrentGroup() = 0;

  /**
   * Mark all the articles in the current group as read
   */
  virtual void markAllAsRead() = 0;

  /**
   * Mark all the articles in the current group as unread
   */
  virtual void markAllAsUnread() = 0;

  /* Header view */

  /**
   * Mark the current article as read
   */
  virtual void markAsRead() = 0;

  /**
   * Mark the current article as unread
   */
  virtual void markAsUnread() = 0;

  /**
   * Mark the current thread as read
   */
  virtual void markThreadAsRead() = 0;

  /**
   * Mark the current thread as unread
   */
  virtual void markThreadAsUnread() = 0;

  /* Articles */

  /**
   * Send the pending articles
   */
  virtual void sendPendingMessages() = 0;

  /**
   * Delete the current article
   */
  virtual void deleteArticle() = 0;

  /**
   * Send the current article
   */
  virtual void sendNow() = 0;

  /**
   * Edit the current article
   */
  virtual void editArticle() = 0;

  /**
   * Fetch all the new article headers
   */
  virtual void fetchHeaders() = 0;

  /**
   * Expire articles in all groups
   */
  virtual void expireArticles() = 0;

  /* Kontact integration */
  /**
   * Process command-line options
   * Return true if args were handled, false if there were no args.
   */
  virtual bool handleCommandLine() = 0;

};

#endif
