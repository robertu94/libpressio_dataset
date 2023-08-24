
#include <libpressio_dataset_ext/loader.h>
#include <std_compat/memory.h>
#include <sstream>

namespace libpressio_dataset { namespace from_data_loader_ns {
  struct from_data_loader: public dataset_loader_base {

    size_t num_datasets_impl() override {
      return data.size();
    }

    int set_options_impl(pressio_options const& options) override {
      uint64_t n = 0;
      if(get(options, "from_data:n", &n) == pressio_options_key_set) {
          data.resize(n);
      }
      uint64_t i = 0;
      for (auto& datum : data) {
          get(options, "from_data:data-" + std::to_string(i),  &datum);
          ++i;
      }
      return 0;
    }

    pressio_options get_options_impl() const override {
      pressio_options options;
      set(options, "from_data:n", static_cast<uint64_t>(data.size()));
      size_t i = 0;
      for (auto const& datum : data) {
          set(options, "from_data:data-" + std::to_string(i),  datum);
          ++i;
      }
      return options;
    }

    pressio_options get_documentation_impl() const override {
      pressio_options options;
      set(options, "from_data:n", "number of data to provide");
      size_t i = 0;
      for (auto const& datum : data) {
          (void)datum;
          set(options, "from_data:data-" + std::to_string(i),  "data element " + std::to_string(i));
          ++i;
      }
      return options;
    }
    
    pressio_data load_data_impl(size_t n) override {
      return data[n];
    }

    pressio_options load_metadata_impl(size_t i) override {
      pressio_data dims = pressio_data(data[i].dimensions().begin(), data[i].dimensions().end());
      pressio_options opts;
      set(opts, "loader:dims", dims);
      set(opts, "loader:dtype", data[i].dtype());
      return opts;
    }

    std::unique_ptr<dataset_loader> clone() override {
            return std::make_unique<from_data_loader>(*this);
    }

    const char* prefix() const override {
      return "from_data";
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

    std::vector<pressio_data> data;
  };

  pressio_register from_data_loader_register(dataset_loader_plugins(), "from_data", []{ return compat::make_unique<from_data_loader>(); });
}}

