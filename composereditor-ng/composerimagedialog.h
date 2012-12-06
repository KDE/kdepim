#ifndef COMPOSERIMAGEDIALOG_H
#define COMPOSERIMAGEDIALOG_H

#include <KDialog>

namespace ComposerEditorNG
{
class ComposerImageDialog : public KDialog
{
public:
    explicit ComposerImageDialog(QWidget *parent);
    ~ComposerImageDialog();
};
}

#endif // COMPOSERIMAGEDIALOG_H
