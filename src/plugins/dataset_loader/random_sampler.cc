#include <libpressio_dataset_ext/loader.h>
#include <std_compat/memory.h>
#include <sstream>
#include <random>
namespace libpressio_dataset { namespace random_sampler_loader_ns {

  struct random_sampler_loader: public dataset_loader_base {

    void scan() {
      if(!sample) {
          sample = std::vector<size_t>();

          std::seed_seq seed{this->seed};
          std::mt19937 gen{seed};
          std::uniform_int_distribution<size_t> dist(0, loader->num_datasets());
          sample->reserve(N);
          for (uint64_t i = 0; i < N; ++i) {
              sample->emplace_back(dist(gen));
          }
      }
    }

    size_t num_datasets_impl() override {
      scan();
      return N;
    }

    int set_options_impl(pressio_options const& options) override {
      pressio_data new_block_size;
      get_meta(options, "random_sampler:loader", dataset_loader_plugins(), loader_id, loader);
      get(options, "random_sampler:n", &N);
      get(options, "random_sampler:seed", &seed);
      return 0;
    }

    pressio_options get_options_impl() const override {
      pressio_options options;
      set_meta(options, "random_sampler:loader", loader_id, loader);
      set(options, "random_sampler:n", N);
      set(options, "random_sampler:seed", seed);
      return options;
    }

    pressio_options get_documentation_impl() const override {
      pressio_options options;
      set_meta_docs(options, "random_sampler:loader", "loader to sample from", loader);
      set(options, "random_sampler:n", "number of samples to take from the data source");
      set(options, "random_sampler:seed", "seed");
      return options;
    }
    
    pressio_data load_data_impl(size_t n) override {
      scan();
      return loader->load_data(sample->at(n));
    }

    pressio_options load_metadata_impl(size_t n) override {
      scan();
      pressio_options metadata = loader->load_metadata(sample->at(n));
      pressio_dtype dtype;
      pressio_data dims;
      metadata.get(loader->get_name(), "loader:dims", &dims);
      metadata.get(loader->get_name(), "loader:dtype", &dtype);

      set(metadata, "loader:dims", dims);
      set(metadata, "loader:dtype", dtype);
      return metadata;
    }

    void set_name_impl(std::string const& new_name) override {
      loader->set_name(new_name + "/" + loader->prefix());
    }

    std::unique_ptr<dataset_loader> clone() override {
      return std::make_unique<random_sampler_loader>(*this);
    }

    const char* prefix() const override {
      return "random_sampler";
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

    uint64_t seed = 0;
    uint64_t N = 1;
    compat::optional<std::vector<size_t>> sample;
    std::string loader_id = "io_loader";
    pressio_dataset_loader loader = dataset_loader_plugins().build(loader_id);
  };

  pressio_register random_sampler_loader_register(dataset_loader_plugins(), "random_sampler", []{ return compat::make_unique<random_sampler_loader>(); });
}}

