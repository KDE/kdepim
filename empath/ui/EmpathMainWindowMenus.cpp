#include <kmenubar.h>

#include "EmpathDefines.h"

extern QString EmpathAboutText;

void
EmpathMainWindow::_setupMenuBar()
{
    messageMenu_    = new QPopupMenu;
    selectMenu_     = new QPopupMenu;
    goMenu_         = new QPopupMenu;
    folderMenu_     = new QPopupMenu;
    threadMenu_     = new QPopupMenu;
    optionsMenu_    = new QPopupMenu;
    optionsMenu_->setCheckable(true);

    QActionCollection * actionCollection 
            = messageListWidget_->actionCollection();

    // Message menu
   
    actionCollection->action("messageView")->plug(messageMenu_);
    
    messageMenu_->insertSeparator();

    actionCollection->action("messageCompose")->plug(messageMenu_);
    actionCollection->action("messageReply")->plug(messageMenu_);
    actionCollection->action("messageReplyAll")->plug(messageMenu_);
    actionCollection->action("messageForward")->plug(messageMenu_);
    
    messageMenu_->insertItem(empathIcon("menu-bounce"),
        i18n("&Bounce"),
        this, SLOT(s_messageBounce()));

    actionCollection->action("messageDelete")->plug(messageMenu_);
    actionCollection->action("messageSaveAs")->plug(messageMenu_);

    messageMenu_->insertItem(empathIcon("menu-copy"),
        i18n("&Copy to..."),
        this, SLOT(s_messageCopyTo()));
    
    messageMenu_->insertItem(empathIcon("menu-move"),
        i18n("&Move to..."),
        this, SLOT(s_messageMoveTo()));
    
    messageMenu_->insertSeparator();
        
    messageMenu_->insertItem(empathIcon("menu-print"),
        i18n("&Print") + "...",
        this, SLOT(s_messagePrint()));
    
    messageMenu_->insertSeparator();

    messageMenu_->insertItem(empathIcon("menu-send"), i18n("&Send Pending Mail"),
        this, SLOT(s_fileSendNew()));
    
    messageMenu_->insertItem(i18n("&Close"),
        this, SLOT(s_fileClose()));
 
    // Select menu
    
    selectMenu_->insertItem(empathIcon("tree-marked"), i18n("&Tagged"),
        this, SLOT(s_editSelectTagged()));
    
    selectMenu_->insertItem(empathIcon("tree-read"), i18n("&Read"),
        this, SLOT(s_editSelectRead()));
    
    selectMenu_->insertItem(i18n("&All"),
        this, SLOT(s_editSelectAll()));
    
    selectMenu_->insertItem(i18n("&Invert"),
        this, SLOT(s_editInvertSelection()));
  
    // Go menu
  
    actionCollection->action("goPrevious")->plug(goMenu_);
    actionCollection->action("goNext")->plug(goMenu_);
    actionCollection->action("goNextUnread")->plug(goMenu_);
    
    // Folder menu

    folderMenu_->insertItem(empathIcon("folder-normal"),
        i18n("&New") + "...",
        this, SLOT(s_folderNew()));

    folderMenu_->insertItem(
        i18n("&Properties") + "...",
        this, SLOT(s_folderEdit()));

    folderMenu_->insertItem(empathIcon("folder-outbox"),
        i18n("&Clear") + "...",
        this, SLOT(s_folderClear()));

    folderMenu_->insertItem(
        i18n("Delete") + "...",
        this, SLOT(s_folderDelete()));
   
    // Thread menu
    
    actionCollection->action("threadExpand")->plug(threadMenu_);
    actionCollection->action("threadCollapse")->plug(threadMenu_);
    
    // Options menu

    hideReadIndex_ = optionsMenu_->insertItem(empathIcon("tree"),
        i18n("Show only &new messages"),
        messageListWidget_, SLOT(s_hideRead()));

    optionsMenu_->setItemChecked(hideReadIndex_, false);

    optionsMenu_->insertSeparator();

    optionsMenu_->insertItem(empathIcon("settings-display"),
        i18n("&Display"),
        this, SLOT(s_setupDisplay()));
    
    optionsMenu_->insertItem(empathIcon("settings-identity"),
        i18n("&Identity"),
        this, SLOT(s_setupIdentity()));
    
    optionsMenu_->insertItem(empathIcon("settings-compose"),
        i18n("&Compose"),
        this, SLOT(s_setupComposing()));
    
    optionsMenu_->insertItem(empathIcon("settings-sending"),
        i18n("&Sending"),
        this, SLOT(s_setupSending()));
    
    optionsMenu_->insertItem(empathIcon("settings-accounts"),
        i18n("&Accounts"),
        this, SLOT(s_setupAccounts()));
    
    optionsMenu_->insertItem(empathIcon("menu-filter"),
        i18n("&Filters"),
        this, SLOT(s_setupFilters()));
    
    helpMenu_ = helpMenu(EmpathAboutText);
    
    menu_->insertItem(i18n("&Message"), messageMenu_);
    menu_->insertItem(i18n("&Select"), selectMenu_);
    menu_->insertItem(i18n("&Go"), goMenu_);
    menu_->insertItem(i18n("&Thread"), threadMenu_);
    menu_->insertItem(i18n("&Options"), optionsMenu_);
    menu_->insertSeparator();
    menu_->insertItem(i18n("&Help"), helpMenu_);
}



// vim:ts=4:sw=4:tw=78
