/* Created on: Mar 11, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_IO_H_
# define MEDIAN_PATH_IO_H_

# include "median_path.h"

# include <string>

BEGIN_MP_NAMESPACE
  class median_skeleton;

  /**@namespace io
   *
   * @brief Load and save skeletons.
   *
   * This namespace contains functions for skeleton I/O. Here are the currently
   * handled file format:
   * - .moff The format used in Mesecina, the software of Balint Miklos on Scale Axis Transform
   * - .balls A file starting with a number of balls n, followed by n * 4 floating point numbers.
   * Those numbers are taken four by four to build median atoms. This format is useful to import
   * data from other software. The topology of the skeleton can then be built by reconstruction
   * algorithms from this library.
   * - .median The format used by this library.
   */
  namespace io {

    struct loader {
      virtual ~loader(){}
      virtual bool can_load_from( const std::string& filename ) = 0;
      virtual bool load( median_skeleton& skeleton, const std::string& filename ) = 0;
    };

    struct saver {
      virtual ~saver(){}
      virtual bool can_save_to( const std::string& filename ) = 0;
      virtual bool save( median_skeleton& skeleton, const std::string& filename ) = 0;
    };

    /**@brief Check if a skeleton file can be loaded.
     *
     * Check if there is a loader that can load a given file.
     * @param filename File name of the skeleton file.
     * @return True if a skeleton file can be loaded.
     */
    bool can_load_from( const std::string& filename );
    /**@brief Check if a skeleton can be saved to a specified file.
     *
     * Check if there is a saver than can save a skeleton to a given file.
     * @param filename File name of the skeleton file.
     * @return True if a skeleton can be saved to the skeleton file.
     */
    bool can_save_to( const std::string& filename );
    /**@brief Load a skeleton from a file.
     *
     * Load a skeleton from a given file.
     * @param skeleton The skeleton to load into.
     * @param filename The name of the file describing the skeleton to load.
     * @return Ture if the operation is successful.
     */
    bool load( median_skeleton& skeleton, const std::string& filename );
    /**@brief Save a skeleton to a file.
     *
     * Save a skeleton to a given file.
     * @param skeleton The skeleton to save.
     * @param filename The name of the file to save the skeleton to.
     * @return True if the operation is successful.
     */
    bool save( median_skeleton& skeleton, const std::string& filename );
    /**@brief Add a loader to load more skeleton file types.
     *
     * Add a skeleton file loader to the loaders list. This function
     * allows the user to handle skeleton files produced by external software.
     * @param loader A skeleton file loader.
     */
    void add_loader( loader* ldr );
    /**@brief Add a saver to save to more skeleton file types.
     *
     * Add a skeleton file saver to the savers list. This function
     * allows the user to export skeletons to other formats.
     * @param saver A skeleton file saver.
     */
    void add_saver( saver* svr );

    void init_default_loaders_and_savers() __attribute__((constructor));
    void release_loaders_and_savers() __attribute__((destructor));
  }
END_MP_NAMESPACE
# endif 
