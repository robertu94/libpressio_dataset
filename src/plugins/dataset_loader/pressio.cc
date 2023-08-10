#include <libpressio_dataset_ext/loader.h>
#include <std_compat/memory.h>
#include <sstream>
namespace libpressio_dataset { namespace pressio_loader_ns {
  struct pressio_loader: public dataset_loader_base {

    size_t num_datasets_impl() override {
      return loader->num_datasets();
    }

    int set_options_impl(pressio_options const& options) override {
      get_meta(options, "pressio:loader", dataset_loader_plugins(), loader_id, loader);
      return 0;
    }

    pressio_options get_options_impl() const override {
      pressio_options options;
      set_meta(options, "pressio:loader", loader_id, loader);
      return options;
    }

    pressio_options get_documentation_impl() const override {
      pressio_options options;
      set_meta_docs(options, "pressio:loader", "base loader plugin", loader);
      return options;
    }
    
    pressio_data load_data_impl(size_t n) override {
      return loader->load_data(n);
    }

    pressio_options load_metadata_impl(size_t n) override {
        auto metadata = loader->load_metadata(n);
        pressio_data dims;
        pressio_dtype dtype;
        metadata.get(loader->get_name(), "loader:dims", &dims);
        metadata.get(loader->get_name(), "loader:dtype", &dtype);
        set(metadata, "loader:dims", dims);
        set(metadata, "loader:dtype", dtype);
        return metadata;
    }

    std::unique_ptr<dataset_loader> clone() override {
      return std::make_unique<pressio_loader>(*this);
    }

    const char* prefix() const override {
      return "pressio";
    }
    const char* version() const override{
      static std::string s = [this]{
        std::stringstream ss;
        ss << this->major_version() << '.';
        ss << this->minor_version() << '.';
        ss << this->patch_version();
        return ss.str();
      }();
      return s.c_str();
    }

    void set_name_impl(std::string const& new_name) override {
      loader->set_name(new_name + "/" + loader->prefix());
    }

    std::string loader_id = "io_loader";
    pressio_dataset_loader loader = dataset_loader_plugins().build(loader_id);
  };

  pressio_register pressio_loader_register(dataset_loader_plugins(), "pressio", []{ return compat::make_unique<pressio_loader>(); });
}}
