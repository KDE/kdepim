#include <config-kleopatra.h>

#include "initialization.h"

#include <assuan.h>
#include <gpg-error.h>

using namespace KleopatraClient;

Initialization::Initialization() {
    assuan_set_assuan_err_source( GPG_ERR_SOURCE_DEFAULT );
}

Initialization::~Initialization() {

}
