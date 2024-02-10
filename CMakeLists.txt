## template for a CMake C++ Library
cmake_minimum_required(VERSION 3.14)
project(libpressio_dataset VERSION "0.0.8" LANGUAGES CXX)

#correct was to set a default build type
# https://blog.kitware.com/cmake-and-the-default-build-type/
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type was set. Setting build type to ${default_build_type}.")
  set(CMAKE_BUILD_TYPE ${default_build_type} CACHE 
    STRING "Choose the type to build" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")

include(CTest)
include(GNUInstallDirs)
include(CheckCXXCompilerFlag)

#compiler flags and standard conformance checks
check_cxx_compiler_flag("-fno-omit-frame-pointer" HAVE_NO_OMIT_FRAME_POINTER)
set(NO_OMIT_FRAME_POINTER_FLAG "")
if(HAVE_NO_OMIT_FRAME_POINTER)
  set(NO_OMIT_FRAME_POINTER_FLAG "-fno-omit-frame-pointer")
endif()

if(NOT USE_SANITIZER)
  set(USE_SANITIZER "" CACHE STRING "use a build sanitizer" FORCE)
endif()
set_property(CACHE USE_SANITIZER PROPERTY STRINGS "" "Address" "Thread" "Undefined" "Leak" "Memory")
if (USE_SANITIZER STREQUAL "Address")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
elseif (USE_SANITIZER STREQUAL "Thread")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
elseif (USE_SANITIZER STREQUAL "Undefined")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
elseif (USE_SANITIZER STREQUAL "Leak")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=leak")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")
elseif (USE_SANITIZER STREQUAL "Memory")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=memory")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory")
endif()


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_SHARED_LIBS "build libpressio_dataset as a shared library" ON)

find_package(LibPressio REQUIRED)
find_package(std_compat REQUIRED)

add_library(libpressio_dataset
  ./src/libpressio_dataset.cc
  ./src/plugins/dataset_loader/loader_base.cc
  ./src/plugins/dataset_loader/io_loader.cc
  ./src/plugins/dataset_loader/folder_loader.cc
  ./src/plugins/dataset_loader/block_sampler.cc
  ./src/plugins/dataset_loader/block_slicer.cc
  ./src/plugins/dataset_loader/cache_loader.cc
  ./src/plugins/dataset_loader/random_sampler.cc
  ./src/plugins/dataset_loader/from_data.cc
  ./src/plugins/dataset_loader/pressio.cc
  ./include/libpressio_dataset.h
  ./include/libpressio_dataset_ext/loader.h
  )
target_compile_features(libpressio_dataset PUBLIC cxx_std_17)
target_link_libraries(libpressio_dataset PUBLIC LibPressio::libpressio)
target_include_directories(libpressio_dataset
    PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> 
    )
target_include_directories(libpressio_dataset 
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/libpressio_dataset>
  )
target_compile_options(libpressio_dataset PRIVATE 
  $<$<CONFIG:Debug>: -Wall  -Wextra -Wpedantic>
  $<$<CONFIG:RelWithDebInfo>: ${NO_OMIT_FRAME_POINTER_FLAG}>
  )

option(LIBPRESSIO_DATASET_WERROR ON)
if(LIBPRESSIO_DATASET_WERROR)
  target_compile_options(libpressio_dataset PRIVATE 
    $<$<CONFIG:Debug>: -Werror>
    )
endif()


option(LIBPRESSIO_DATASET_HAS_HDF5 ON)
if(LIBPRESSIO_DATASET_HAS_HDF5)
    enable_language(C)
    find_package(HDF5 REQUIRED COMPONENTS C)
    target_sources(libpressio_dataset PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/dataset_loader/hdf5_loader.cc)
    target_link_libraries(libpressio_dataset PRIVATE ${HDF5_C_LIBRARIES})
    target_include_directories(libpressio_dataset PRIVATE ${HDF5_C_INCLUDE_DIRS})
    target_compile_definitions(libpressio_dataset PRIVATE ${HDF5_C_DEFINITIONS})
    if(${HDF5_IS_PARALLEL})
      find_package(MPI REQUIRED)
      target_link_libraries(libpressio_dataset PRIVATE MPI::MPI_CXX)
    endif()
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/libpressio_dataset_version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/libpressio_dataset_version.h
  )
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/libpressio_dataset.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/libpressio_dataset.pc
  @ONLY
  )

export(TARGETS libpressio_dataset NAMESPACE LibPressioDataset:: FILE libpressio_dataset.cmake)
install(TARGETS libpressio_dataset EXPORT LibPressioDatasetTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/LibPressioDatasetConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/LibPressioDatasetConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LibPressioDataset
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/LibPressioDatasetConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY AnyNewerVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/LibPressioDatasetConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/LibPressioDatasetConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LibPressioDataset
)
install(EXPORT LibPressioDatasetTargets NAMESPACE LibPressioDataset:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LibPressioDataset)
install(DIRECTORY include/ DESTINATION
  ${CMAKE_INSTALL_INCLUDEDIR}/libpressio_dataset)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/libpressio_dataset_version.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libpressio_dataset)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libpressio_dataset.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pkgconfig)


if(BUILD_TESTING)
  add_subdirectory(test)
endif()

option(USE_CLANG_TIDY "include clang-tidy warnings in the build log" OFF)
if(USE_CLANG_TIDY)
  find_program(CLANG_TIDY clang-tidy)
  set_target_properties(libpressio_dataset PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY}")
  set_target_properties(libpressio_dataset PROPERTIES C_CLANG_TIDY "${CLANG_TIDY}")
endif()

option(USE_INCLUDE_WHAT_YOU_USE "include include-what-you-use warnings in the build log" OFF)
if(USE_INCLUDE_WHAT_YOU_USE)
  find_program(INCLUDE_WHAT_YOU_USE include-what-you-use)
  set_target_properties(libpressio_dataset PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${INCLUDE_WHAT_YOU_USE}")
  set_target_properties(libpressio_dataset PROPERTIES C_INCLUDE_WHAT_YOU_USE "${INCLUDE_WHAT_YOU_USE}")
endif()


option(BUILD_DOCS "build the documentation" OFF)
if(BUILD_DOCS)
  find_package(Doxygen REQUIRED dot)
  set(DOXYGEN_MAN_LINKS YES)
  set(DOXYGEN_GENERATE_MAN YES)
  set(DOXYGEN_GENERATE_HTML YES)
  set(DOXYGEN_EXTRACT_LOCAL_METHODS YES)
  set(DOXYGEN_EXTRACT_STATIC YES)
  set(DOXYGEN_MACRO_EXPANSION YES)
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
  doxygen_add_docs(
    docs
    ${PROJECT_SOURCE_DIR}/README.md
    ${PROJECT_SOURCE_DIR}/include
    COMMENT "Generate Documenation"
  )
endif()
# vim: ft=cmake :
