#include <libpressio_dataset_ext/loader.h>
#include <std_compat/memory.h>
#include <sstream>
#include <regex>
#include <filesystem>
namespace libpressio_dataset { namespace folder_loader_ns {
  namespace fs = std::filesystem;
  struct folder_loader: public dataset_loader_base {

    size_t num_datasets_impl() override {
      if(!paths) scan();
      return paths->size();
    }

    int set_options_impl(pressio_options const& options) override {
      get_meta(options, "folder:plugin", dataset_loader_plugins(), loader_plugin_id, loader_plugin);
      bool new_recursive = recursive;
      if(get(options, "folder:recursive", &new_recursive) == pressio_options_key_set && new_recursive != recursive) {
        recursive = new_recursive;
        paths.reset();
      }
      std::string new_regex = rgx;
      if(get(options, "folder:regex", &new_regex) == pressio_options_key_set && new_regex != rgx) {
        rgx = std::move(new_regex);
        paths.reset();
      }
      std::string new_base_dir;
      if(get(options, "folder:base_dir", &new_base_dir) == pressio_options_key_set && new_base_dir != base_dir) {
        base_dir = new_base_dir;
        paths.reset();
      }

      //no need to reset here, this can't change the search results, just metadata
      get(options, "folder:groups", &groups);
      get(options, "folder:paths", &paths);

      //provide a way to force a re-scan
      bool tmp;
      if(get(options, "folder:rescan", &tmp)== pressio_options_key_set) {
        paths.reset();
      }
      return 0;
    }

    pressio_options get_options_impl() const override {
      pressio_options options;
      set_meta(options, "folder:plugin", loader_plugin_id, loader_plugin);
      set(options, "folder:recursive", recursive);
      set(options, "folder:regex", rgx);
      set(options, "folder:base_dir", base_dir);
      set(options, "folder:groups", groups);
      set(options, "folder:paths", paths);
      set_type(options, "folder:rescan", pressio_option_bool_type);
      return options;
    }

    pressio_options get_documentation_impl() const override {
      pressio_options options;
      set_meta_docs(options, "folder:plugin", "loader method to load datsets", loader_plugin);
      set(options, "folder:recursive", "search base_dir recursively");
      set(options, "folder:regex", "if this regex matches, load this path");
      set(options, "folder:base_dir", "base directory for the search");
      set(options, "folder:groups", "names for match expresison in the regex");
      set(options, "folder:paths", "list of paths to search");
      set(options, "folder:rescan", "force a rescan if set");
      return options;
    }
    
    pressio_data load_data_impl(size_t n) override {
      if(!paths) scan();
      pressio_options options;
      options.set(loader_plugin->get_name(), "io:path", paths->at(n));
      loader_plugin->set_options(options);
      return loader_plugin->load_data(n);
    }

    pressio_options load_metadata_impl(size_t n) override {
      if(!paths) scan();
      pressio_options options;
      options.set(loader_plugin->get_name(), "io:path", paths->at(n));
      loader_plugin->set_options(options);
      pressio_options metadata = loader_plugin->load_metadata(0);

      pressio_dtype dtype;
      pressio_data dims;
      metadata.get(loader_plugin->get_name(), "loader:dims", &dims);
      metadata.get(loader_plugin->get_name(), "loader:dtype", &dtype);
      set(metadata, "loader:dims", dims);
      set(metadata, "loader:dtype", dtype);

      if(!groups.empty()) {
        std::regex regex(rgx);
        std::smatch match;
        if(std::regex_match(paths->at(n), match, regex)) {
          for (size_t i = 1; i < std::min(match.size(), groups.size()+1); ++i) {
            std::ssub_match s = match[i];
            set(metadata, "folder:group:" + groups[i-1], s.str());
          }
        }
      }
      return metadata;
    }

    template <class It>
    void scan_impl(It dir_it) {
      std::regex regex(rgx);
      std::smatch match;
      for (auto const& i : dir_it) {
        if(!i.is_regular_file()) continue;
        auto const& path = i.path().string();
        if(std::regex_match(path, match, regex)) {
          paths->emplace_back(i.path().string());
        }
      }
    }
    void scan() {
      paths = std::vector<std::string>();
      if(recursive) {
        scan_impl(fs::recursive_directory_iterator(base_dir));
      } else {
        scan_impl(fs::directory_iterator(base_dir));
      }
    }

    void set_name_impl(std::string const& new_name) override {
      loader_plugin->set_name(new_name + "/" + loader_plugin->prefix());
    }

    std::unique_ptr<dataset_loader> clone() override {
      return std::make_unique<folder_loader>(*this);
    }

    const char* prefix() const override {
      return "folder";
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

    bool recursive = false;
    std::string rgx = ".+";
    std::string base_dir = ".";
    std::vector<std::string> groups;
    std::optional<std::vector<std::string>> paths;
    std::string loader_plugin_id = "io_loader";
    pressio_dataset_loader loader_plugin = dataset_loader_plugins().build("io_loader");
  };

  pressio_register folder_loader_register(dataset_loader_plugins(), "folder", []{ return compat::make_unique<folder_loader>(); });
}}
