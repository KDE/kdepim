/* -*- Mode: C++ -*-

  $Id$

  KDGear - useful widgets for Qt

  Copyright (C) 2001 by Klarälvdalens Datakonsult AB
*/

#ifndef __KDLISTBOXPAIR_H__
#define __KDLISTBOXPAIR_H__

#include <qpixmap.h>
#include <qwidget.h>

class QListBox;
class QListBoxItem;

class KDListBoxPair : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString leftLabel READ leftLabel WRITE setLeftLabel )
    Q_PROPERTY( QString rightLabel READ rightLabel WRITE setRightLabel )
    Q_PROPERTY( bool visibleLeftReorder READ isLeftReorderButtonsVisible WRITE setLeftReorderButtonsVisible )
    Q_PROPERTY( bool visibleRightReorder READ isRightReorderButtonsVisible WRITE setRightReorderButtonsVisible )
    Q_PROPERTY( bool enableLeftReorder READ isLeftReorderButtonsEnabled WRITE setLeftReorderButtonsEnabled )
    Q_PROPERTY( bool enableRightReorder READ isRightReorderButtonsEnabled WRITE setRightReorderButtonsEnabled )
    Q_PROPERTY( bool moveButtonsEnabled READ moveButtonsEnabled WRITE setMoveButtonsEnabled )
    Q_PROPERTY( QPixmap leftUpPixmap READ leftUpPixmap WRITE setLeftUpPixmap )
    Q_PROPERTY( QPixmap rightUpPixmap READ rightUpPixmap WRITE setRightUpPixmap )
    Q_PROPERTY( QPixmap leftDownPixmap READ leftDownPixmap WRITE setLeftDownPixmap )
    Q_PROPERTY( QPixmap rightDownPixmap READ rightDownPixmap WRITE setRightDownPixmap )
    Q_PROPERTY( QString leftToRightLabel READ leftToRightLabel WRITE setLeftToRightLabel )
    Q_PROPERTY( QString rightToLeftLabel READ rightToLeftLabel WRITE setRightToLeftLabel )
    Q_PROPERTY( bool dragAndDropEnabled READ isDragAndDropEnabled WRITE setDragAndDropEnabled )

public:
    KDListBoxPair( bool leftReorder, bool rightReorder,
                   const QString& labelLeft, const QString& labelRight,
                   QWidget* parent = 0, const char* name = 0 );
    KDListBoxPair( const QString& labelLeft, const QString& labelRight,
                   QWidget* parent = 0, const char* name = 0 );
    KDListBoxPair( QWidget* parent = 0, const char* name = 0 );

    virtual ~KDListBoxPair();

    void setLeftLabel( const QString& label );
    QString leftLabel() const;
    void setRightLabel( const QString& label );
    QString rightLabel() const;

    QListBox* leftListBox() const;
    QListBox* rightListBox() const;

    void setLeftReorderButtonsVisible( bool visible );
    bool isLeftReorderButtonsVisible() const;
    
    void setRightReorderButtonsVisible( bool visible );
    bool isRightReorderButtonsVisible() const;
    
    void setLeftReorderButtonsEnabled( bool enable );
    bool isLeftReorderButtonsEnabled() const;

    void setRightReorderButtonsEnabled( bool enable );
    bool isRightReorderButtonsEnabled() const;

    void setLeftUpPixmap( const QPixmap& pixmap );
    QPixmap leftUpPixmap() const;

    void setLeftDownPixmap( const QPixmap& pixmap );
    QPixmap leftDownPixmap() const;

    void setRightUpPixmap( const QPixmap& pixmap );
    QPixmap rightUpPixmap() const;

    void setRightDownPixmap( const QPixmap& pixmap );
    QPixmap rightDownPixmap() const;

    void setMoveButtonsEnabled( bool enable );
    bool moveButtonsEnabled() const;

    void setLeftToRightLabel( const QString& label );
    QString leftToRightLabel() const;
    void setRightToLeftLabel( const QString& label );
    QString rightToLeftLabel() const;

    void setDragAndDropEnabled( bool enable );
    bool isDragAndDropEnabled() const;

signals:
    void itemMovedLeftToRight( QListBoxItem* oldItem, QListBoxItem* newItem );
    void itemMovedRightToLeft( QListBoxItem* oldItem, QListBoxItem* newItem );
    void itemReorderedLeft( QListBoxItem* item, int oldPos, int newPos );
    void itemReorderedRight( QListBoxItem* item, int oldPos, int newPos );
protected slots:
    void moveLeftUp();
    void moveLeftDown();
    void moveRightUp();
    void moveRightDown();
    void moveLeftToRight();
    void moveRightToLeft();
private:
    void init( bool leftReorder, bool rightReorder );
    QListBoxItem* moveItem( QListBox*, int i1, int i2 );

    struct KDListBoxPairPrivate;
    KDListBoxPairPrivate* d;

    friend class KDDnDListBox;
};

#endif
