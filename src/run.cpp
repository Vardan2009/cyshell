#include "run.h"

cyScope::sptr cyScope::createGlobal() {
    // TODO: add environment variables (as strings)
    return std::make_unique<cyScope>();
}
