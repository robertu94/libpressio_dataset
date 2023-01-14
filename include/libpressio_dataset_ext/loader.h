#ifndef LIBPRESSIO_DATASET_LOADER_H_5BIKNPI7
#define LIBPRESSIO_DATASET_LOADER_H_5BIKNPI7
#include <libpressio_ext/cpp/configurable.h>
#include <libpressio_ext/cpp/errorable.h>
#include <libpressio_ext/cpp/versionable.h>
#include <libpressio_ext/cpp/pressio.h>

namespace libpressio_dataset {
class dataset_loader: public pressio_configurable, public pressio_errorable, public pressio_versionable {
  public:
  virtual size_t num_datasets()=0;
  virtual pressio_data load_data(size_t)=0;
  virtual pressio_options load_metadata(size_t)=0;

  virtual std::vector<pressio_options> load_all_metadata() {
    std::vector<pressio_options> ret;
    size_t N = num_datasets();
    for (size_t i = 0; i < N; ++i) {
      ret.emplace_back(load_metadata(i));
    }
    return ret;
  }

  virtual std::vector<pressio_data> load_all_data() {
    std::vector<pressio_data> ret;
    size_t N = num_datasets();
    for (size_t i = 0; i < N; ++i) {
      ret.emplace_back(load_data(i));
    }
    return ret;
  }

  virtual std::unique_ptr<dataset_loader> clone() = 0;

  using pressio_errorable::set_error;
  private:
};

class dataset_loader_base: public dataset_loader {
    size_t num_datasets() final {
      return num_datasets_impl();
    }

    int set_options(pressio_options const& options) final {
      return set_options_impl(options);
    }

    pressio_options get_options() const final {
      return get_options_impl();
    }

    pressio_options get_documentation() const final {
      return get_documentation_impl();
    }
    
    pressio_data load_data(size_t n) final {
      return load_data_impl(n);
    }

    pressio_options load_metadata(size_t n) final {
      return load_metadata_impl(n);
    }

    virtual size_t num_datasets_impl()=0;

    virtual int set_options_impl(pressio_options const&) {
      return 0;
    }

    virtual pressio_options get_options_impl() const {
      return {};
    }

    virtual pressio_options get_documentation_impl() const {
      return {};
    }

    void set_name(std::string const& name) final {
      dataset_loader::set_name(name);
    }
    
    virtual pressio_data load_data_impl(size_t n) =0;

    virtual pressio_options load_metadata_impl(size_t n) =0;
};


struct pressio_dataset_loader {
  pressio_dataset_loader(std::unique_ptr<dataset_loader>&& ptr): ptr(std::move(ptr)) {}
  pressio_dataset_loader(pressio_dataset_loader&& ptr)noexcept =default;
  pressio_dataset_loader()=default;
  pressio_dataset_loader(pressio_dataset_loader const& lhs): ptr(lhs.ptr->clone()) {}
  pressio_dataset_loader& operator=(pressio_dataset_loader&& ptr)noexcept =default;
  pressio_dataset_loader& operator=(pressio_dataset_loader const& lhs) { 
    if(&lhs  == this) return *this;
    ptr = lhs.ptr->clone();
    return *this;
  }
  operator bool() const {
    return bool(ptr);
  }
  dataset_loader& operator*() const noexcept {
    return *ptr;
  }
  dataset_loader* operator->() const noexcept {
    return ptr.get();
  }
  private:
  std::unique_ptr<dataset_loader> ptr;
};

pressio_registry<std::unique_ptr<dataset_loader>>& dataset_loader_plugins();
}


#endif /* end of include guard: LIBPRESSIO_DATASET_LOADER_H_5BIKNPI7 */
