#include <libpressio_dataset_version.h>
#include <stddef.h>

#ifndef LIBPRESSIO_DATASET_H_BTENLH3D
#define LIBPRESSIO_DATASET_H_BTENLH3D

#ifdef __cplusplus
extern "C" {
#endif

struct pressio_data;
struct pressio_options;
struct pressio_dataset_loader;
struct pressio;

/**
 * loads a pressio_data loader
 *
 * \param[in] library the library to use for loading
 * \param[in] id the id of the loader to load
 * \returns the newly constructed loader or nullptr on error
 */
struct pressio_dataset_loader* pressio_get_dataset_loader(struct pressio* library, const char* id);

/**
 * frees a libpressio_dataset_loader
 */
void pressio_dataset_loader_free(struct pressio_dataset_loader*);


/**
 * get the options for a libpressio_dataset_loader
 *
 * \param[in] loader the loader to get options for
 * \returns the options for the loader
 */
struct pressio_options* pressio_dataset_loader_get_options(struct pressio_dataset_loader const*);

/**
 * get the configuration for a libpressio_dataset_loader
 *
 * \param[in] loader the loader to get options for
 * \returns the configuration for the loader
 */
struct pressio_options* pressio_dataset_loader_get_configuration(struct pressio_dataset_loader const*);

/**
 * get the documetation for a libpressio_dataset_loader
 *
 * \param[in] loader the loader to get documentation for
 * \returns the configuration for the loader
 */
struct pressio_options* pressio_dataset_loader_get_documentation(struct pressio_dataset_loader const*);

/**
 * set the options for a libpressio_dataset_loader
 *
 * \param[in] loader the loader to apply the options to
 * \param[in] options the options for the loader
 * \returns 0 on success, <0 on warning, >0 on error
 */
int pressio_dataset_loader_set_options(struct pressio_dataset_loader* loader, struct pressio_options const* options);

/**
 * check the options for a libpressio_dataset_loader
 *
 * \param[in] loader the loader to apply the options to
 * \param[in] options the options for the loader
 * \returns 0 on success, <0 on warning, >0 on error
 */
int pressio_dataset_loader_check_options(struct pressio_dataset_loader* loader, struct pressio_options const* options);

/**
 * Assign a new name to a loader.  Names are used to prefix options in meta-loaders.
 *
 * sub-loaders will be renamed either by the of the sub-loaders prefix
 * or by the $prefix:name configuration option
 *
 * i.e. for some new_name and a loader with prefix foo and subloaders
 * with prefixs "abc", "def", "ghi" respectively
 *
 * - if foo:names = ['one', 'two', 'three'], then the names will be `$new_name/one, $new_name/two $new_name/three
 * - otherwise the names will be $new_name/abc, $new_name/def, $new_name/ghi
 *
 * \param[in] loader the loader to get the name of
 * \param[in] new_name the name to set
 */
void pressio_dataset_loader_set_name(struct pressio_dataset_loader* loader, const char* new_name);

/**
 * Get the name of a loader
 * \param[in] loader the compressor to get the name of
 * \returns a string with the loader name. The string becomes invalid if
 *          the name is set_name is called.
 */
const char* pressio_dataset_loader_get_name(struct pressio_dataset_loader const* loader);

/**
 * Returns the name this loader uses its keys
 *
 * \param[in] loader the loader to get the prefix for
 * \returns the prefix for this loader
 */
const char* pressio_dataset_loader_get_prefix(const struct pressio_dataset_loader* loader);

/**
 * Clones a loader and its configuration
 *
 * \param[in] loader the loader to clone. It will not be modified
 *                       except to modify a reference count as needed.
 * \returns a pointer to a new loader plugin reference which is equivalent
 *          to the loader cloned.  It the loader is not thread safe, it may
 *          return a new reference to the same object.
 *                
 */
struct pressio_dataset_loader* pressio_dataset_loader_clone(struct pressio_dataset_loader* loader);

/*!
 * Get the version and feature information.  The version string may include more information than major/minor/patch provide.
 * \param[in] dataset_loader the dataset_loader to query
 * \returns a implementation specific version string
 */
const char* pressio_dataset_loader_version(struct pressio_dataset_loader const* dataset_loader);
/*!
 * \param[in] dataset_loader the dataset_loader to query
 * \returns the major version number or 0 if there is none
 */
int pressio_dataset_loader_major_version(struct pressio_dataset_loader const* dataset_loader);
/*!
 * \param[in] dataset_loader the dataset_loader to query
 * \returns the major version number or 0 if there is none
 */
int pressio_dataset_loader_minor_version(struct pressio_dataset_loader const* dataset_loader);
/*!
 * \param[in] dataset_loader the dataset_loader to query
 * \returns the major version number or 0 if there is none
 */
int pressio_dataset_loader_patch_version(struct pressio_dataset_loader const* dataset_loader);

/**
 * \param[in] dataset_loader the dataset_loader to query
 * \returns last error code for the dataset_loader
 */
int pressio_dataset_loader_error_code(struct pressio_dataset_loader const* dataset_loader);

/**
 * \param[in] dataset_loader the dataset_loader to query
 * \returns last error message for the dataset_loader
 */
const char* pressio_dataset_loader_error_msg(struct pressio_dataset_loader* dataset_loader);

int pressio_datset_loader_num_datasets(struct pressio_dataset_loader* dataset_loader, size_t* n);

int pressio_dataset_loader_load_all_metadata(struct pressio_dataset_loader* dataset_loader, size_t* n, struct pressio_options***);

int pressio_dataset_loader_load_all_data(struct pressio_dataset_loader* dataset_loader, size_t* n, struct pressio_data***);

int pressio_dataset_loader_load_metadata(struct pressio_dataset_loader* dataset_loader, size_t n, struct pressio_options**);

int pressio_dataset_loader_load_data(struct pressio_dataset_loader* dataset_loader, size_t n, struct pressio_data**);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: LIBPRESSIO_DATASET_H_BTENLH3D */
