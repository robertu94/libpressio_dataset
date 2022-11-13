#include "gtest/gtest.h"
#include <libpressio_dataset_ext/loader.h>
#include <libpressio_ext/cpp/libpressio.h>
#include <string>
#include <filesystem>
#include <chrono>

using namespace std::string_literals;
using namespace libpressio_dataset;
namespace fs = std::filesystem;
fs::path datadir = LIBPRESSIO_DATASET_TEST_DATA_DIR;

TEST(libpressio_dataset, io_loader) {
  pressio_dataset_loader loader = dataset_loader_plugins().build("io_loader");
  ASSERT_TRUE(loader);
  loader->set_options({
      {"io_loader:dims", pressio_data{500,500}},
      {"io_loader:dtype", pressio_float_dtype},
      {"io_loader:use_template", true},
      {"io_loader:plugin", "posix"s},
      {"io:path", (datadir/"s0-CLOUDf48.bin.f32").string()},
  });
  ASSERT_EQ(loader->num_datasets(), 1);

  pressio_data p_dims;
  pressio_dtype dtype;
  auto metadata = loader->load_metadata(0);
  ASSERT_EQ(metadata.get("loader:dims", &p_dims), pressio_options_key_set);
  ASSERT_EQ(metadata.get("loader:dtype", &dtype), pressio_options_key_set);
  std::vector<size_t> expected_dims{500,500};
  ASSERT_EQ(p_dims.to_vector<size_t>(), expected_dims);
  ASSERT_EQ(dtype, pressio_float_dtype);
}

TEST(libpressio_dataset, folder_loader) {
  pressio_dataset_loader loader = dataset_loader_plugins().build("folder");
  ASSERT_TRUE(loader);
  loader->set_options({
      {"io_loader:dims", pressio_data{500,500}},
      {"io_loader:dtype", pressio_float_dtype},
      {"io_loader:use_template", true},
      {"io_loader:plugin", "posix"s},
      {"folder:regex", "(?:[^/]*/)+s(\\d+)-([A-Z]+)f(\\d+).bin.f32"s},
      {"folder:base_dir", (datadir.string())},
      {"folder:groups", std::vector<std::string>{"slice", "field", "timestep"}},
  });
  ASSERT_EQ(loader->num_datasets(), 26);

  //directory iterations order is undefined query for the path order
  auto options = loader->get_options();
  std::vector<std::string> found_paths;
  ASSERT_EQ(options.get("folder:paths", &found_paths), pressio_options_key_set);
  auto it = std::find_if(found_paths.begin(), found_paths.end(), [](auto const& item){
      return item.find("s0-CLOUDf48.bin.f32");
  });
  ASSERT_NE(it, found_paths.end());

  auto metadata = loader->load_metadata(std::distance(found_paths.begin(), it));
  std::string slice, field, timestep;
  ASSERT_EQ(metadata.get("folder:group:slice", &slice), pressio_options_key_set);
  ASSERT_EQ(metadata.get("folder:group:field", &field), pressio_options_key_set);
  ASSERT_EQ(metadata.get("folder:group:timestep", &timestep), pressio_options_key_set);
  ASSERT_EQ(slice, "0");
  ASSERT_EQ(field, "CLOUD");
  ASSERT_EQ(timestep, "48");
}


TEST(libpressio_dataset, block_sampler) {
  pressio_dataset_loader loader = dataset_loader_plugins().build("block_sampler");
  ASSERT_TRUE(loader);
  loader->set_options({
      {"block_sampler:block_size", pressio_data{100,100}},
      {"block_sampler:n", uint64_t{4}},
      {"block_sampler:loader", "io_loader"s},
      {"io_loader:dims", pressio_data{500,500}},
      {"io_loader:dtype", pressio_float_dtype},
      {"io_loader:use_template", true},
      {"io_loader:plugin", "posix"s},
      {"io:path", (datadir/"s0-CLOUDf48.bin.f32").string()},
  });
  ASSERT_EQ(loader->num_datasets(), 4);

  pressio_data p_dims;
  pressio_dtype dtype;
  auto metadata = loader->load_metadata(0);
  ASSERT_EQ(metadata.get("loader:dims", &p_dims), pressio_options_key_set);
  ASSERT_EQ(metadata.get("loader:dtype", &dtype), pressio_options_key_set);
  std::vector<size_t> expected_dims{100,100};
  ASSERT_EQ(p_dims.to_vector<size_t>(), expected_dims);
  ASSERT_EQ(dtype, pressio_float_dtype);

  std::vector<pressio_data> samples = loader->load_all_data();
}

TEST(libpressio_dataset, block_sampler2) {
  pressio_dataset_loader loader = dataset_loader_plugins().build("block_sampler");
  ASSERT_TRUE(loader);
  loader->set_options({
      {"block_sampler:block_size", pressio_data{100,100}},
      {"block_sampler:n", uint64_t{4}},
      {"block_sampler:loader", "folder"s},
      {"io_loader:dims", pressio_data{500,500}},
      {"io_loader:dtype", pressio_float_dtype},
      {"io_loader:use_template", true},
      {"io_loader:plugin", "posix"s},
      {"folder:regex", "(?:[^/]*/)+s(\\d+)-([A-Z]+)f(\\d+).bin.f32"s},
      {"folder:base_dir", (datadir.string())},
      {"folder:groups", std::vector<std::string>{"slice", "field", "timestep"}},
  });
  ASSERT_EQ(loader->num_datasets(), 4*26);

  pressio_data p_dims;
  pressio_dtype dtype;
  auto metadata = loader->load_metadata(0);
  ASSERT_EQ(metadata.get("loader:dims", &p_dims), pressio_options_key_set);
  ASSERT_EQ(metadata.get("loader:dtype", &dtype), pressio_options_key_set);
  std::vector<size_t> expected_dims{100,100};
  ASSERT_EQ(p_dims.to_vector<size_t>(), expected_dims);
  ASSERT_EQ(dtype, pressio_float_dtype);

  std::vector<pressio_data> samples = loader->load_all_data();
}

TEST(libpressio_dataset, block_sampler_named) {
  pressio_dataset_loader loader = dataset_loader_plugins().build("block_sampler");
  ASSERT_TRUE(loader);
  loader->set_options({
      {"block_sampler:block_size", pressio_data{100,100}},
      {"block_sampler:n", uint64_t{4}},
      {"block_sampler:loader", "folder"s},
      {"io_loader:dims", pressio_data{500,500}},
      {"io_loader:dtype", pressio_float_dtype},
      {"io_loader:use_template", true},
      {"io_loader:plugin", "posix"s},
      {"folder:regex", "(?:[^/]*/)+s(\\d+)-([A-Z]+)f(\\d+).bin.f32"s},
      {"folder:base_dir", (datadir.string())},
      {"folder:groups", std::vector<std::string>{"slice", "field", "timestep"}},
  });
  loader->set_name("pressio");
  ASSERT_EQ(loader->num_datasets(), 4*26);

  pressio_data p_dims;
  pressio_dtype dtype;
  auto metadata = loader->load_metadata(0);
  ASSERT_EQ(metadata.get("loader:dims", &p_dims), pressio_options_key_does_not_exist);
  ASSERT_EQ(metadata.get("loader:dtype", &dtype), pressio_options_key_does_not_exist);
  ASSERT_EQ(metadata.get("/pressio:loader:dims", &p_dims), pressio_options_key_set);
  ASSERT_EQ(metadata.get("/pressio:loader:dtype", &dtype), pressio_options_key_set);
  std::vector<size_t> expected_dims{100,100};
  ASSERT_EQ(p_dims.to_vector<size_t>(), expected_dims);
  ASSERT_EQ(dtype, pressio_float_dtype);

  std::vector<pressio_data> samples = loader->load_all_data();
}

TEST(libpressio_dataset, cache) {
  uint64_t N = 30;
  auto with_cache_begin = std::chrono::steady_clock::now();
  {
  pressio_dataset_loader loader = dataset_loader_plugins().build("block_sampler");
  ASSERT_TRUE(loader);
  loader->set_options({
      {"block_sampler:block_size", pressio_data{100,100}},
      {"block_sampler:n", N},
      {"block_sampler:loader", "cache"s},
      {"cache:loader", "folder"s},
      {"io_loader:dims", pressio_data{500,500}},
      {"io_loader:dtype", pressio_float_dtype},
      {"io_loader:use_template", true},
      {"io_loader:plugin", "posix"s},
      {"folder:regex", "(?:[^/]*/)+s(\\d+)-([A-Z]+)f(\\d+).bin.f32"s},
      {"folder:base_dir", (datadir.string())},
      {"folder:groups", std::vector<std::string>{"slice", "field", "timestep"}},
  });
  ASSERT_EQ(loader->num_datasets(), N*26);

  pressio_data p_dims;
  pressio_dtype dtype;
  auto metadata = loader->load_metadata(0);
  ASSERT_EQ(metadata.get("loader:dims", &p_dims), pressio_options_key_set);
  ASSERT_EQ(metadata.get("loader:dtype", &dtype), pressio_options_key_set);
  std::vector<size_t> expected_dims{100,100};
  ASSERT_EQ(p_dims.to_vector<size_t>(), expected_dims);
  ASSERT_EQ(dtype, pressio_float_dtype);

  std::vector<pressio_data> samples = loader->load_all_data();
  }
  auto with_cache_end = std::chrono::steady_clock::now();
  {
  pressio_dataset_loader loader = dataset_loader_plugins().build("block_sampler");
  ASSERT_TRUE(loader);
  loader->set_options({
      {"block_sampler:block_size", pressio_data{100,100}},
      {"block_sampler:n", N},
      {"block_sampler:loader", "folder"s},
      {"io_loader:dims", pressio_data{500,500}},
      {"io_loader:dtype", pressio_float_dtype},
      {"io_loader:use_template", true},
      {"io_loader:plugin", "posix"s},
      {"folder:regex", "(?:[^/]*/)+s(\\d+)-([A-Z]+)f(\\d+).bin.f32"s},
      {"folder:base_dir", (datadir.string())},
      {"folder:groups", std::vector<std::string>{"slice", "field", "timestep"}},
  });
  ASSERT_EQ(loader->num_datasets(), N*26);

  pressio_data p_dims;
  pressio_dtype dtype;
  auto metadata = loader->load_metadata(0);
  ASSERT_EQ(metadata.get("loader:dims", &p_dims), pressio_options_key_set);
  ASSERT_EQ(metadata.get("loader:dtype", &dtype), pressio_options_key_set);
  std::vector<size_t> expected_dims{100,100};
  ASSERT_EQ(p_dims.to_vector<size_t>(), expected_dims);
  ASSERT_EQ(dtype, pressio_float_dtype);

  std::vector<pressio_data> samples = loader->load_all_data();
  }
  auto without_cache_end = std::chrono::steady_clock::now();

  /*
  std::cout << "without cache" << 
    std::chrono::duration_cast<std::chrono::milliseconds>(without_cache_end-with_cache_end).count() << std::endl;
  std::cout << "with cache" << 
    std::chrono::duration_cast<std::chrono::milliseconds>(with_cache_end-with_cache_begin).count() << std::endl;
  */
}
