#include <libpressio_dataset_ext/loader.h>
namespace libpressio_dataset {
  pressio_registry<std::unique_ptr<dataset_loader>>& dataset_loader_plugins() {
    static pressio_registry<std::unique_ptr<dataset_loader>> loader;
    return loader;
  }
}
