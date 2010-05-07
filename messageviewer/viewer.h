/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef MESSAGEVIEWER_H
#define MESSAGEVIEWER_H

#include "messageviewer_export.h"

#include <QWidget>
#include <QTimer>
#include <QStringList>
#include <QCloseEvent>
#include <QEvent>
#include <QList>
#include <QMap>
#include <QResizeEvent>
#include <kurl.h>
#include <kservice.h>
#include <kvbox.h>

#include <kmime/kmime_message.h>

//Akonadi includes
#include <akonadi/item.h>

class QAbstractItemModel;
class QSplitter;
class QTreeWidgetItem;
class QString;
class QTextCodec;
class QTreeView;
class QModelIndex;

class KHBox;
class KActionCollection;
class KAction;
class KSelectAction;
class KToggleAction;
class KToggleAction;
class KUrl;
class KConfigSkeleton;

namespace Akonadi {
  class ItemFetchJob;
}

namespace MessageViewer {
  class AttachmentStrategy;
  class ViewerPrivate;
  class CSSHelper;
  class HeaderStrategy;
  class HeaderStyle;

//TODO(Andras) once only those methods are public that really need to be public, probably export the whole class instead of just some methods

/**
 * This is the main widget for the viewer.
 * See the documentation of ViewerPrivate for implementation details.
 * See Mainpage.dox for an overview of the classes in the messageviewer library.
 */
class MESSAGEVIEWER_EXPORT Viewer: public QWidget
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(Viewer)

  public:
  /**
   * Create a mail viewer widget
   * @param parent parent widget
   * @param mainWindow the application's main window
   * @param actionCollection the action collection where the widget's actions will belong to
   * @param f window flags
   */
  Viewer( QWidget *parent, QWidget *mainWindow = 0, KActionCollection *actionCollection = 0,
          Qt::WindowFlags f = 0 );
  virtual ~Viewer();

  /**
   * Returns the current message displayed in the viewer.
   */
  KMime::Message::Ptr message() const;

  /**
   * Returns the current message item displayed in the viewer.
   */
  Akonadi::Item messageItem() const;

  enum AttachmentAction
  {
    Open = 1,
    OpenWith = 2,
    View = 3,
    Save = 4,
    Properties = 5,
    ChiasmusEncrypt = 6,
    Delete = 7,
    Edit = 8,
    Copy = 9,
    ScrollTo = 10
  };

  /**
   * The display update mode: Force updates the display immediately, Delayed updates
   * after some time (150ms by default
   */
  enum UpdateMode {
    Force = 0,
    Delayed
  };

  /**
   * Set the message that shall be shown.
   * @param msg - the message to be shown. If 0, an empty page is displayed.
   * @param updateMode - update the display immediately or not. See UpdateMode.
   */
  void setMessage( KMime::Message::Ptr message, UpdateMode updateMode = Delayed );

  /**
   * Set the Akonadi item that will be displayed.
   * @param item - the Akonadi item to be displayed. If it doesn't hold a mail (KMime::Message::Ptr as payload data),
   *               an empty page is shown.
   * @param updateMode - update the display immediately or not. See UpdateMode.
   */
  void setMessageItem( const Akonadi::Item& item, UpdateMode updateMode = Delayed );

  /**
   * The path to the message in terms of Akonadi collection hierarchy.
   */
  QString messagePath() const;

  /**
   * Set the path to the message in terms of Akonadi collection hierarchy.
   */
  void setMessagePath( const QString &path );

  /**
   * Instead of settings a message to be shown sets a message part
   * to be shown
   */
  void setMessagePart( KMime::Content* aMsgPart, bool aHTML,
                   const QString& aFileName, const QString& pname );

  /**
   * Convenience method to clear the reader and discard the current message. Sets the internal message pointer
   * returned by message() to 0.
   * @param updateMode - update the display immediately or not. See UpdateMode.
   */
  void clear( UpdateMode updateMode = Delayed ) { setMessage( KMime::Message::Ptr(), updateMode ); }

  void update( UpdateMode updateMode = Delayed );

  /**
   * Sets a message as the current one and print it immediately.
   * @param message the message to display and print
   */
  void printMessage( KMime::Message::Ptr message );
  void printMessage( const Akonadi::Item &msg );

  /** Print the currently displayed message */
  void print();

  /** Return selected text */
  QString selectedText();

  /** Get the html override setting */
  bool htmlOverride() const;

  /** Override default html mail setting */
  void setHtmlOverride( bool override );

  /** Get the load external references override setting */
  bool htmlLoadExtOverride() const;

  /** Override default load external references setting */
  void setHtmlLoadExtOverride( bool override );

  /** Is html mail to be supported? Takes into account override */
  bool htmlMail() const;

  /** Is loading ext. references to be supported? Takes into account override */
  bool htmlLoadExternal() const;

  /**
   * Display a generic HTML splash page instead of a message.
   * @param info - the text to be displayed in HTML format
   */
  void displaySplashPage( const QString& info );

  /** Enable the displaying of messages again after an splash (or other) page was displayed */
  void enableMessageDisplay();

  /** Returns true if the message view is scrolled to the bottom. */
  bool atBottom() const;

  bool isFixedFont() const;
  void setUseFixedFont( bool useFixedFont );

  QWidget* mainWindow();

  /** Enforce message decryption. */
  void setDecryptMessageOverwrite( bool overwrite = true );

  /**
   * Get an instance for the configuration widget. The caller has the ownership and must delete the widget. See also configObject();
   * The caller should also call the widget's slotSettingsChanged() if the configuration has changed.
   */
  QWidget* configWidget();

  /**
   * Initiates a delete, by sending a signal to delete the message item */
  void deleteMessage();

  const AttachmentStrategy * attachmentStrategy() const;
  void setAttachmentStrategy( const AttachmentStrategy * strategy );

  QString overrideEncoding() const;
  void setOverrideEncoding( const QString &encoding );
  CSSHelper* cssHelper() const;
  void setPrinting(bool enable);

  KToggleAction *toggleFixFontAction();

  KToggleAction *toggleMimePartTreeAction();

  KAction *selectAllAction();
  KAction *copyURLAction();
  KAction *copyAction();
  KAction *urlOpenAction();

  const HeaderStrategy * headerStrategy() const;

  HeaderStyle * headerStyle() const;

  void setHeaderStyleAndStrategy( HeaderStyle * style,
                                  const HeaderStrategy * strategy );
  void writeConfig( bool withSync=true );

  KUrl urlClicked() const;

  void readConfig();

  /** A QAIM tree model of the message structure. */
  QAbstractItemModel* messageTreeModel() const;

  /**
   * Create an item fetch job that is suitable for using to fetch the message item that will
   * be displayed on this viewer.
   * It will set the correct fetch scope.
   * You still need to connect to the job's result signal.
   */
  static Akonadi::ItemFetchJob* createFetchJob( const Akonadi::Item &item );

signals:

  /**
   * Emitted when a status bar message is shown. Note that the status bar message is also set to
   * KPIM::BroadcastStatus in addition.
   */
  void showStatusBarMessage( const QString &message );

  /**
   * Emitted after parsing of a message to have it stored
   * in unencrypted state in it's folder.
   */
  void replaceMsgByUnencryptedVersion();

  /** The user presses the right mouse button. 'url' may be 0. */
  void popupMenu(KMime::Message &msg, const KUrl &url, const QPoint& mousePos);
  /** The user presses the right mouse button. 'url' may be 0. */
  void popupMenu(const Akonadi::Item &msg, const KUrl &url, const QPoint& mousePos);

  /**
   * The message viewer handles some types of urls itself, most notably http(s)
   * and ftp(s). When it can't handle the url it will emit this signal.
   */
  void urlClicked( const Akonadi::Item &, const KUrl& );

  /** The user has clicked onto an URL that is no attachment. */
  void urlClicked(const KUrl &url, int button);

  void requestConfigSync();
  void showReader( KMime::Content* aMsgPart, bool aHTML, const QString& aFileName,
                   const QString& pname, const QString & encoding );
  void showMessage( KMime::Message::Ptr message, const QString& encoding );

  void deleteMessage( const Akonadi::Item & );

public slots:

  /**
   * HTML Widget scrollbar and layout handling.
   *
   * Scrolling always happens in the direction of the slot that is called. I.e.
   * the methods take the absolute value of
   */
  void slotScrollUp( int pixels = 10 );
  void slotScrollDown( int pixels = 10 );
  void slotScrollPrior();
  void slotScrollNext();
  void slotJumpDown();
  void slotFind();
  void slotUrlClicked();
  void slotSaveMessage();
  void slotAttachmentSaveAs();
  void slotShowMessageSource();

protected:
  /** Some necessary event handling. */
  virtual void closeEvent(QCloseEvent *);
  virtual void resizeEvent(QResizeEvent *);
  /** Watch for palette changes */
  virtual bool event(QEvent *e);

#ifdef KDEPIM_MOBILE_UI
  friend class MessageViewItem;
#endif
  ViewerPrivate* const d_ptr;
};

}

#endif

