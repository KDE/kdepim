#include "EmpathEnum.h"
#include "EmpathUIUtils.h"
#include <kmenubar.h>

    void
EmpathComposeWindow::setupMenuBar()
{
    fileMenu_       = new QPopupMenu;
    editMenu_       = new QPopupMenu;
    attachmentMenu_ = new QPopupMenu;
    messageMenu_    = new QPopupMenu;
    priorityMenu_   = new QPopupMenu;

    QActionCollection * actionCollection 
            = composeWidget_->actionCollection();

    // File menu
    
    fileMenu_->insertItem(empathIcon("menu-send"), i18n("&Send Message"),
        this, SLOT(s_fileSendMessage()));
    
    fileMenu_->insertItem(empathIcon("menu-sendlater"), i18n("Send &Later"),
        this, SLOT(s_fileSendLater()));
    
    fileMenu_->insertSeparator();

    fileMenu_->insertItem(i18n("&Close"), this, SLOT(s_fileClose()));

    // Edit menu
    
    editMenu_->insertItem(i18n("&Undo"),
        composeWidget_, SLOT(s_undo()));
    
    editMenu_->insertItem(i18n("&Redo"),
        composeWidget_, SLOT(s_redo()));
    
    editMenu_->insertSeparator();

    editMenu_->insertItem(empathIcon("cut"), i18n("Cu&t"),
        this, SLOT(s_editCut()));
    
    editMenu_->insertItem(empathIcon("copy"), i18n("&Copy"),
        this, SLOT(s_editCopy()));
    
    editMenu_->insertItem(empathIcon("paste"), i18n("&Paste"),
        this, SLOT(s_editPaste()));
    
//    editMenu_->insertItem(empathIcon("blank"), i18n("&Delete"),
//        this, SLOT(s_editDelete()));

    editMenu_->insertSeparator();
    
    editMenu_->insertItem(i18n("&Select All"), this, SLOT(s_editSelectAll()));
    
#if 0
    editMenu_->insertSeparator();
    
    editMenu_->insertItem(empathIcon("blank"), i18n("Find..."),
        this, SLOT(s_editFind()));
    
    editMenu_->insertItem(empathIcon("blank"), i18n("Find &Again"),
        this, SLOT(s_editFindAgain()));
#endif
    
    // Attachment menu
  
    actionCollection->action("attachmentAdd")->plug(attachmentMenu_);
  
/*   
    attachmentMenu_->insertItem(i18n("&Add attachment"),
        composeWidget_, SLOT(s_addAttachment()));
    
    attachmentMenu_->insertItem(i18n("&Edit attachment"),
        composeWidget_, SLOT(s_editAttachment()));
    
    attachmentMenu_->insertItem(i18n("&Remove attachment"),
        composeWidget_, SLOT(s_removeAttachment()));
*/
    
    // Message Menu
    messageMenu_->insertItem(empathIcon("menu-compose"), i18n("&New"),
        empath, SLOT(s_compose()));

    messageMenu_->insertItem(empathIcon("menu-save"), i18n("Save &As"),
        this, SLOT(s_messageSaveAs()));

    messageMenu_->insertItem(empathIcon("menu-copy"), i18n("&Copy to..."),
        this, SLOT(s_messageCopyTo()));

    priorityMenu_->insertItem(i18n("Highest"),  Highest);
    priorityMenu_->insertItem(i18n("High"),     High);
    priorityMenu_->insertItem(i18n("Normal"),   Normal);
    priorityMenu_->insertItem(i18n("Low"),      Low);
    priorityMenu_->insertItem(i18n("Lowest"),   Lowest);
    
    menuBar()->insertItem(i18n("&File"), fileMenu_);
    menuBar()->insertItem(i18n("&Edit"), editMenu_);
    menuBar()->insertItem(i18n("&Attachment"), attachmentMenu_);
    menuBar()->insertItem(i18n("&Message"), messageMenu_);
    menuBar()->insertItem(i18n("&Priority"), priorityMenu_);
    menuBar()->insertSeparator();
    menuBar()->insertItem(i18n("&Help"), helpMenu());
}


// vim:ts=4:sw=4:tw=78
