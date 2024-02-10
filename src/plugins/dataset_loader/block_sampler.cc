#include <libpressio_dataset_ext/loader.h>
#include <std_compat/memory.h>
#include <sstream>
#include <random>
namespace libpressio_dataset { namespace block_sampler_loader_ns {

  struct block_sampler_loader: public dataset_loader_base {

    size_t num_datasets_impl() override {
      return N * loader->num_datasets();
    }

    int set_options_impl(pressio_options const& options) override {
      pressio_data new_block_size;
      get_meta(options, "block_sampler:loader", dataset_loader_plugins(), loader_id, loader);
      if(get(options, "block_sampler:block_size", &new_block_size)==pressio_options_key_set) {
        block_size = new_block_size.to_vector<size_t>();
      }
      get(options, "block_sampler:n", &N);
      get(options, "block_sampler:seed", &seed);
      return 0;
    }

    pressio_options get_options_impl() const override {
      pressio_options options;
      set_meta(options, "block_sampler:loader", loader_id, loader);
      set(options, "block_sampler:block_size", pressio_data(block_size.begin(), block_size.end()));
      set(options, "block_sampler:n", N); set(options, "block_sampler:seed", seed);
      return options;
    }

    pressio_options get_documentation_impl() const override {
      pressio_options options;
      set_meta_docs(options, "block_sampler:loader", "loader to sample from", loader);
      set(options, "block_sampler:block_size", "block size to sample");
      set(options, "block_sampler:n", "number of samples to take from each data source");
      set(options, "block_sampler:seed", "seed");
      return options;
    }
    
    pressio_data load_data_impl(size_t n) override {
      pressio_data data = loader->load_data(n/N);
      return sample(data, seed+n);
    }

    pressio_options load_metadata_impl(size_t n) override {
      pressio_options metadata = loader->load_metadata(n/N);
      pressio_dtype dtype = pressio_byte_dtype;
      metadata.get(loader->get_name(), "loader:dtype", &dtype);
      set(metadata, "loader:dims", pressio_data(block_size.begin(), block_size.end()));
      set(metadata, "loader:dtype", dtype);
      return metadata;
    }

    pressio_data sample(pressio_data const& dat, size_t sample_seed) {
      std::seed_seq seed{sample_seed};
      std::mt19937 gen{seed};

      return pressio_data_for_each<pressio_data>(dat, [this, &dat, &gen](auto src, auto){
          pressio_data sample = pressio_data::owning(dat.dtype(), block_size);
          std::vector<size_t> const& dat_dims = dat.dimensions();
          std::vector<size_t> sampled_block(block_size.size());
          for (size_t i = 0; i < block_size.size(); ++i) {
            if(block_size[i] > dat_dims[i]) {
              throw std::runtime_error("block_size must be smaller than data size");
            }
            size_t upper_bound = 
              (dat_dims[i]%block_size[i] == 0) ? ((dat_dims[i]/block_size[i])-2):
                (dat_dims[i]/block_size[i] - 1);
            std::uniform_int_distribution<size_t> dist(0, upper_bound);
            sampled_block[i] = dist(gen);
          }
          using pointer = typename std::iterator_traits<decltype(src)>::pointer;
          pointer dst = static_cast<pointer>(sample.data());

          if(block_size.size() == 1) {
            size_t base0 = block_size[0]*sampled_block[0];
            for (size_t i = 0; i < block_size[0]; ++i) {
              dst[i] = src[base0+i];
            }
          }
          else if (block_size.size() == 2) {
            const size_t base0 = block_size[0]*sampled_block[0];
            const size_t base1 = block_size[1]*sampled_block[1];
            for (size_t i = 0; i < block_size[0]; ++i) {
                const size_t dst_offset_i = i*block_size[1];
                const size_t src_offset_i = (i+base0)*dat_dims[1];
            for (size_t j = 0; j < block_size[1]; ++j) {
                const size_t dst_idx = j + dst_offset_i;
                const size_t src_idx = (j+base1) + src_offset_i;
                dst[dst_idx] = src[src_idx];
            }}
          }
          else if (block_size.size() == 3) {
            const size_t base0 = block_size[0]*sampled_block[0];
            const size_t base1 = block_size[1]*sampled_block[1];
            const size_t base2 = block_size[2]*sampled_block[2];
            for (size_t i = 0; i < block_size[0]; ++i) {
                const size_t dst_offset_i = i*block_size[2]*block_size[1];
                const size_t src_offset_i = (i+base0)*dat_dims[2]*dat_dims[1];
            for (size_t j = 0; j < block_size[1]; ++j) {
                const size_t dst_offset_j = j*block_size[2] + dst_offset_i;
                const size_t src_offset_j = (j+base1)*dat_dims[2] + src_offset_i;
            for (size_t k = 0; k < block_size[2]; ++k) {
                const size_t dst_idx = k + dst_offset_j;
                const size_t src_idx = (k+base2) + src_offset_j;
                dst[dst_idx] = src[src_idx];
            }}}
          }
          else if (block_size.size() == 4) {
            const size_t base0 = block_size[0]*sampled_block[0];
            const size_t base1 = block_size[1]*sampled_block[1];
            const size_t base2 = block_size[2]*sampled_block[2];
            const size_t base3 = block_size[3]*sampled_block[3];
            for (size_t i = 0; i < block_size[0]; ++i) {
                const size_t dst_offset_i = i*block_size[3]*block_size[2]*block_size[1];
                const size_t src_offset_i = (i+base0)*dat_dims[3]*dat_dims[2]*dat_dims[1];
            for (size_t j = 0; j < block_size[1]; ++j) {
                const size_t dst_offset_j = j*block_size[3]*block_size[2] + dst_offset_i;
                const size_t src_offset_j = (j+base1)*dat_dims[3]*dat_dims[2] + src_offset_i;
            for (size_t k = 0; k < block_size[2]; ++k) {
                const size_t dst_offset_k = k*block_size[3] + dst_offset_j;
                const size_t src_offset_k = (k+base2)*dat_dims[3] + src_offset_j;
            for (size_t l = 0; l < block_size[3]; ++l) {
                const size_t dst_idx = l + dst_offset_k;
                const size_t src_idx = (base3+l) + src_offset_k;
                dst[dst_idx] = src[src_idx];
            }}}}
          }
          else {
            throw std::runtime_error("unsupported size");
          }

          return sample;
      });
    }

    void set_name_impl(std::string const& new_name) override {
      loader->set_name(new_name + "/" + loader->prefix());
    }

    std::unique_ptr<dataset_loader> clone() override {
      return std::make_unique<block_sampler_loader>(*this);
    }

    const char* prefix() const override {
      return "block_sampler";
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
    std::vector<size_t> block_size;
    std::string loader_id = "io_loader";
    pressio_dataset_loader loader = dataset_loader_plugins().build(loader_id);
  };

  pressio_register block_sampler_loader_register(dataset_loader_plugins(), "block_sampler", []{ return compat::make_unique<block_sampler_loader>(); });
}}
