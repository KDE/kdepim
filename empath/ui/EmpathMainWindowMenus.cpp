#include <kmenubar.h>

#include "EmpathDefines.h"

extern QString EmpathAboutText;

void
EmpathMainWindow::_setupMenuBar()
{
    fileMenu_       = new QPopupMenu;
    selectMenu_     = new QPopupMenu;
    folderMenu_     = new QPopupMenu;
    messageMenu_    = new QPopupMenu;
    optionsMenu_    = new QPopupMenu;
    optionsMenu_->setCheckable(true);

    // File menu
    
    fileMenu_->insertItem(empathIcon("menu-send"), i18n("&Send Pending Mail"),
        this, SLOT(s_fileSendNew()));

    fileMenu_->insertItem(i18n("&Close"),
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

    
    messageMenu_->insertItem(empathIcon("menu-view"),
        i18n("&View"),
        this, SLOT(s_messageView()));
    
    messageMenu_->insertSeparator();

    messageListWidget_->messageCompose->plug(messageMenu_);
    messageListWidget_->messageReply->plug(messageMenu_);
    messageListWidget_->messageReplyAll->plug(messageMenu_);
    messageListWidget_->messageForward->plug(messageMenu_);
    
/*
    messageMenu_->insertItem(empathIcon("menu-compose"),
        i18n("&New"),
        this, SLOT(s_messageNew()));

    messageMenu_->insertItem(empathIcon("menu-reply"),
        i18n("&Reply"),
        this, SLOT(s_messageReply()));

    messageMenu_->insertItem(empathIcon("menu-reply"),
        i18n("Reply to &All"),
        this, SLOT(s_messageReplyAll()));

    messageMenu_->insertItem(empathIcon("menu-forward"),
        i18n("&Forward"),
        this, SLOT(s_messageForward()));
*/

    messageMenu_->insertItem(empathIcon("menu-bounce"),
        i18n("&Bounce"),
        this, SLOT(s_messageBounce()));

    messageListWidget_->messageDelete->plug(messageMenu_);
    messageListWidget_->messageSaveAs->plug(messageMenu_);

/*
    messageMenu_->insertItem(empathIcon("menu-delete"),
        i18n("&Delete"),
        this, SLOT(s_messageDelete()));

    messageMenu_->insertItem(empathIcon("menu-save"),
        i18n("Save &As"),
        this, SLOT(s_messageSaveAs()));
*/
    
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
    
//    messageMenu_->insertItem(
//        i18n("Fil&ter"),
//        this, SLOT(s_messageFilter()));

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
    
    menu_->insertItem(i18n("&File"), fileMenu_);
    menu_->insertItem(i18n("&Select"), selectMenu_);
    menu_->insertItem(i18n("&Message"), messageMenu_);
    menu_->insertItem(i18n("&Options"), optionsMenu_);
    menu_->insertSeparator();
    menu_->insertItem(i18n("&Help"), helpMenu_);
}



// vim:ts=4:sw=4:tw=78
