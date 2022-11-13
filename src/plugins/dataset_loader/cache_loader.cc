#include <libpressio_dataset_ext/loader.h>
#include <std_compat/memory.h>
#include <sstream>
namespace libpressio_dataset { namespace cache_loader_ns {
  struct cache_loader: public dataset_loader_base {

    size_t num_datasets_impl() override {
      if(!num_datasets_cache) {
        num_datasets_cache = loader->num_datasets();
      }
      return *num_datasets_cache;
    }

    int set_options_impl(pressio_options const& options) override {
      get_meta(options, "cache:loader", dataset_loader_plugins(), loader_id, loader);
      bool reset = false;
      if(get(options, "cache:flush", &reset) == pressio_options_key_set) {
        reset_cache();
      }
      return 0;
    }

    pressio_options get_options_impl() const override {
      pressio_options options;
      set_meta(options, "cache:loader", loader_id, loader);
      set_type(options, "cache:flush", pressio_option_bool_type);
      return options;
    }

    pressio_options get_documentation_impl() const override {
      pressio_options options;
      set_meta_docs(options, "cache:loader", "plugin to use for cache", loader);
      set(options, "cache:flush", "flush the cache");
      return options;
    }
    
    pressio_data load_data_impl(size_t n) override {
      auto it = data_cache.find(n);
      if(it == data_cache.end()) {
        data_cache[n] = loader->load_data(n);
      }
      return data_cache[n];
    }

    pressio_options load_metadata_impl(size_t n) override {
      auto it = metadata_cache.find(n);
      if(it == metadata_cache.end()) {
        metadata_cache[n] = loader->load_metadata(n);
        pressio_data dims;
        pressio_dtype dtype;
        metadata_cache[n].get(loader->get_name(), "loader:dims", &dims);
        metadata_cache[n].get(loader->get_name(), "loader:dtype", &dtype);
        set(metadata_cache[n], "loader:dims", dims);
        set(metadata_cache[n], "loader:dtype", dtype);
      }
      return metadata_cache[n];
    }

    std::unique_ptr<dataset_loader> clone() override {
      return std::make_unique<cache_loader>(*this);
    }

    void reset_cache() {
      num_datasets_cache.reset();
      metadata_cache.clear();
      data_cache.clear();
    }

    const char* prefix() const override {
      return "cache";
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

    std::optional<size_t> num_datasets_cache;
    std::map<size_t,pressio_options> metadata_cache;
    std::map<size_t,pressio_data> data_cache;
  };

  pressio_register cache_loader_register(dataset_loader_plugins(), "cache", []{ return compat::make_unique<cache_loader>(); });
}}
