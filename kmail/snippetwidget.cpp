/***************************************************************************
 *   snippet feature from kdevelop/plugins/snippet/                        *
 *                                                                         *
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <tqlayout.h>
#include <kpushbutton.h>
#include <klistview.h>
#include <tqheader.h>
#include <klineedit.h>
#include <ktextedit.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <tqtooltip.h>
#include <kpopupmenu.h>
#include <tqregexp.h>
#include <tqinputdialog.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqwhatsthis.h>
#include <tqdragobject.h>
#include <tqtimer.h>
#include <kcombobox.h>
#include <kmedit.h>
#include <kiconloader.h>
#include <kshortcut.h>
#include <kaction.h>
#include <kkeybutton.h>

#include "snippetdlg.h"
#include "snippetitem.h"
#include "snippetwidget.h"

#include <cassert>

SnippetWidget::SnippetWidget(KMEdit* editor, KActionCollection* actionCollection, TQWidget* parent)
		: KListView(parent, "snippet widget"), TQToolTip( viewport() ),
		mEditor( editor ), mActionCollection( actionCollection )
{
    // init the QPtrList
    _list.setAutoDelete(TRUE);

    // init the KListView
    setSorting( -1 );
    addColumn( "" );
    setFullWidth(true);
    header()->hide();
    setAcceptDrops(true);
    setDragEnabled(true);
    setDropVisualizer(false);
    setRootIsDecorated(true);

    //connect the signals
    connect( this, TQT_SIGNAL( contextMenuRequested ( TQListViewItem *, const TQPoint & , int ) ),
             this, TQT_SLOT( showPopupMenu(TQListViewItem *, const TQPoint & , int ) ) );

    connect( this, TQT_SIGNAL( doubleClicked (TQListViewItem *) ),
             this, TQT_SLOT( slotEdit( TQListViewItem *) ) );
    connect( this, TQT_SIGNAL( returnPressed (TQListViewItem *) ),
             this, TQT_SLOT( slotExecuted( TQListViewItem *) ) );

    connect( this, TQT_SIGNAL( dropped(TQDropEvent *, TQListViewItem *) ),
             this, TQT_SLOT( slotDropped(TQDropEvent *, TQListViewItem *) ) );

    connect( editor, TQT_SIGNAL( insertSnippet() ), this, TQT_SLOT( slotExecute() ));

    _cfg = 0;

    TQTimer::singleShot(0, this, TQT_SLOT(initConfig()));
}

SnippetWidget::~SnippetWidget()
{
  writeConfig();
  delete _cfg;

  /* We need to delete the child items before the parent items
     otherwise KDevelop would crash on exiting */
  SnippetItem * item;
  while (_list.count() > 0) {
    for (item=_list.first(); item; item=_list.next()) {
      if (item->childCount() == 0)
          _list.remove(item);
    }
  }
}


/*!
    \fn SnippetWidget::slotAdd()
    Opens the dialog to add a snippet
 */
void SnippetWidget::slotAdd()
{
  //kdDebug(5006) << "Ender slotAdd()" << endl;
  SnippetDlg dlg( mActionCollection, this, "SnippetDlg");

  /*check if the user clicked a SnippetGroup
    If not, we set the group variable to the SnippetGroup
    which the selected item is a child of*/
  SnippetGroup * group = dynamic_cast<SnippetGroup*>(selectedItem());
  if ( !group && selectedItem() )
    group = dynamic_cast<SnippetGroup*>(selectedItem()->parent());

  /* still no group, let's make a default one */
  if (!group ) {
    if ( _list.isEmpty() ) {
      group = new SnippetGroup(this, i18n("General"), SnippetGroup::getMaxId() );
      _list.append( group );
    } else {
      group = dynamic_cast<SnippetGroup*>( _list.first() );
    }
  }
  assert( group );

  /*fill the combobox with the names of all SnippetGroup entries*/
  for (SnippetItem *it=_list.first(); it; it=_list.next()) {
    if (dynamic_cast<SnippetGroup*>(it)) {
      dlg.cbGroup->insertItem(it->getName());
    }
  }
  dlg.cbGroup->setCurrentText(group->getName());

  if (dlg.exec() == TQDialog::Accepted) {
      group = dynamic_cast<SnippetGroup*>(SnippetItem::findItemByName(dlg.cbGroup->currentText(), _list));
      _list.append( makeItem( group, dlg.snippetName->text(), dlg.snippetText->text(), dlg.keyButton->shortcut() ) );
  }
}

/*!
    \fn SnippetWidget::makeItem( SnippetItem* parent, const TQString& name, const TQString& text )
    Helper factory method.
 */
SnippetItem* SnippetWidget::makeItem( SnippetItem* parent, const TQString& name, const TQString& text, const KShortcut& shortcut )
{
    SnippetItem * item = new SnippetItem(parent, name, text);
    const TQString actionName = i18n("Snippet %1").arg(name);
    const TQString normalizedName = TQString(actionName).replace(" ", "_");
    if ( !mActionCollection->action(normalizedName.utf8() ) ) {
        KAction * action = new KAction( actionName, shortcut, item,
                                        TQT_SLOT( slotExecute() ), mActionCollection,
                                        normalizedName.utf8() );
        item->setAction(action);
        connect( item, TQT_SIGNAL( execute( TQListViewItem* ) ),
                 this, TQT_SLOT( slotExecuted( TQListViewItem* ) ) );
    }
    return item;
}

/*!
    \fn SnippetWidget::slotAddGroup()
    Opens the didalog to add a snippet
 */
void SnippetWidget::slotAddGroup()
{
  //kdDebug(5006) << "Ender slotAddGroup()" << endl;
  SnippetDlg dlg( mActionCollection, this, "SnippetDlg");
  dlg.setShowShortcut( false );
  dlg.snippetText->setEnabled(false);
  dlg.snippetText->setText(i18n("GROUP"));
  dlg.setCaption(i18n("Add Group"));
  dlg.cbGroup->insertItem(i18n("All"));
  dlg.cbGroup->setCurrentText(i18n("All"));

  if (dlg.exec() == TQDialog::Accepted) {
    _list.append( new SnippetGroup(this, dlg.snippetName->text(), SnippetGroup::getMaxId() ) );
  }
}


/*!
    \fn SnippetWidget::slotRemove()
    Removes the selected snippet
 */
void SnippetWidget::slotRemove()
{
  //get current data
  TQListViewItem * item = currentItem();
  SnippetItem *snip = dynamic_cast<SnippetItem*>( item );
  SnippetGroup *group = dynamic_cast<SnippetGroup*>( item );
  if (!snip)
    return;

  if (group) {
    if (group->childCount() > 0 &&
        KMessageBox::warningContinueCancel(this, i18n("Do you really want to remove this group and all its snippets?"),TQString::null,KStdGuiItem::del())
        == KMessageBox::Cancel)
      return;

    SnippetItem *it=_list.first();
    while ( it ) {
      if (it->getParent() == group->getId()) {
        SnippetItem *doomed = it;
        it = _list.next();
        //kdDebug(5006) << "remove " << doomed->getName() << endl;
        _list.remove(doomed);
      } else {
        it = _list.next();
      }
    }
  }

  //kdDebug(5006) << "remove " << snip->getName() << endl;
  _list.remove(snip);
}



/*!
    \fn SnippetWidget::slotEdit()
    Opens the dialog of editing the selected snippet
 */
void SnippetWidget::slotEdit( TQListViewItem* item )
{
    if( item == 0 ) {
	item = currentItem();
    }

  SnippetGroup *pGroup = dynamic_cast<SnippetGroup*>(item);
  SnippetItem *pSnippet = dynamic_cast<SnippetItem*>( item );
  if (!pSnippet || pGroup) /*selected item must be a SnippetItem but MUST not be a SnippetGroup*/
    return;

  //init the dialog
  SnippetDlg dlg( mActionCollection, this, "SnippetDlg");
  dlg.snippetName->setText(pSnippet->getName());
  dlg.snippetText->setText(pSnippet->getText());
  dlg.keyButton->setShortcut( pSnippet->getAction()->shortcut(), false );
  dlg.btnAdd->setText(i18n("&Apply"));

  dlg.setCaption(i18n("Edit Snippet"));
  /*fill the combobox with the names of all SnippetGroup entries*/
  for (SnippetItem *it=_list.first(); it; it=_list.next()) {
    if (dynamic_cast<SnippetGroup*>(it)) {
      dlg.cbGroup->insertItem(it->getName());
    }
  }
  dlg.cbGroup->setCurrentText(SnippetItem::findGroupById(pSnippet->getParent(), _list)->getName());

  if (dlg.exec() == TQDialog::Accepted) {
    //update the KListView and the SnippetItem
    item->setText( 0, dlg.snippetName->text() );
    pSnippet->setName( dlg.snippetName->text() );
    pSnippet->setText( dlg.snippetText->text() );
    pSnippet->getAction()->setShortcut( dlg.keyButton->shortcut());

    /* if the user changed the parent we need to move the snippet */
    if ( SnippetItem::findGroupById(pSnippet->getParent(), _list)->getName() != dlg.cbGroup->currentText() ) {
      SnippetGroup * newGroup = dynamic_cast<SnippetGroup*>(SnippetItem::findItemByName(dlg.cbGroup->currentText(), _list));
      pSnippet->parent()->takeItem(pSnippet);
      newGroup->insertItem(pSnippet);
      pSnippet->resetParent();
    }

    setSelected(item, TRUE);
  }
}

/*!
    \fn SnippetWidget::slotEditGroup()
    Opens the dialog of editing the selected snippet-group
 */
void SnippetWidget::slotEditGroup()
{
  //get current data
  TQListViewItem * item = currentItem();

  SnippetGroup *pGroup = dynamic_cast<SnippetGroup*>( item );
  if (!pGroup) /*selected item MUST be a SnippetGroup*/
    return;

  //init the dialog
  SnippetDlg dlg( mActionCollection, this, "SnippetDlg" );
  dlg.setShowShortcut( false );
  dlg.snippetName->setText(pGroup->getName());
  dlg.snippetText->setText(pGroup->getText());
  dlg.btnAdd->setText(i18n("&Apply"));
  dlg.snippetText->setEnabled(FALSE);
  dlg.setCaption(i18n("Edit Group"));
  dlg.cbGroup->insertItem(i18n("All"));

  if (dlg.exec() == TQDialog::Accepted) {
    //update the KListView and the SnippetGroup
    item->setText( 0, dlg.snippetName->text() );
    pGroup->setName( dlg.snippetName->text() );

    setSelected(item, TRUE);
  }
}

void SnippetWidget::slotExecuted(TQListViewItem * item)
{
    if( item == 0 )
    {
	item = currentItem();
    }

  SnippetItem *pSnippet = dynamic_cast<SnippetItem*>( item );
  if (!pSnippet || dynamic_cast<SnippetGroup*>(item))
      return;

  //process variables if any, then insert into the active view
  insertIntoActiveView( parseText(pSnippet->getText(), _SnippetConfig.getDelimiter()) );
}


/*!
    \fn SnippetWidget::insertIntoActiveView(TQString text)
    Inserts the parameter text into the activ view
 */
void SnippetWidget::insertIntoActiveView( const TQString &text )
{
    mEditor->insert( text );
}


/*!
    \fn SnippetWidget::writeConfig()
    Write the cofig file
 */
void SnippetWidget::writeConfig()
{
  if( !_cfg )
    return;
  _cfg->deleteGroup("SnippetPart");  //this is neccessary otherwise delete entries will stay in list until
                                     //they get overwritten by a more recent entry
  _cfg->setGroup("SnippetPart");

  TQString strKeyName="";
  TQString strKeyText="";
  TQString strKeyId="";

  int iSnipCount = 0;
  int iGroupCount = 0;

  SnippetItem* item=_list.first();
  while ( item != 0) {

    //kdDebug(5006) << "-->ERROR " << item->getName() << endl;
    //kdDebug(5006) << "SnippetWidget::writeConfig() " << item->getName() << endl;
    SnippetGroup * group = dynamic_cast<SnippetGroup*>(item);
    if (group) {
      //kdDebug(5006) << "-->GROUP " << item->getName() << group->getId() << " " << iGroupCount<< endl;
      strKeyName=TQString("snippetGroupName_%1").arg(iGroupCount);
      strKeyId=TQString("snippetGroupId_%1").arg(iGroupCount);

      _cfg->writeEntry(strKeyName, group->getName());
      _cfg->writeEntry(strKeyId, group->getId());

      iGroupCount++;
    } else if (dynamic_cast<SnippetItem*>(item)) {
      //kdDebug(5006) << "-->ITEM " << item->getName() << item->getParent() << " " << iSnipCount << endl;
      strKeyName=TQString("snippetName_%1").arg(iSnipCount);
      strKeyText=TQString("snippetText_%1").arg(iSnipCount);
      strKeyId=TQString("snippetParent_%1").arg(iSnipCount);

      _cfg->writeEntry(strKeyName, item->getName());
      _cfg->writeEntry(strKeyText, item->getText());
      _cfg->writeEntry(strKeyId, item->getParent());

      KAction * action = item->getAction();
      assert( action );
      const KShortcut& sc = action->shortcut();
      if (!sc.isNull() ) {
          _cfg->writeEntry( TQString("snippetShortcut_%1").arg(iSnipCount), sc.toString() );
      }
      iSnipCount++;
    } else {
      //kdDebug(5006) << "-->ERROR " << item->getName() << endl;
    }
    item = _list.next();
  }
  _cfg->writeEntry("snippetCount", iSnipCount);
  _cfg->writeEntry("snippetGroupCount", iGroupCount);

  int iCount = 1;
  TQMap<TQString, TQString>::Iterator it;
  for ( it = _mapSaved.begin(); it != _mapSaved.end(); ++it ) {  //write the saved variable values
    if (it.data().length()<=0) continue;  //is the saved value has no length -> no need to save it

    strKeyName=TQString("snippetSavedName_%1").arg(iCount);
    strKeyText=TQString("snippetSavedVal_%1").arg(iCount);

    _cfg->writeEntry(strKeyName, it.key());
    _cfg->writeEntry(strKeyText, it.data());

    iCount++;
  }
  _cfg->writeEntry("snippetSavedCount", iCount-1);


  _cfg->writeEntry( "snippetDelimiter", _SnippetConfig.getDelimiter() );
  _cfg->writeEntry( "snippetVarInput", _SnippetConfig.getInputMethod() );
  _cfg->writeEntry( "snippetToolTips", _SnippetConfig.useToolTips() );
  _cfg->writeEntry( "snippetGroupAutoOpen", _SnippetConfig.getAutoOpenGroups() );

  _cfg->writeEntry("snippetSingleRect", _SnippetConfig.getSingleRect() );
  _cfg->writeEntry("snippetMultiRect", _SnippetConfig.getMultiRect() );

  _cfg->sync();
}

/*!
    \fn SnippetWidget::initConfig()
    Initial read the cofig file
 */
void SnippetWidget::initConfig()
{
  if (_cfg == NULL)
    _cfg = new KConfig("kmailsnippetrc", false, false);

  _cfg->setGroup("SnippetPart");

  TQString strKeyName="";
  TQString strKeyText="";
  TQString strKeyId="";
  SnippetItem *item;
  SnippetGroup *group;

  //kdDebug(5006) << "SnippetWidget::initConfig() " << endl;

  //if entry doesn't get found, this will return -1 which we will need a bit later
  int iCount = _cfg->readNumEntry("snippetGroupCount", -1);

  for ( int i=0; i<iCount; i++) {  //read the group-list
    strKeyName=TQString("snippetGroupName_%1").arg(i);
    strKeyId=TQString("snippetGroupId_%1").arg(i);

    TQString strNameVal="";
    int iIdVal=-1;

    strNameVal = _cfg->readEntry(strKeyName, "");
    iIdVal = _cfg->readNumEntry(strKeyId, -1);
    //kdDebug(5006) << "Read group "  << " " << iIdVal << endl;

    if (strNameVal != "" && iIdVal != -1) {
      group = new SnippetGroup(this, strNameVal, iIdVal);
      //kdDebug(5006) << "Created group " << group->getName() << " " << group->getId() << endl;
      _list.append(group);
    }
  }

  /* Check if the snippetGroupCount property has been found
     if iCount is -1 this means, that the user has his snippets
     stored without groups. Therefore we will call the
     initConfigOldVersion() function below */
  // should not happen since this function has been removed

  if (iCount != -1) {
    iCount = _cfg->readNumEntry("snippetCount", 0);
    for ( int i=0; i<iCount; i++) {  //read the snippet-list
        strKeyName=TQString("snippetName_%1").arg(i);
        strKeyText=TQString("snippetText_%1").arg(i);
        strKeyId=TQString("snippetParent_%1").arg(i);

        TQString strNameVal="";
        TQString strTextVal="";
        int iParentVal = -1;

        strNameVal = _cfg->readEntry(strKeyName, "");
        strTextVal = _cfg->readEntry(strKeyText, "");
        iParentVal = _cfg->readNumEntry(strKeyId, -1);
        //kdDebug(5006) << "Read item " << strNameVal << " " << iParentVal << endl;

        if (strNameVal != "" && strTextVal != "" && iParentVal != -1) {
            KShortcut shortcut( _cfg->readEntry( TQString("snippetShortcut_%1").arg(i), TQString() ) );
            item = makeItem( SnippetItem::findGroupById(iParentVal, _list), strNameVal, strTextVal, shortcut );
            //kdDebug(5006) << "Created item " << item->getName() << " " << item->getParent() << endl;
            _list.append(item);
        }
    }
  } else {
      //kdDebug() << "iCount is not -1";
  }

  iCount = _cfg->readNumEntry("snippetSavedCount", 0);

  for ( int i=1; i<=iCount; i++) {  //read the saved-values and store in QMap
    strKeyName=TQString("snippetSavedName_%1").arg(i);
    strKeyText=TQString("snippetSavedVal_%1").arg(i);

    TQString strNameVal="";
    TQString strTextVal="";

    strNameVal = _cfg->readEntry(strKeyName, "");
    strTextVal = _cfg->readEntry(strKeyText, "");

    if (strNameVal != "" && strTextVal != "") {
      _mapSaved[strNameVal] = strTextVal;
    }
  }


  _SnippetConfig.setDelimiter( _cfg->readEntry("snippetDelimiter", "$") );
  _SnippetConfig.setInputMethod( _cfg->readNumEntry("snippetVarInput", 0) );
  _SnippetConfig.setToolTips( _cfg->readBoolEntry("snippetToolTips", true) );
  _SnippetConfig.setAutoOpenGroups( _cfg->readNumEntry("snippetGroupAutoOpen", 1) );

  _SnippetConfig.setSingleRect( _cfg->readRectEntry("snippetSingleRect", 0L) );
  _SnippetConfig.setMultiRect( _cfg->readRectEntry("snippetMultiRect", 0L) );
}

/*!
    \fn SnippetWidget::maybeTip( const TQPoint & p )
    Shows the Snippet-Text as ToolTip
 */
void SnippetWidget::maybeTip( const TQPoint & p )
{
	SnippetItem * item = dynamic_cast<SnippetItem*>( itemAt( p ) );
	if (!item)
	  return;

	TQRect r = itemRect( item );

	if (r.isValid() &&
        _SnippetConfig.useToolTips() )
	{
	    tip( r, item->getText() );  //show the snippet's text
	}
}

/*!
    \fn SnippetWidget::showPopupMenu( TQListViewItem * item, const TQPoint & p, int )
    Shows the Popup-Menu depending item is a valid pointer
*/
void SnippetWidget::showPopupMenu( TQListViewItem * item, const TQPoint & p, int )
{
    KPopupMenu popup;

    SnippetItem * selectedItem = static_cast<SnippetItem *>(item);
    if ( item ) {
        popup.insertTitle( selectedItem->getName() );
        if (dynamic_cast<SnippetGroup*>(item)) {
            popup.insertItem( i18n("Edit &group..."), this, TQT_SLOT( slotEditGroup() ) );
        } else {
            popup.insertItem( SmallIconSet("editpaste"), i18n("&Paste"), this, TQT_SLOT( slotExecuted() ) );
            popup.insertItem( SmallIconSet("edit"), i18n("&Edit..."), this, TQT_SLOT( slotEdit() ) );
        }
        popup.insertItem( SmallIconSet("editdelete"), i18n("&Remove"), this, TQT_SLOT( slotRemove() ) );
        popup.insertSeparator();
    } else {
        popup.insertTitle(i18n("Text Snippets"));
    }
    popup.insertItem( i18n("&Add Snippet..."), this, TQT_SLOT( slotAdd() ) );
    popup.insertItem( i18n("Add G&roup..."), this, TQT_SLOT( slotAddGroup() ) );

    popup.exec(p);
}


//  fn SnippetWidget::parseText(TQString text, TQString del)
/*!
    This function is used to parse the given TQString for variables. If found the user will be prompted
    for a replacement value. It returns the string text with all replacements made
 */
TQString SnippetWidget::parseText(TQString text, TQString del)
{
  TQString str = text;
  TQString strName = "";
  TQString strNew = "";
  TQString strMsg="";
  int iFound = -1;
  int iEnd = -1;
  TQMap<TQString, TQString> mapVar;
  int iInMeth = _SnippetConfig.getInputMethod();
  TQRect rSingle = _SnippetConfig.getSingleRect();
  TQRect rMulti = _SnippetConfig.getMultiRect();

  do {
    iFound = text.find(TQRegExp("\\"+del+"[A-Za-z-_0-9\\s]*\\"+del), iEnd+1);  //find the next variable by this QRegExp
    if (iFound >= 0) {
      iEnd = text.find(del, iFound+1)+1;
      strName = text.mid(iFound, iEnd-iFound);

      if ( strName != del+del  ) {  //if not doubel-delimiter
        if (iInMeth == 0) { //if input-method "single" is selected
          if ( mapVar[strName].length() <= 0 ) {  // and not already in map
            strMsg=i18n("Please enter the value for <b>%1</b>:").arg(strName);
            strNew = showSingleVarDialog( strName, &_mapSaved, rSingle );
            if (strNew=="")
              return ""; //user clicked Cancle
          } else {
            continue; //we have already handled this variable
          }
        } else {
          strNew = ""; //for inputmode "multi" just reset new valaue
        }
      } else {
        strNew = del; //if double-delimiter -> replace by single character
      }

      if (iInMeth == 0) {  //if input-method "single" is selected
        str.replace(strName, strNew);
      }

      mapVar[strName] = strNew;
    }
  } while (iFound != -1);

  if (iInMeth == 1) {  //check config, if input-method "multi" is selected
    int w, bh, oh;
    w = rMulti.width();
    bh = rMulti.height();
    oh = rMulti.top();
    if (showMultiVarDialog( &mapVar, &_mapSaved, w, bh, oh )) {  //generate and show the dialog
      TQMap<TQString, TQString>::Iterator it;
      for ( it = mapVar.begin(); it != mapVar.end(); ++it ) {  //walk through the map and do the replacement
        str.replace(it.key(), it.data());
      }
    } else {
      return "";
    }

    rMulti.setWidth(w);   //this is a hack to save the dialog's dimensions in only one QRect
    rMulti.setHeight(bh);
    rMulti.setTop(oh);
    rMulti.setLeft(0);
     _SnippetConfig.setMultiRect(rMulti);
  }

  _SnippetConfig.setSingleRect(rSingle);

  return str;
}


//  fn SnippetWidget::showMultiVarDialog()
/*!
    This function constructs a dialog which contains a label and a linedit for every
    variable that is stored in the given map except the double-delimiter entry
    It return true if everything was ok and false if the user hit cancel
 */
bool SnippetWidget::showMultiVarDialog(TQMap<TQString, TQString> * map, TQMap<TQString, TQString> * mapSave,
                                       int & iWidth, int & iBasicHeight, int & iOneHeight)
{
  //if no var -> no need to show
  if (map->count() == 0)
    return true;

  //if only var is the double-delimiter -> no need to show
  TQMap<TQString, TQString>::Iterator it = map->begin();
  if ( map->count() == 1 && it.data()==_SnippetConfig.getDelimiter()+_SnippetConfig.getDelimiter() )
    return true;

  TQMap<TQString, KTextEdit *> mapVar2Te;  //this map will help keeping track which TEXTEDIT goes with which variable
  TQMap<TQString, TQCheckBox *> mapVar2Cb;  //this map will help keeping track which CHECKBOX goes with which variable

  // --BEGIN-- building a dynamic dialog
  TQDialog dlg(this);
  dlg.setCaption(i18n("Enter Values for Variables"));

  TQGridLayout * layout = new TQGridLayout( &dlg, 1, 1, 11, 6, "layout");
  TQGridLayout * layoutTop = new TQGridLayout( 0, 1, 1, 0, 6, "layoutTop");
  TQGridLayout * layoutVar = new TQGridLayout( 0, 1, 1, 0, 6, "layoutVar");
  TQGridLayout * layoutBtn = new TQGridLayout( 0, 1, 1, 0, 6, "layoutBtn");

  KTextEdit *te = NULL;
  TQLabel * labTop = NULL;
  TQCheckBox * cb = NULL;

  labTop = new TQLabel( &dlg, "label" );
  labTop->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0,
                         labTop->sizePolicy().hasHeightForWidth() ) );
  labTop->setText(i18n("Enter the replacement values for these variables:"));
  layoutTop->addWidget(labTop, 0, 0);
  layout->addMultiCellLayout( layoutTop, 0, 0, 0, 1 );


  int i = 0;                                           //walk through the variable map and add
  for ( it = map->begin(); it != map->end(); ++it ) {  //a checkbox, a lable and a lineedit to the main layout
    if (it.key() == _SnippetConfig.getDelimiter() + _SnippetConfig.getDelimiter())
      continue;

    cb = new TQCheckBox( &dlg, "cbVar" );
    cb->setChecked( FALSE );
    cb->setText(it.key());
    layoutVar->addWidget( cb, i ,0, Qt::AlignTop );

    te = new KTextEdit( &dlg, "teVar" );
    layoutVar->addWidget( te, i, 1, Qt::AlignTop );

    if ((*mapSave)[it.key()].length() > 0) {
      cb->setChecked( TRUE );
      te->setText((*mapSave)[it.key()]);
    }

    mapVar2Te[it.key()] = te;
    mapVar2Cb[it.key()] = cb;

    TQToolTip::add( cb, i18n("Enable this to save the value entered to the right as the default value for this variable") );
    TQWhatsThis::add( cb, i18n("If you enable this option, the value entered to the right will be saved. "
                              "If you use the same variable later, even in another snippet, the value entered to the right "
			      "will be the default value for that variable.") );

    i++;
  }
  layout->addMultiCellLayout( layoutVar, 1, 1, 0, 1 );

  KPushButton * btn1 = new KPushButton( KStdGuiItem::cancel(), &dlg, "pushButton1" );
  btn1->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0,
                         btn1->sizePolicy().hasHeightForWidth() ) );
  layoutBtn->addWidget( btn1, 0, 0 );

  KPushButton * btn2 = new KPushButton( KStdGuiItem::apply(), &dlg, "pushButton2" );
  btn2->setDefault( TRUE );
  btn2->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)1, (TQSizePolicy::SizeType)0, 0, 0,
                         btn2->sizePolicy().hasHeightForWidth() ) );
  layoutBtn->addWidget( btn2, 0, 1 );

  layout->addMultiCellLayout( layoutBtn, 2, 2, 0, 1 );
  // --END-- building a dynamic dialog

  //connect the buttons to the TQDialog default slots
  connect(btn1, TQT_SIGNAL(clicked()), &dlg, TQT_SLOT(reject()) );
  connect(btn2, TQT_SIGNAL(clicked()), &dlg, TQT_SLOT(accept()) );

  //prepare to execute the dialog
  bool bReturn = false;
  //resize the textedits
  if (iWidth > 1) {
    TQRect r = dlg.geometry();
    r.setHeight(iBasicHeight + iOneHeight*mapVar2Te.count());
    r.setWidth(iWidth);
    dlg.setGeometry(r);
  }
  if ( i > 0 && // only if there are any variables
    dlg.exec() == TQDialog::Accepted ) {

    TQMap<TQString, KTextEdit *>::Iterator it2;
    for ( it2 = mapVar2Te.begin(); it2 != mapVar2Te.end(); ++it2 ) {
      if (it2.key() == _SnippetConfig.getDelimiter() + _SnippetConfig.getDelimiter())
        continue;
      (*map)[it2.key()] = it2.data()->text();    //copy the entered values back to the given map

      if (mapVar2Cb[it2.key()]->isChecked())     //if the checkbox is on; save the values for later
        (*mapSave)[it2.key()] = it2.data()->text();
      else
        (*mapSave).erase(it2.key());
    }
    bReturn = true;

    iBasicHeight = dlg.geometry().height() - layoutVar->geometry().height();
    iOneHeight = layoutVar->geometry().height() / mapVar2Te.count();
    iWidth = dlg.geometry().width();
  }

  //do some cleanup
  TQMap<TQString, KTextEdit *>::Iterator it1;
  for (it1 = mapVar2Te.begin(); it1 != mapVar2Te.end(); ++it1)
    delete it1.data();
  mapVar2Te.clear();
  TQMap<TQString, TQCheckBox *>::Iterator it2;
  for (it2 = mapVar2Cb.begin(); it2 != mapVar2Cb.end(); ++it2)
    delete it2.data();
  mapVar2Cb.clear();
  delete layoutTop;
  delete layoutVar;
  delete layoutBtn;
  delete layout;

  if (i==0) //if nothing happened this means, that there are no variables to translate
    return true; //.. so just return OK

  return bReturn;
//  return true;
}


//  fn SnippetWidget::showSingleVarDialog(TQString var, TQMap<TQString, TQString> * mapSave)
/*!
    This function constructs a dialog which contains a label and a linedit for the given variable
    It return either the entered value or an empty string if the user hit cancel
 */
TQString SnippetWidget::showSingleVarDialog(TQString var, TQMap<TQString, TQString> * mapSave, TQRect & dlgSize)
{
  // --BEGIN-- building a dynamic dialog
  TQDialog dlg(this);
  dlg.setCaption(i18n("Enter Values for Variables"));

  TQGridLayout * layout = new TQGridLayout( &dlg, 1, 1, 11, 6, "layout");
  TQGridLayout * layoutTop = new TQGridLayout( 0, 1, 1, 0, 6, "layoutTop");
  TQGridLayout * layoutVar = new TQGridLayout( 0, 1, 1, 0, 6, "layoutVar");
  TQGridLayout * layoutBtn = new TQGridLayout( 0, 2, 1, 0, 6, "layoutBtn");

  KTextEdit *te = NULL;
  TQLabel * labTop = NULL;
  TQCheckBox * cb = NULL;

  labTop = new TQLabel( &dlg, "label" );
  layoutTop->addWidget(labTop, 0, 0);
  labTop->setText(i18n("Enter the replacement values for %1:").arg( var ));
  layout->addMultiCellLayout( layoutTop, 0, 0, 0, 1 );


  cb = new TQCheckBox( &dlg, "cbVar" );
  cb->setChecked( FALSE );
  cb->setText(i18n( "Make value &default" ));

  te = new KTextEdit( &dlg, "teVar" );
  layoutVar->addWidget( te, 0, 1, Qt::AlignTop);
  layoutVar->addWidget( cb, 1, 1, Qt::AlignTop);
  if ((*mapSave)[var].length() > 0) {
    cb->setChecked( TRUE );
    te->setText((*mapSave)[var]);
  }

  TQToolTip::add( cb, i18n("Enable this to save the value entered to the right as the default value for this variable") );
  TQWhatsThis::add( cb, i18n("If you enable this option, the value entered to the right will be saved. "
                            "If you use the same variable later, even in another snippet, the value entered to the right "
                            "will be the default value for that variable.") );

  layout->addMultiCellLayout( layoutVar, 1, 1, 0, 1 );

  KPushButton * btn1 = new KPushButton( KStdGuiItem::cancel(), &dlg, "pushButton1" );
  layoutBtn->addWidget( btn1, 0, 0 );

  KPushButton * btn2 = new KPushButton( KStdGuiItem::apply(), &dlg, "pushButton2" );
  btn2->setDefault( TRUE );
  layoutBtn->addWidget( btn2, 0, 1 );

  layout->addMultiCellLayout( layoutBtn, 2, 2, 0, 1 );
  te->setFocus();
  // --END-- building a dynamic dialog

  //connect the buttons to the TQDialog default slots
  connect(btn1, TQT_SIGNAL(clicked()), &dlg, TQT_SLOT(reject()) );
  connect(btn2, TQT_SIGNAL(clicked()), &dlg, TQT_SLOT(accept()) );

  //execute the dialog
  TQString strReturn = "";
  if (dlgSize.isValid())
    dlg.setGeometry(dlgSize);
  if ( dlg.exec() == TQDialog::Accepted ) {
    if (cb->isChecked())     //if the checkbox is on; save the values for later
      (*mapSave)[var] = te->text();
    else
      (*mapSave).erase(var);

    strReturn = te->text();    //copy the entered values back the the given map

    dlgSize = dlg.geometry();
  }

  //do some cleanup
  delete cb;
  delete te;
  delete labTop;
  delete btn1;
  delete btn2;
  delete layoutTop;
  delete layoutVar;
  delete layoutBtn;
  delete layout;

  return strReturn;
}

//  fn SnippetWidget::acceptDrag (TQDropEvent *event) const
/*!
    Reimplementation from KListView.
    Check here if the data the user is about to drop fits our restrictions.
    We only accept dropps of plaintext, because from the dropped text
    we will create a snippet.
 */
bool SnippetWidget::acceptDrag (TQDropEvent *event) const
{
  //kdDebug(5006) << "Format: " << event->format() << "" << event->pos() << endl;

  TQListViewItem * item = itemAt(event->pos());

  if (item &&
      TQString(event->format()).startsWith("text/plain") &&
      static_cast<SnippetWidget *>(event->source()) != this) {
    ///kdDebug(5006) << "returning TRUE " << endl;
    return TRUE;
  } else if(item &&
      TQString(event->format()).startsWith("x-kmailsnippet") &&
      static_cast<SnippetWidget *>(event->source()) != this)
  {
    //kdDebug(5006) << "returning TRUE " << endl;
    return TRUE;
  } else {
    //kdDebug(5006) << "returning FALSE" << endl;
    event->acceptAction(FALSE);
    return FALSE;
  }
}

//  fn SnippetWidget::slotDropped(TQDropEvent *e, TQListViewItem *after)
/*!
    This slot is connected to the dropped signal.
    If it is emitted, we need to construct a new snippet entry with
    the data given
 */
void SnippetWidget::slotDropped(TQDropEvent *e, TQListViewItem *)
{
  TQListViewItem * item2 = itemAt(e->pos());

  SnippetGroup *group = dynamic_cast<SnippetGroup *>(item2);
  if (!group)
    group = dynamic_cast<SnippetGroup *>(item2->parent());

  TQCString dropped;
  TQByteArray data = e->encodedData("text/plain");
  if ( e->provides("text/plain") && data.size()>0 ) {
    //get the data from the event...
    TQString encData(data.data());
    //kdDebug(5006) << "encData: " << encData << endl;

    //... then fill the dialog with the given data
    SnippetDlg dlg( mActionCollection, this, "SnippetDlg" );
    dlg.snippetName->clear();
    dlg.snippetText->setText(encData);

    /*fill the combobox with the names of all SnippetGroup entries*/
    for (SnippetItem *it=_list.first(); it; it=_list.next()) {
      if (dynamic_cast<SnippetGroup*>(it)) {
        dlg.cbGroup->insertItem(it->getName());
      }
    }
    dlg.cbGroup->setCurrentText(group->getName());

    if (dlg.exec() == TQDialog::Accepted) {
      /* get the group that the user selected with the combobox */
      group = dynamic_cast<SnippetGroup*>(SnippetItem::findItemByName(dlg.cbGroup->currentText(), _list));
      _list.append( makeItem(group, dlg.snippetName->text(), dlg.snippetText->text(), dlg.keyButton->shortcut() ) );
    }
  }
}

void SnippetWidget::startDrag()
{
    TQString text = dynamic_cast<SnippetItem*>( currentItem() )->getText();
    TQTextDrag *drag = new TQTextDrag(text, this);
    drag->setSubtype("x-textsnippet");
    drag->drag();
}

void SnippetWidget::slotExecute()
{
    slotExecuted(currentItem());
}


#include "snippetwidget.moc"

