#include <libpressio_dataset_ext/loader.h>
#include <std_compat/memory.h>
#include <sstream>

namespace libpressio_dataset { namespace io_loader {

  struct io_loader: public dataset_loader_base {

    size_t num_datasets_impl() override {
      return 1;
    }

    int set_options_impl(pressio_options const& options) override {
      get_meta(options, "io_loader:plugin", io_plugins(), io, io_plugin);
      pressio_data tmp_dims;
      if(get(options, "io_loader:dims", &tmp_dims) == pressio_options_key_set) {
        dims = tmp_dims.to_vector<size_t>();
      }
      get(options, "io_loader:dtype", &dtype);
      get(options, "io_loader:use_template", &use_template);
      return 0;
    }

    pressio_options get_options_impl() const override {
      pressio_options options;
      set_meta(options, "io_loader:plugin", io, io_plugin);
      set(options, "io_loader:dims", pressio_data(dims.begin(), dims.end()));
      set(options, "io_loader:dtype", dtype);
      set(options, "io_loader:use_template", use_template);
      return options;
    }

    pressio_options get_documentation_impl() const override {
      pressio_options options;
      set_meta_docs(options, "io_loader:plugin", "io plugin to load the data", io_plugin);
      return options;
    }
    
    pressio_data load_data_impl(size_t) override {
      if (use_template) {
        pressio_data template_data(pressio_data::owning(dtype, dims));
        pressio_data* ptr = io_plugin->read(&template_data);
        if(ptr == nullptr) {
          throw std::runtime_error(io_plugin->error_msg());
        }
        pressio_data out = std::move(*ptr);
        pressio_data_free(ptr);
        return out;
      } else {
        pressio_data* ptr = io_plugin->read(nullptr);
        if(ptr == nullptr) {
          throw std::runtime_error(io_plugin->error_msg());
        }
        pressio_data out = std::move(*ptr);
        pressio_data_free(ptr);
        return out;
      }
    }

    pressio_options load_metadata_impl(size_t) override {
      pressio_options metadata;
      if(use_template) {
        set(metadata, "loader:dims", pressio_data(dims.begin(), dims.end()));
        set(metadata, "loader:dtype", dtype);
      } else {
        pressio_data data = std::move(*io_plugin->read(nullptr));
        auto const& ddims = data.dimensions();
        set(metadata, "loader:dims", pressio_data(ddims.begin(), ddims.end()));
        set(metadata, "loader:dtype", data.dtype());

      }
      return metadata;
    }

    std::unique_ptr<dataset_loader> clone() override {
      return std::make_unique<io_loader>(*this);
    }

    void set_name_impl(std::string const& new_name) override {
      io_plugin->set_name(new_name + '/' + io_plugin->prefix());
    }

    const char* prefix() const override {
      return "io_loader";
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

    std::string io = "posix";
    pressio_io io_plugin = io_plugins().build(io);
    bool use_template = false;
    std::vector<size_t> dims;
    pressio_dtype dtype;
  };

  pressio_register io_loader_register(dataset_loader_plugins(), "io_loader", []{ return compat::make_unique<io_loader>(); });
}}
