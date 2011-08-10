
#ifndef MAILCOMMON_EXPIRYPROPERTIESDIALOG_H
#define MAILCOMMON_EXPIRYPROPERTIESDIALOG_H

#include "mailcommon_export.h"

#include <kdialog.h>
#include <QSharedPointer>

class QCheckBox;
class QRadioButton;
class KIntSpinBox;

namespace MailCommon {

class FolderRequester;
class FolderCollection;
class Kernel;

class MAILCOMMON_EXPORT ExpiryPropertiesDialog : public KDialog
{
    Q_OBJECT

public:
    explicit ExpiryPropertiesDialog(
      QWidget *tree,
      const QSharedPointer<FolderCollection> &folder);
    ~ExpiryPropertiesDialog();

protected slots:
    void accept();
    void slotUpdateControls();

private:
    QSharedPointer<FolderCollection> mFolder;

    QCheckBox *expireReadMailCB;
    KIntSpinBox *expireReadMailSB;
    QCheckBox *expireUnreadMailCB;
    KIntSpinBox *expireUnreadMailSB;
    QRadioButton *moveToRB;
    FolderRequester *folderSelector;
    QRadioButton *deletePermanentlyRB;
};

} // namespace

#endif
