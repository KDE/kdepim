/****************************************************************************
** Form interface generated from reading ui file 'expirypropertiesdialog.ui'
**
** Created: Sat Jan 29 12:59:18 2005
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef EXPIRYPROPERTIESDIALOG_H
#define EXPIRYPROPERTIESDIALOG_H

#include <tqvariant.h>
#include <kdialogbase.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QGroupBox;
class QCheckBox;
class QSpinBox;
class QLabel;
class QRadioButton;
class QButtonGroup;
class KMFolderTree;
class KMFolder;

namespace KMail {

  class FolderRequester;

class ExpiryPropertiesDialog : public KDialogBase
{
    Q_OBJECT

public:
    ExpiryPropertiesDialog( KMFolderTree* tree, KMFolder* folder );
    ~ExpiryPropertiesDialog();

    TQCheckBox* expireReadMailCB;
    TQSpinBox* expireReadMailSB;
    TQLabel* labelDays;
    TQCheckBox* expireUnreadMailCB;
    TQSpinBox* expireUnreadMailSB;
    TQLabel* labelDays2;
    TQLabel* expiryActionLabel;
    TQRadioButton* moveToRB;
    FolderRequester *folderSelector;
    TQRadioButton* deletePermanentlyRB;
    TQLabel* note;
    TQButtonGroup* actionsGroup;

protected slots:
    void slotOk();
    void slotUpdateControls();

protected:
    TQVBoxLayout* globalVBox;
    TQHBoxLayout* readHBox;
    TQHBoxLayout* unreadHBox;
    TQHBoxLayout* expiryActionHBox;
    TQVBoxLayout* actionsHBox;
    TQHBoxLayout* moveToHBox;
    KMFolder* mFolder;
};
} // namespace
#endif // EXPIRYPROPERTIESDIALOG_H
