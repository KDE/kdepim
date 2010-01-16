/*
  Copyright 2010 Olivier Trichet <nive@nivalis.org>

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#ifndef KNODE_COMPOSER_ATTACHMENT_VIEW_H
#define KNODE_COMPOSER_ATTACHMENT_VIEW_H

#include "knarticle.h"

#include <QTreeWidget>


namespace KNode {
namespace Composer {

/**
  Attachment view in the composer.
*/
class AttachmentView : public QTreeWidget
{
  Q_OBJECT

  public:
    /**
      List of column. The value is the column index.
    */
    enum Column {
      File = 0,        /**< file name. */
      Type = 1,        /**< mime-type. */
      Size = 2,        /**< size. */
      Description = 3, /**< description of the attachment. */
      Encoding = 4     /**< MIME encoding (7bit, base64, etc.). */
    };


    /**
      Constructor for the selection widget of identity.
      @param view The view this Attachement
    */
    explicit AttachmentView( QWidget *parent = 0 );
    /**
      Destructor.
    */
    virtual ~AttachmentView();


    /**
      Returns the list of attachments contained by this view .
    */
    const QList<KNAttachment::Ptr> attachments();


  public slots:
    /**
      Remove the currently selected attachment if there is a selection.
    */
    void removeCurrentAttachment();
    /**
      Edit the currently selected attachment.
    */
    void editCurrentAttachment();


  signals:
    /**
      This signal is emitted when an attachment was actually removed.
      @param attachment the removed attachment.
      @param last true when @p attachment was the last attachment of this message.
    */
    void attachmentRemoved( KNAttachment::Ptr attachment, bool last );

    /**
      This signal is emitted when the Delete key is pressed on this widget.
    */
    void deletePressed();
    /**
      This signal is emitted when the Return or Enter key is pressed on this widget.
    */
    void returnPressed();

    /**
      Request a context menu on an attachment item at @p point.
      @param point a global position.
    */
    void contextMenuRequested( const QPoint &point );

    protected:
    /**
      Reimplemented to emit the deletePressed() and returnPressed() signals.
    */
    virtual void keyPressEvent( QKeyEvent *event );
    /**
      Reimplemented to emit the contextMenuRequested() signal.
    */
    virtual void contextMenuEvent( QContextMenuEvent *event );
};


/**
  Item of the AttachmentView. A wrapper around a KNAttachment.
*/
class AttachmentViewItem : public QTreeWidgetItem
{
  friend class AttachmentView;

  public:
    /**
      Constructor.
    */
    AttachmentViewItem( AttachmentView *parent, KNAttachment::Ptr attachment );
    /**
      Destructor.
    */
    virtual ~AttachmentViewItem();

    /**
      Reimplemented to return data from the underlying KNAttachment.
    */
    QVariant data( int column, int role ) const;

  private:
    KNAttachment::Ptr mAttachment;
};


} // namespace Composer
} // namespace KNode


#endif // KNODE_COMPOSER_ATTACHMENT_VIEW_H
