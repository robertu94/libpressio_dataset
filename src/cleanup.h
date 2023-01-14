#ifndef LIBPRESSIO_DATASET_CLEANUP_H_76YDNZF4
#define LIBPRESSIO_DATASET_CLEANUP_H_76YDNZF4
#include <functional>
#include <std_compat/utility.h>
namespace libpressio_dataset {

/**
 * this class is a standard c++ idiom for closing resources
 * it calls the function passed in during the destructor.
 */
class cleanup {
  public:
    cleanup() noexcept: cleanup_fn([]{}), do_cleanup(false) {}

    template <class Function>
    cleanup(Function f) noexcept: cleanup_fn(std::forward<Function>(f)), do_cleanup(true) {}
    cleanup(cleanup&& rhs) noexcept: cleanup_fn(std::move(rhs.cleanup_fn)), do_cleanup(compat::exchange(rhs.do_cleanup, false)) {}
    cleanup(cleanup const&)=delete;
    cleanup& operator=(cleanup const&)=delete;
    cleanup& operator=(cleanup && rhs) noexcept { 
      if(&rhs == this) return *this;
      do_cleanup = compat::exchange(rhs.do_cleanup, false);
      cleanup_fn = std::move(rhs.cleanup_fn);
      return *this;
    }
    ~cleanup() { if(do_cleanup) cleanup_fn(); }

  private:
    std::function<void()> cleanup_fn;
    bool do_cleanup;
};
template<class Function>
cleanup make_cleanup(Function&& f) {
  return cleanup(std::forward<Function>(f));
}
}
#endif /* end of include guard: CLEANUP_H_76YDNZF4 */
