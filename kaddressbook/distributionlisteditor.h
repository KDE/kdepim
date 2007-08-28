#ifndef KPIM_DISTRIBUTIONLISTEDITOR_H
#define KPIM_DISTRIBUTIONLISTEDITOR_H

#include <kdialogbase.h>

namespace KABC {
    class AddressBook;
}

namespace KPIM {

class DistributionList;

namespace DistributionListEditor {

class EditorWidgetPrivate;
class EditorWidget : public KDialogBase
{
    Q_OBJECT
public:
    explicit EditorWidget( KABC::AddressBook* book, QWidget* parent = 0 );
    ~EditorWidget();

    void setDistributionList( const KPIM::DistributionList& list );
    KPIM::DistributionList distributionList() const;

private:
    EditorWidgetPrivate* const d;
};

} // namespace DisributionListEditor
} // namespace KPIM

#endif // KPIM_DISTRIBUTIONLISTEDITOR_H
