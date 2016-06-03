/*  Created on: Mar 20, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */

# ifndef MEDIAN_PATH_WEB_FORMAT_H_
# define MEDIAN_PATH_WEB_FORMAT_H_

# include "../io.h"
# include "io_utilities.h"

# include <graphics-origin/tools/filesystem.h>
# include <graphics-origin/tools/log.h>

# include <iomanip>

BEGIN_MP_NAMESPACE
namespace io {

  static const std::string web_format_extension = ".web";

  struct web_saver
    : public saver {
    bool can_save_to( const std::string& filename ) override
    {
      return graphics_origin::tools::get_extension( filename ) == web_format_extension;
    }
    bool save( median_skeleton& skeleton, const std::string& filename ) override
    {
      std::ofstream output( filename );
      if( !output.is_open() )
        {
          LOG( error, "cannot open file " << filename );
          return false;
        }
      output.precision( 8 );

      const auto natoms = skeleton.get_number_of_atoms();
      const auto nlinks = skeleton.get_number_of_links();
      const auto nfaces = skeleton.get_number_of_faces();
      real min_radius = REAL_MAX, max_radius = -1.0;
      skeleton.process_atoms( [&min_radius,&max_radius]( median_skeleton::atom& atom )
        {
          min_radius = std::min( min_radius, atom.w );
          max_radius = std::max( max_radius, atom.w );
        }, false );

      output << "{\"author\":\"Dr. T. Delame\",\"number_of_atoms\":" << natoms
          << ",\"number_of_links\":" << nlinks
          << ",\"number_of_faces\":" << nfaces
          << ",\"max_radius\":" << max_radius
          << ",\"min_radius\":" << min_radius
          << ",\"atoms\":[";

      for( median_skeleton::atom_index i = 0; i < natoms; ++ i )
        {
          const auto& atom = skeleton.get_atom_by_index( i );
          output << atom.x << ',' << atom.y << ',' << atom.z << ',' << atom.w;
          if( i + 1 < natoms ) output << ',';
        }
      output << "],\"links\":[";
      for( median_skeleton::link_index i = 0; i < nlinks; ++ i )
        {
          const auto& link = skeleton.get_link_by_index( i );
          output << skeleton.get_index(link.h1) << ',' << skeleton.get_index(link.h2);
          if( i + 1 < nlinks ) output << ',';
        }
      output << "],\"faces\":[";
      for( median_skeleton::face_index i = 0; i < nfaces; ++ i )
        {
          const auto& face = skeleton.get_face_by_index( i );
          output << skeleton.get_index( face.atoms[0] ) << ','
              << skeleton.get_index( face.atoms[1] ) << ','
              << skeleton.get_index( face.atoms[2] );
          if( i + 1 < nfaces ) output << ',';
        }
      output << "]}" << std::flush;
      output.close();
      return true;
    }
  };

}
END_MP_NAMESPACE
# endif
