#ifndef ABSTRACTCONTACTEDITORWIDGET_H
#define ABSTRACTCONTACTEDITORWIDGET_H

#include <QtGui/QWidget>

namespace KABC
{
class Addressee;
}

class AbstractContactEditorWidget : public QWidget
{
  public:
    virtual void loadContact( const KABC::Addressee &contact ) = 0;
    virtual void storeContact( KABC::Addressee &contact ) const = 0;
};

#endif
