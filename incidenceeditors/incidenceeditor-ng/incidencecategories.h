#ifndef INCIDENCECATEGORIES_H
#define INCIDENCECATEGORIES_H

#include "incidenceeditor-ng.h"

namespace Ui {
class EventOrTodoDesktop;
}

namespace IncidenceEditorsNG {

class INCIDENCEEDITORS_NG_EXPORT IncidenceCategories : public IncidenceEditor
{
  Q_OBJECT

  public:
    IncidenceCategories( Ui::EventOrTodoDesktop *ui );

    virtual void load(KCal::Incidence::ConstPtr incidence);
    virtual void save(KCal::Incidence::Ptr incidence);
    virtual bool isDirty() const;

  private slots:
    void selectCategories();
    void setCategories( const QStringList &categories );

  private:
    QStringList mSelectedCategories;
    Ui::EventOrTodoDesktop *mUi;
};

}

#endif // INCIDENCECATEGORIES_H
