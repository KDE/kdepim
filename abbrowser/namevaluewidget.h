/* This file is part of KDE PIM
    Copyright (C) 1999 Don Sanders <dsanders@kde.org>

    License: GNU GPL
*/

#ifndef NAMEVALUE_H 
#define NAMEVALUE_H 

#include <KabEntity.h>

#include <qframe.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <qsize.h>
#include <qwidget.h>
#include <qscrollview.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcombobox.h>

#include "entry.h"

/**
 * A table with two columns and a variable number of rows. The columns are 
 * field name and field value. The field value column is editable.
 *
 * An Entity object is updated as values are changed.
 */
class NameValueSheet : public QFrame
{
    Q_OBJECT

public:
/**
 * Constructs a name value sheet.
 *
 * Arguments:
 *
 * @param rows The number of rows.
 * @param name A list of entry field names.
 * @param entryField A list of entry field keys.
 * @param ce An Entity object that will be updated as values are changed.
 */
    NameValueSheet( QWidget *parent, int rows, QStringList name, QStringList entryField, KAB::Entity *ce );

/**
 * Destroys the name NameValueSheet object
 */
    virtual~NameValueSheet();

/**
 * Returns the size of a row in the NameValueSheet
 */
    QSize cellSize();
private:
    enum fudgeFactor { verticalTrim = 4 };
    QLabel *lCell;
    int rows;
    int minNameWidth;
    int minNameHeight;
    QLabel *temp;
};

/**
 * A possibly scrollable frame for placing a NameValueSheet in.
 */ 
class NameValueFrame : public QScrollView
{
    Q_OBJECT

public:
/**
 * Creates a NameValueFrame object that contains a NameValueSheet object.
 */
    NameValueFrame( QWidget *parent, NameValueSheet* vs );

/**
 * Updates the NameValueSheet contained.
 */
    virtual void setSheet( NameValueSheet* vs );
protected:
    virtual void resizeEvent(QResizeEvent*);
    NameValueSheet* vs;
    QLabel *lName;
    QLabel *lValue;
};

/**
 * A ContactLineEdit object is substitutable for a QLineEdit object.
 * It both automatically updates an associated Entity object and 
 * is itself  updated if changes are made to the Entity object.
 */
class ContactLineEdit : public QLineEdit
{
    Q_OBJECT

public:
/**
 * Create a ContactLineEdit object.
 *
 * Arguments:
 *
 * @param name Both the name of the widget and the name of the key used in the
 * Entity object.
 * @param ce The Entity object associated with this LineEdit.
 */
    ContactLineEdit( QWidget *parent, const char *name, KAB::Entity *ce );

/**
 * When the widget loses focus the associated Entity object is updated.
 */
    virtual void focusOutEvent ( QFocusEvent * );

/**
 * Changes the Entity key this widget is associated with, and updates
 * the text of this ContactLineEdit.
 */
    virtual void setName ( const char * name );

private:
    KAB::Entity *ce;

private slots:
    void sync();
};

/*
 * Same idea as ContactLineEdit but for a MultiLineEdit instead of a
 * a LineEdit
 */
class ContactMultiLineEdit : public QMultiLineEdit
{
    Q_OBJECT

public:
    ContactMultiLineEdit( QWidget *parent, const char *name, KAB::Entity *ce );
    virtual void focusOutEvent ( QFocusEvent * );
    virtual void setName ( const char * name );

private:
    KAB::Entity *ce;

private slots:
    void sync();
};

/*
 * Same idea as ContactLineEdit but for a  read/write ComboBox
 * instead of a LineEdit
 */
class FileAsComboBox : public QComboBox
{
    Q_OBJECT

public:
    FileAsComboBox( QWidget *parent, const char *name, KAB::Entity *ce );
    virtual void setName ( const char * name );

private:
    KAB::Entity *ce;

public slots:
    virtual void updateContact();

private slots:
    void sync();
};

/*
  A ContactComboBox object is a non-editable QComboBox like widget. Each 
  ContactComboBox has a buddy widget, normally a ContactLineEdit associated
  with it.

  Each item in ContactComboBox object has an Entity key associated
  with it. When a new item is selected the buddy widget is renamed to
  the value of the key associated with the selected item. This will result
  in the buddy widget being associated with the new key.

  A quick and dirty derivation. A ContactComboBox object isn't
  substitutable for a QComboBox object.
*/
class ContactComboBox : public QComboBox
{
    Q_OBJECT

public:
    ContactComboBox( QWidget *parent );
    virtual void setBuddy( QWidget *buddy );
    virtual void insertItem ( const QString & text, const QString & vText );
    QString currentEntryField();

public slots:
    void updateBuddy( int index );

private:
    QWidget *buddy;
    QStringList vlEntryField;
};
#endif
