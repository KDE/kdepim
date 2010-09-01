/*
    This file is part of libkdepim.
    Copyright (c) 2002 Helge Deller <deller@gmx.de>
                  2002 Lubos Lunak <llunak@suse.cz>
                  2001,2003 Carsten Pfeiffer <pfeiffer@kde.org>
                  2001 Waldo Bastian <bastian@kde.org>
                  2004 Daniel Molkentin <danimo@klaralvdalens-datakonsult.se>
                  2004 Karl-Heinz Zimmer <khz@klaralvdalens-datakonsult.se>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef ADDRESSEELINEEDIT_H
#define ADDRESSEELINEEDIT_H

#include <tqobject.h>
#include <tqptrlist.h>
#include <tqtimer.h>
#include <tqpair.h>
#include <tqvaluelist.h>

#include <kabc/addressee.h>

#include "clicklineedit.h"
#include "kmailcompletion.h"
#include <dcopobject.h>
#include <kdepimmacros.h>

class KConfig;

namespace KPIM {
class LdapSearch;
class LdapResult;
typedef TQValueList<LdapResult> LdapResultList;
typedef TQMap< TQString, QPair<int,int> > CompletionItemsMap;
}

namespace KPIM {

class KDE_EXPORT AddresseeLineEdit : public ClickLineEdit, public DCOPObject
{
  K_DCOP
  Q_OBJECT

  public:
    AddresseeLineEdit( TQWidget* parent, bool useCompletion = true,
                     const char *name = 0L);
    virtual ~AddresseeLineEdit();

    virtual void setFont( const TQFont& );
    void allowSemiColonAsSeparator( bool );

    /// Sets if distribution lists will be used for completion.
    /// This is true by default.
    /// Call this right after the constructor, before anything calls loadContacts(),
    /// otherwise this has no effect.
    void allowDistributionLists( bool allowDistLists );

  public slots:
    void cursorAtEnd();
    void enableCompletion( bool enable );
    /** Reimplemented for stripping whitespace after completion */
    virtual void setText( const TQString& txt );

  protected slots:
    virtual void loadContacts();
  protected:
    void addContact( const KABC::Addressee&, int weight, int source = -1 );
    virtual void keyPressEvent( TQKeyEvent* );
    /**
     * Reimplemented for smart insertion of email addresses.
     * Features:
     * - Automatically adds ',' if necessary to separate email addresses
     * - Correctly decodes mailto URLs
     * - Recognizes email addresses which are protected against address
     *   harvesters, i.e. "name at kde dot org" and "name(at)kde.org"
     */
    virtual void insert( const TQString &text );
    /** Reimplemented for smart insertion of pasted email addresses. */
    virtual void paste();
    /** Reimplemented for smart insertion with middle mouse button. */
    virtual void mouseReleaseEvent( TQMouseEvent *e );
    /** Reimplemented for smart insertion of dragged email addresses. */
    virtual void dropEvent( TQDropEvent *e );
    void doCompletion( bool ctrlT );
    virtual TQPopupMenu *createPopupMenu();

    /**
     * Adds the name of a completion source to the internal list of
     * such sources and returns its index, such that that can be used
     * for insertion of items associated with that source.
     * 
     * If the source already exists, the weight will be updated.
     */
    int addCompletionSource( const TQString&, int weight );

    /** return whether we are using sorted or weighted display */
    static KCompletion::CompOrder completionOrder();

  k_dcop:
    // Connected to the DCOP signal
    void slotIMAPCompletionOrderChanged();

  private slots:
    void slotCompletion();
    void slotPopupCompletion( const TQString& );
    void slotReturnPressed( const TQString& );
    void slotStartLDAPLookup();
    void slotLDAPSearchData( const KPIM::LdapResultList& );
    void slotEditCompletionOrder();
    void slotUserCancelled( const TQString& );

  private:
    virtual bool eventFilter(TQObject *o, TQEvent *e);
    void init();
    void startLoadingLDAPEntries();
    void stopLDAPLookup();
    void updateLDAPWeights();

    void setCompletedItems( const TQStringList& items, bool autoSuggest );
    void addCompletionItem( const TQString& string, int weight, int source, const TQStringList * keyWords=0 );
    TQString completionSearchText( TQString& );
    const TQStringList getAdjustedCompletionItems( bool fullSearch );
    void updateSearchString();

    TQString m_previousAddresses;
    TQString m_searchString;
    bool m_useCompletion;
    bool m_completionInitialized;
    bool m_smartPaste;
    bool m_addressBookConnected;
    bool m_lastSearchMode;
    bool m_searchExtended; //has \" been added?
    bool m_useSemiColonAsSeparator;
    bool m_allowDistLists;

    //TQMap<TQString, KABC::Addressee> m_contactMap;

    static bool s_addressesDirty;
    static KMailCompletion *s_completion;
    static CompletionItemsMap* s_completionItemMap;
    static TQTimer *s_LDAPTimer;
    static KPIM::LdapSearch *s_LDAPSearch;
    static TQString *s_LDAPText;
    static AddresseeLineEdit *s_LDAPLineEdit;
    static TQStringList *s_completionSources;
    static TQMap<int,int> *s_ldapClientToCompletionSourceMap;

    class AddresseeLineEditPrivate;
    AddresseeLineEditPrivate *d;

    //until MenuID moves into protected in KLineEdit, we keep a copy here
    //Constants that represent the ID's of the popup menu.
      enum MenuID
      {
        Default = 42,
        NoCompletion,
        AutoCompletion,
        ShellCompletion,
        PopupCompletion,
        ShortAutoCompletion,
        PopupAutoCompletion
      };

};

}

#endif
