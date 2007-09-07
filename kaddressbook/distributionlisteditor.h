#ifndef KPIM_DISTRIBUTIONLISTEDITOR_H
#define KPIM_DISTRIBUTIONLISTEDITOR_H

#include <KDialog>

namespace KABC {
    class AddressBook;
}

namespace KPIM {

class DistributionList;

namespace DistributionListEditor {

class EditorWidgetPrivate;
class EditorWidget : public KDialog
{
    Q_OBJECT
public:
    explicit EditorWidget( KABC::AddressBook* book, QWidget* parent = 0 );
    ~EditorWidget();

    void setDistributionList( const KPIM::DistributionList& list );
    KPIM::DistributionList distributionList() const;

private slots:

    void saveList();
    void lineTextChanged( int id );

private:
    EditorWidgetPrivate* const d;
};

} // namespace DisributionListEditor
} // namespace KPIM

#endif // KPIM_DISTRIBUTIONLISTEDITOR_H
