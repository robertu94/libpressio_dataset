#include <libpressio_dataset_ext/loader.h>
#include <std_compat/memory.h>
#include <regex>
#include <sstream>
#include <cleanup.h>
#include <H5Opublic.h>
#include <H5Fpublic.h>
#include <H5Ppublic.h>
#include <H5Tpublic.h>
#include <H5Spublic.h>

namespace libpressio_dataset { namespace hdf5_loader_ns {

  extern "C" herr_t libpressio_dataset_loader_iterate_hdf5 (hid_t obj, const char *name, const H5O_info_t *info, void *op_data);

  compat::optional<pressio_dtype> h5t_to_pressio(hid_t h5type) {
    if(H5Tequal(h5type, H5T_NATIVE_INT8) > 0) return pressio_int8_dtype;
    if(H5Tequal(h5type, H5T_NATIVE_INT16) > 0) return pressio_int16_dtype;
    if(H5Tequal(h5type, H5T_NATIVE_INT32) > 0) return pressio_int32_dtype;
    if(H5Tequal(h5type, H5T_NATIVE_INT64) > 0) return pressio_int64_dtype;
    if(H5Tequal(h5type, H5T_NATIVE_UINT8) > 0) return pressio_uint8_dtype;
    if(H5Tequal(h5type, H5T_NATIVE_UINT16) > 0) return pressio_uint16_dtype;
    if(H5Tequal(h5type, H5T_NATIVE_UINT32) > 0) return pressio_uint32_dtype;
    if(H5Tequal(h5type, H5T_NATIVE_UINT64) > 0) return pressio_uint64_dtype;
    if(H5Tequal(h5type, H5T_NATIVE_FLOAT) > 0) return pressio_float_dtype;
    if(H5Tequal(h5type, H5T_NATIVE_HBOOL) > 0) return pressio_bool_dtype;
    if(H5Tequal(h5type, H5T_NATIVE_DOUBLE) > 0) return pressio_double_dtype;
    return compat::optional<pressio_dtype>{};
  }

  struct hdf5_loader: public dataset_loader_base {

    void scan() {
      if(!files){
          files = std::vector<std::string>();
          H5open();
          hid_t fid = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
          H5Ovisit(fid, H5_INDEX_NAME, H5_ITER_NATIVE, libpressio_dataset_loader_iterate_hdf5, this, H5O_INFO_BASIC);
          H5Fclose(fid);
      }
    }

    size_t num_datasets_impl() override {
      scan();
      return files->size();
    }

    int set_options_impl(pressio_options const& options) override {
      std::string new_filename = filename;
      if(get(options, "io:path", &new_filename) == pressio_options_key_set && new_filename != filename) {
        filename = std::move(new_filename);
        files.reset();
      }

      std::string new_regex = pattern;
      if(get(options, "hdf5_datasets:regex", &new_regex) == pressio_options_key_set && new_regex != pattern) {
        pattern = std::move(new_regex);
        files.reset();
      }
      get(options, "hdf5_datasets:groups", &groups);

      return 0;
    }

    pressio_options get_options_impl() const override {
      pressio_options options;
      set(options, "io:path", filename);
      set(options, "hdf5_datasets:regex", pattern);
      set(options, "hdf5_datasets:groups", groups);
      set_type(options, "hdf5_datasets:rescan", pressio_option_bool_type);
      return options;
    }

    pressio_options get_documentation_impl() const override {
      pressio_options options;
      set(options, "io:path", "path to load data from");
      set(options, "hdf5_datasets:regex", "if this regex matches, load this dataset");
      set(options, "hdf5_datasets:groups", "names for match expresison in the regex");
      set(options, "hdf5_datasets:rescan", "force a rescan if set");
      return options;
    }
    
    pressio_data load_data_impl(size_t n) override {
      hid_t fid = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
      if(fid < 0) {
          throw std::runtime_error("failed to open file");
      }
      auto cleanup_fid = make_cleanup([fid]{ H5Fclose(fid);});

      hid_t did = H5Dopen2(fid, files->at(n).c_str(), H5P_DEFAULT);
      if(did < 0) {
          throw std::runtime_error("failed to open dataset " + files->at(n));
      }
      auto cleanup_did = make_cleanup([did]{ H5Dclose(did);});
      hid_t sid = H5Dget_space(did);
      if(sid < 0) {
          throw std::runtime_error("failed to get space " + files->at(n));
      }
      auto cleanup_sid = make_cleanup([sid]{ H5Sclose(sid);});
      hid_t tid = H5Dget_type(did);
      if(tid < 0) {
          throw std::runtime_error("failed to get type " + files->at(n));
      }
      auto cleanup_tid = make_cleanup([tid]{ H5Tclose(tid);});

      const int ndims = H5Sget_simple_extent_ndims(sid);
      std::vector<hsize_t> hdims(ndims, 0);
      H5Sget_simple_extent_dims(sid, hdims.data(), nullptr);
      std::vector<uint64_t> dims(hdims.begin(), hdims.end());

      auto dtype = h5t_to_pressio(tid);
      pressio_data ret(pressio_data::owning(*dtype, dims));

      H5Dread(did, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, ret.data());
      return ret;
    }

    pressio_options load_metadata_impl(size_t n) override {
      scan();
      pressio_options metadata;

      hid_t fid = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
      if(fid < 0) {
          throw std::runtime_error("failed to open file");
      }
      auto cleanup_fid = make_cleanup([fid]{ H5Fclose(fid);});
      hid_t did = H5Dopen2(fid, files->at(n).c_str(), H5P_DEFAULT);
      if(did < 0) {
          throw std::runtime_error("failed to open dataset " + files->at(n));
      }
      auto cleanup_did = make_cleanup([did]{ H5Dclose(did);});
      hid_t sid = H5Dget_space(did);
      if(sid < 0) {
          throw std::runtime_error("failed to get space " + files->at(n));
      }
      auto cleanup_sid = make_cleanup([sid]{ H5Sclose(sid);});
      hid_t tid = H5Dget_type(did);
      if(tid < 0) {
          throw std::runtime_error("failed to get type " + files->at(n));
      }
      auto cleanup_tid = make_cleanup([tid]{ H5Tclose(tid);});

      const int ndims = H5Sget_simple_extent_ndims(sid);
      std::vector<hsize_t> hdims(ndims, 0);
      H5Sget_simple_extent_dims(sid, hdims.data(), nullptr);
      std::vector<uint64_t> dims(hdims.begin(), hdims.end());

      auto dtype = h5t_to_pressio(tid);
      if(!dtype) throw std::runtime_error("failed to convert type");

      set(metadata, "loader:dims", pressio_data(dims.begin(), dims.end()));
      set(metadata, "loader:dtype", *dtype);
      if(!groups.empty()) {
        std::regex regex(pattern);
        std::smatch match;
        if(std::regex_match(files->at(n), match, regex)) {
          for (size_t i = 1; i < std::min(match.size(), groups.size()+1); ++i) {
            std::ssub_match s = match[i];
            set(metadata, "hdf5_datasets:group:" + groups[i-1], s.str());
          }
        }
      }
      return metadata;
    }

    std::unique_ptr<dataset_loader> clone() override {
            return std::make_unique<hdf5_loader>(*this);
    }

    const char* prefix() const override {
      return "hdf5_datasets";
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

    std::string filename;
    std::string pattern = ".+";
    std::optional<std::vector<std::string>> files;
    std::vector<std::string> groups;
    friend herr_t libpressio_dataset_loader_iterate_hdf5(hid_t, const char*, const H5O_info_t, void*);
  };

  extern "C" herr_t libpressio_dataset_loader_iterate_hdf5 (hid_t obj, const char *name, const H5O_info_t *info, void *op_data) {
      hdf5_loader* ptr = static_cast<hdf5_loader*>(op_data);
      if(info->type == H5O_TYPE_DATASET) {
          hid_t did = H5Dopen(obj, name, H5P_DEFAULT);
          hid_t tid = H5Dget_type(did);
          if(tid < 0) {
              throw std::runtime_error("unexpected type");
          }
          H5T_class_t cid = H5Tget_class(tid);
          std::regex regex(ptr->pattern);
          std::smatch match;
          if(cid == H5T_INTEGER || cid == H5T_FLOAT) {
                std::string str(name);
                if(std::regex_match(str, match, regex)) {
                  ptr->files->emplace_back(name);
                }
          }
          H5Tclose(tid);
      }
      
      return 0;
  };


  pressio_register hdf5_loader_register(dataset_loader_plugins(), "hdf5_datasets", []{ return compat::make_unique<hdf5_loader>(); });
}}
