/*
	imeditorwidget.h

	IM addresses editor widget for KAddressbook

	Copyright (c) 2004 Will Stephenson   <lists@stevello.free-online.co.uk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef IMEDITORWIDGET_H
#define IMEDITORWIDGET_H

#include <qvaluelist.h>
#include <klistview.h>
#include <kdialogbase.h>

#include "contacteditorwidget.h"


class AddressWidget;
class KPluginInfo;
class IMEditorBase;

enum IMContext { Any, Home, Work };

/* Note regarding Context:
 * It wasn not possible to get an idea of Context into Kopete in time for KDE 3.3,
 * so it has been removed from the UI and functionally disabled in the code.
 */

/**
 * The widget we add to KAddressbook's contact editor dialog
 */
class IMEditorWidget : public KDialogBase
{
  Q_OBJECT

  public:
    IMEditorWidget( QWidget *parent, const QString &preferredIM, const char *name = 0 );
    ~IMEditorWidget() {};

    void loadContact( KABC::Addressee *addr );
    void storeContact( KABC::Addressee *addr );
    void setReadOnly( bool readOnly );
    QValueList<KPluginInfo *> availableProtocols() const; 
    bool isModified() const;
    QString preferred();

  protected slots:
    void slotUpdateButtons();
    void slotAdd();
    void slotEdit();
    void slotDelete();
    void slotSetStandard();

  protected:
    /**
     * Helper method to split the contents of an addressbook field up
     */
    static void splitField( const QString &str, QString &app, QString &name, QString &value );

    /**
     * Find a protocol that matches the KABC key, or 0 if none found
     */
    KPluginInfo * protocolFromString( const QString fieldValue );

  private:
    bool mReadOnly;
    bool mModified;
    QString mPreferred;
    IMEditorBase *mWidget;
    void setModified( bool modified );

    // Used to track changed protocols to reduce KABC writes
    QValueList<KPluginInfo *> mChangedProtocols;
    QValueList<KPluginInfo *> mProtocols;
};

/**
 * List view item representing a single IM address.
 */

// VCard has been disabled as there is no standard VCard location to store IM addresses yet.
class IMAddressLVI : public KListViewItem
{
  public:
    IMAddressLVI( KListView *parent, KPluginInfo * protocol,
                  const QString &address, const IMContext &context = Any );

    void setAddress( const QString &address );
    void setProtocol( KPluginInfo * protocol );
    void setContext( const IMContext &context );
    void activate();
    KPluginInfo * protocol() const;
    QString address() const;
    IMContext context() const;

    void setPreferred( bool preferred );
    bool preferred() const;
  
  protected:
    virtual void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment);
	   
  private:
    KPluginInfo * mProtocol;
    bool mPreferred;
    IMContext mContext;
    QString mAddress;
};

#endif

