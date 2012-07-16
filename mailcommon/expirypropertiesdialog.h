
#ifndef MAILCOMMON_EXPIRYPROPERTIESDIALOG_H
#define MAILCOMMON_EXPIRYPROPERTIESDIALOG_H

#include "mailcommon_export.h"

#include <Akonadi/Collection>
#include <kdialog.h>

class QCheckBox;
class QRadioButton;
class KIntSpinBox;

class KJob;

namespace MailCommon {

class FolderRequester;

class MAILCOMMON_EXPORT ExpiryPropertiesDialog : public KDialog
{
    Q_OBJECT

public:
  explicit ExpiryPropertiesDialog(QWidget *parent, const Akonadi::Collection &collection);
    ~ExpiryPropertiesDialog();

protected slots:
    void accept();
    void slotUpdateControls();
    void slotCollectionModified(KJob* job);
    void slotChanged();
private:
    Akonadi::Collection mCollection;
    bool mChanged;
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
