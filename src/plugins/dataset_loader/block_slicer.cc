#include <libpressio_dataset_ext/loader.h>
#include <std_compat/memory.h>
#include <std_compat/optional.h>
#include <std_compat/numeric.h>
#include <std_compat/functional.h>
#include <sstream>
#include <numeric>
#include <iterator>
#include <cmath>
#include <functional>
namespace libpressio_dataset { namespace block_slicer_loader_ns {
  template <class SizeType, SizeType N>
  struct basic_indexer {
    template <class... T, typename std::enable_if<compat::conjunction<std::is_integral<typename std::decay<T>::type>...>::value,int>::type = 0>
    basic_indexer(T&&... args) noexcept: max_dims{static_cast<SizeType>(args)...} {}

    basic_indexer(std::array<SizeType,N> args) noexcept: max_dims(args) {}

    basic_indexer(std::initializer_list<SizeType> args) noexcept: max_dims([](std::initializer_list<SizeType> args){
          std::array<SizeType,N> dims;
          std::copy(args.begin(), args.end(), dims.begin());
          return dims;
        }(args)) {}

    template <class It>
    basic_indexer(It first, It second) noexcept:
      max_dims([](It first, It second){
          std::array<SizeType,N> dims;
          std::copy(first, second, dims.begin());
          return dims;
        }(first, second)) {
      }

    template <class... T>
    typename std::enable_if<compat::conjunction<std::is_integral<typename std::decay<T>::type>...>::value && sizeof...(T) >= 1,std::size_t>::type
    operator()(T&&... args) const noexcept {
      std::array<SizeType, sizeof...(T)> dims{static_cast<SizeType>(args)...};
      return operator()(dims);
    }
    SizeType operator()(std::array<SizeType, N> const idxs) const noexcept {
      SizeType idx = idxs.back();
      SizeType i = N-1;
      do  {
        i--;
        idx*= max_dims[i]; 
        idx+= idxs[i];
      } while (i);
      return idx;
    }

    SizeType operator[](std::size_t i) const noexcept {
      return max_dims[i]; }
    SizeType size() const noexcept {
      return std::accumulate(max_dims.begin(), max_dims.end(), SizeType{1}, compat::multiplies<>{});
    }

    std::vector<SizeType> as_vec() {
      return std::vector<SizeType>(max_dims.begin(), max_dims.end());
    }

    std::array<SizeType, N> const max_dims;
  };

  template <size_t N>
  using indexer = basic_indexer<size_t, N>;
  struct block_slicer_loader: public dataset_loader_base {

    size_t num_datasets_impl() override {
      if(!N) {
          set_n();
      }
      return *N * loader->num_datasets();
    }

    int set_options_impl(pressio_options const& options) override {
      pressio_data new_block_size;
      get_meta(options, "block_slicer:loader", dataset_loader_plugins(), loader_id, loader);
      if(get(options, "block_slicer:block_size", &new_block_size)==pressio_options_key_set) {
        block_size = new_block_size.to_vector<size_t>();
      }
      return 0;
    }

    pressio_options get_options_impl() const override {
      pressio_options options;
      set_meta(options, "block_slicer:loader", loader_id, loader);
      set(options, "block_slicer:block_size", pressio_data(block_size.begin(), block_size.end()));
      return options;
    }

    pressio_options get_documentation_impl() const override {
      pressio_options options;
      set_meta_docs(options, "block_slicer:loader", "loader to sample from", loader);
      set(options, "block_slicer:block_size", "block size to sample");
      return options;
    }
    
    pressio_data load_data_impl(size_t n) override {
      if(!N) set_n();
      pressio_data data = loader->load_data(n/ *N);
      return sample(data, n % *N);
    }

    pressio_options load_metadata_impl(size_t n) override {
      if(!N) set_n();
      pressio_options metadata = loader->load_metadata(n / *N);
      pressio_dtype dtype = pressio_byte_dtype;
      metadata.get(loader->get_name(), "loader:dtype", &dtype);
      set(metadata, "loader:dims", pressio_data(block_size.begin(), block_size.end()));
      set(metadata, "loader:dtype", dtype);
      return metadata;
    }

    std::unique_ptr<dataset_loader> clone() override {
            return std::make_unique<block_slicer_loader>(*this);
    }

    const char* prefix() const override {
      return "block_slicer";
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
      private:

    pressio_data sample(pressio_data const& dat, size_t sample_seed) {
      return pressio_data_for_each<pressio_data>(dat, [this, &dat, sample_seed](auto src, auto){
          pressio_data sample = pressio_data::owning(dat.dtype(), block_size);
          std::vector<size_t> const& dat_dims = dat.dimensions();
          if(block_size.size() != dat_dims.size()) {
            throw std::runtime_error("expected block ndims and data ndims to be the same");
          }

          std::vector<size_t> n_blocks(dat_dims.size());
          for (size_t i = 0; i < dat_dims.size(); ++i) {
             if(dat_dims[i] % block_size[i] != 0) {
                throw std::runtime_error("for now, data dims need to be a multiple of block size");
             }
             n_blocks[i] = dat_dims[i]/block_size[i];
          }
          std::vector<size_t> sampled_block(block_size.size());
          std::vector<size_t> strides;
          size_t idx  = sample_seed;
          compat::exclusive_scan(
                  n_blocks.begin(),
                  n_blocks.end(),
                  std::back_inserter(strides),
                  size_t{1},
                  compat::multiplies<>{});
          const size_t l = dat_dims.size()-1;
          for (size_t i = 0; i < dat_dims.size(); ++i) {
            sampled_block[l-i] = idx / strides[l-i];
            idx %= strides[l-i];
          }

          using pointer = typename std::iterator_traits<decltype(src)>::pointer;
          pointer dst = static_cast<pointer>(sample.data());

          if(block_size.size() == 1) {
            indexer<1> src_idx{dat_dims[0]};
            indexer<1> dst_idx{block_size[0]};
            size_t base0 = block_size[0]*sampled_block[0];
            for (size_t i = 0; i < block_size[0]; ++i) {
              dst[dst_idx(i)] = src[src_idx(base0+i)];
            }
          }
          else if (block_size.size() == 2) {
            indexer<2> src_idx(dat_dims.begin(), dat_dims.end());
            indexer<2> dst_idx(block_size.begin(), block_size.end());
            size_t base0 = block_size[0]*sampled_block[0];
            size_t base1 = block_size[1]*sampled_block[1];
            for (size_t i = 0; i < block_size[0]; ++i) {
            for (size_t j = 0; j < block_size[1]; ++j) {
              dst[dst_idx(i,j)] = src[src_idx(base0+i, base1+j)];
            }}
          }
          else if (block_size.size() == 3) {
            indexer<3> src_idx(dat_dims.begin(), dat_dims.end());
            indexer<3> dst_idx(block_size.begin(), block_size.end());
            size_t base0 = block_size[0]*sampled_block[0];
            size_t base1 = block_size[1]*sampled_block[1];
            size_t base2 = block_size[2]*sampled_block[2];
            for (size_t i = 0; i < block_size[0]; ++i) {
            for (size_t j = 0; j < block_size[1]; ++j) {
            for (size_t k = 0; k < block_size[2]; ++k) {
              dst[dst_idx(i,j,k)] = src[src_idx(base0+i, base1+j, base2+k)];
            }}}
          }
          else if (block_size.size() == 4) {
            indexer<4> src_idx(dat_dims.begin(), dat_dims.end());
            indexer<4> dst_idx(block_size.begin(), block_size.end());
            size_t base0 = block_size[0]*sampled_block[0];
            size_t base1 = block_size[1]*sampled_block[1];
            size_t base2 = block_size[2]*sampled_block[2];
            size_t base3 = block_size[3]*sampled_block[3];
            for (size_t i = 0; i < block_size[0]; ++i) {
            for (size_t j = 0; j < block_size[1]; ++j) {
            for (size_t k = 0; k < block_size[2]; ++k) {
            for (size_t w = 0; w < block_size[3]; ++w) {
              dst[dst_idx(i,j,k,w)] = src[src_idx(base0+i, base1+j, base2+k,base3+w)];
            }}}}
          }
          else {
            throw std::runtime_error("unsupported size");
          }

          return sample;
      });
    }
    void set_n() {
      pressio_options metadata = loader->load_metadata(0);
      pressio_data dims;
      metadata.get(loader->get_name(), "loader:dims", &dims);
      std::vector<size_t> data_dims = dims.to_vector<size_t>();
      *N = 1;
      for (size_t i = 0; i < std::min(data_dims.size(), block_size.size()); ++i) {
          *N *= (size_t)std::ceil(data_dims[i]/(double)block_size[i]);
      }
    }

    compat::optional<size_t> N;
    std::vector<size_t> block_size;
    std::string loader_id = "io_loader";
    pressio_dataset_loader loader = dataset_loader_plugins().build(loader_id);
  };

  pressio_register block_slicer_loader_register(dataset_loader_plugins(), "block_slicer", []{ return compat::make_unique<block_slicer_loader>(); });
}}
