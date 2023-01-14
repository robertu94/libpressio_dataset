#include "libpressio_dataset_ext/loader.h"
#include <cassert>
#include <cstring>

extern "C" {

    using namespace libpressio_dataset;
    using namespace std::string_literals;

pressio_dataset_loader* pressio_get_dataset_loader(struct pressio* library, const char* id) {
    assert(library && "library cannot be null");
    assert(id && "id cannot be null");
    auto plugin = dataset_loader_plugins().build(id);
    if(plugin != nullptr) {
        return new pressio_dataset_loader(std::move(plugin));
    } else {
        library->set_error(1, "unable to find loader "s + id);
        return nullptr;
    }
}

void pressio_dataset_loader_free(struct pressio_dataset_loader* loader) {
    if(loader) delete loader;
}


struct pressio_options* pressio_dataset_loader_get_options(struct pressio_dataset_loader const* loader) {
   assert(loader && "loader cannot be null");
   return new pressio_options((*loader)->get_options()); 
}

struct pressio_options* pressio_dataset_loader_get_configuration(struct pressio_dataset_loader const* loader) {
   assert(loader && "loader cannot be null");
   return new pressio_options((*loader)->get_configuration()); 
}

struct pressio_options* pressio_dataset_loader_get_documentation(struct pressio_dataset_loader const* loader) {
   assert(loader && "loader cannot be null");
   return new pressio_options((*loader)->get_documentation()); 
}

int pressio_dataset_loader_set_options(struct pressio_dataset_loader* loader, struct pressio_options const* options) {
   assert(loader && "loader cannot be null");
    return (*loader)->set_options(*options);
}

int pressio_dataset_loader_check_options(struct pressio_dataset_loader* loader, struct pressio_options const* options) {
   assert(loader && "loader cannot be null");
    return (*loader)->check_options(*options);
}

void pressio_dataset_loader_set_name(struct pressio_dataset_loader* loader, const char* new_name) {
   assert(loader && "loader cannot be null");
   assert(new_name && "new_name cannot be null");
    return (*loader)->set_name(new_name);
}

const char* pressio_dataset_loader_get_name(struct pressio_dataset_loader const* loader) {
   assert(loader && "loader cannot be null");
    return strdup((*loader)->get_name().c_str());
}

const char* pressio_dataset_loader_get_prefix(const struct pressio_dataset_loader* loader) {
   assert(loader && "loader cannot be null");
    return strdup((*loader)->prefix());
}

struct pressio_dataset_loader* pressio_dataset_loader_clone(struct pressio_dataset_loader* loader) {
    assert(loader && "loader cannot be null");
    return new pressio_dataset_loader((*loader)->clone());
}

const char* pressio_dataset_loader_version(struct pressio_dataset_loader const* dataset_loader) {
    assert(dataset_loader && "loader cannot be null");
    return (*dataset_loader)->version();
}

int pressio_dataset_loader_major_version(struct pressio_dataset_loader const* dataset_loader) {
    assert(dataset_loader && "loader cannot be null");
    return (*dataset_loader)->major_version();
}

int pressio_dataset_loader_minor_version(struct pressio_dataset_loader const* dataset_loader) {
    assert(dataset_loader && "loader cannot be null");
    return (*dataset_loader)->minor_version();
}

int pressio_dataset_loader_patch_version(struct pressio_dataset_loader const* dataset_loader) {
    assert(dataset_loader && "loader cannot be null");
    return (*dataset_loader)->patch_version();
}


int pressio_dataset_loader_error_code(struct pressio_dataset_loader const* dataset_loader) {
    assert(dataset_loader && "loader cannot be null");
    return (*dataset_loader)->error_code();
}


const char* pressio_dataset_loader_error_msg(struct pressio_dataset_loader* dataset_loader) {
    assert(dataset_loader && "loader cannot be null");
    return (*dataset_loader)->error_msg();
}

int pressio_datset_loader_num_datasets(struct pressio_dataset_loader* dataset_loader, size_t* n) {
    assert(dataset_loader && "loader cannot be null");
    assert(n && "n cannot be null");
    try {
        *n = (*dataset_loader)->num_datasets();
        return 0;
    } catch(std::exception const& ex) {
        return (*dataset_loader)->set_error(1,ex.what());
    }
}

int pressio_dataset_loader_load_all_metadata(struct pressio_dataset_loader* dataset_loader, size_t* n, struct pressio_options*** metadata) {
    assert(dataset_loader && "loader cannot be null");
    assert(metadata && "loader cannot be null");
    assert(n && "n cannot be null");
    try {
        auto ret = (*dataset_loader)->load_all_metadata();
        *metadata = new pressio_options*[ret.size()];
        for (size_t i = 0; i < ret.size(); ++i) {
            *metadata[i] = new pressio_options(std::move(ret[i]));
        }
        return 0;
    } catch(std::exception const& ex) {
        return (*dataset_loader)->set_error(1,ex.what());
    }
}

int pressio_dataset_loader_load_all_data(struct pressio_dataset_loader* dataset_loader, size_t* n, struct pressio_data*** data) {
    assert(dataset_loader && "loader cannot be null");
    assert(data && "loader cannot be null");
    assert(n && "n cannot be null");
    try {
        auto ret = (*dataset_loader)->load_all_data();
        *data = new pressio_data*[ret.size()];
        for (size_t i = 0; i < ret.size(); ++i) {
            *data[i] = new pressio_data(std::move(ret[i]));
        }
        return 0;
    } catch(std::exception const& ex) {
        return (*dataset_loader)->set_error(1,ex.what());
    }
}

int pressio_dataset_loader_load_metadata(struct pressio_dataset_loader* dataset_loader, size_t n, struct pressio_options** metadata) {
    assert(dataset_loader && "loader cannot be null");
    assert(metadata && "loader cannot be null");
        try {
            *metadata = new pressio_options((*dataset_loader)->load_metadata(n));
            return 0;
        } catch(std::exception const& ex) {
            return (*dataset_loader)->set_error(1,ex.what());
        }
}

int pressio_dataset_loader_load_data(struct pressio_dataset_loader* dataset_loader, size_t n, struct pressio_data** data) {
    assert(dataset_loader && "loader cannot be null");
    assert(data && "loader cannot be null");
        try {
            *data = new pressio_data((*dataset_loader)->load_data(n));
            return 0;
        } catch(std::exception const& ex) {
            return (*dataset_loader)->set_error(1,ex.what());
        }
}
}
