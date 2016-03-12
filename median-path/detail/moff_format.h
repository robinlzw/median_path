/* Created on: Mar 11, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_MOFF_FORMAT_H_
# define MEDIAN_PATH_MOFF_FORMAT_H_

# include "../io.h"
# include "io_utilities.h"

# include <graphics-origin/tools/filesystem.h>
# include <graphics-origin/tools/log.h>

# include <iomanip>

BEGIN_MP_NAMESPACE
  namespace io {

    static const std::string moff_format_extension;

    struct moff_saver
      : public saver {
      bool can_save_to( const std::string& filename ) override
      {
        return graphics_origin::tools::get_extension( filename ) == moff_format_extension;
      }
      bool save( median_skeleton& skeleton, const std::string& filename ) override
      {
        std::ofstream output( filename );

        output << "MOFF " << skeleton.get_number_of_atoms() << " " << skeleton.get_number_of_faces() << "\n";
        output.precision( 10 );
        skeleton.process( [&output]( median_skeleton::atom& atom )
          {
            output << std::setw( 13 ) << atom.x << " "
                   << std::setw( 13 ) << atom.y << " "
                   << std::setw( 13 ) << atom.z << " "
                   << std::setw( 13 ) << atom.w << "\n";
          }
        , false );

        skeleton.process( [&output,&skeleton]( median_skeleton::face& face )
          {
            output << "3 " << skeleton.get_index( face.atoms[0] ) << " "
                << skeleton.get_index( face.atoms[1] ) << " "
                << skeleton.get_index( face.atoms[2] ) << "\n";
          }
        , false );

        output.close();
        return true;
      }
    };

    struct moff_loader
      : public loader {
      bool can_load_from( const std::string& filename ) override
      {
        return graphics_origin::tools::get_extension( filename ) == moff_format_extension;
      }
      bool load( median_skeleton& skeleton, const std::string& filename ) override
      {
        std::ifstream input( filename );
        size_t lnumber = 0;

        bool result = true;
        if( !read_header( input, lnumber, skeleton )
          ||!read_atoms( input, lnumber, skeleton )
          ||!read_faces( input, lnumber, skeleton ))
          {
            skeleton.clear(0,0,0);
            LOG( error, "failed to load skeleton from MOFF file [" << filename << "]");
            result = true;
          }
        input.close();
        return result;
      }

      bool read_header( std::ifstream& input, size_t& lnumber, median_skeleton& skeleton )
      {
        std::string header_line;
        if( !get_next_relevant_line( input, lnumber, header_line ) )
          return false;

        std::istringstream tokenizer( header_line );
        std::string magic_word;
        real scale = 0;
        median_skeleton::atom_index natoms = 0;
        median_skeleton::face_index nfaces = 0;

        tokenizer >> magic_word >> natoms >> nfaces >> scale;
        if( tokenizer.fail() )
          {
            LOG( error, "line " << lnumber << " \"" << header_line << "\" is not a valid MOFF header");
            return false;
          }
        if( magic_word != "MOFF" )
          {
            LOG( error, "wrong magic word in MOFF header at line " << lnumber << " \"" << header_line << "\"");
            return false;
          }
        skeleton.clear( natoms, natoms * 3, nfaces );
        return true;
      }

      bool read_atoms( std::ifstream& input, size_t& lnumber, median_skeleton& skeleton )
      {
        auto natoms = skeleton.get_atoms_capacity();
        std::string vertex_string;
        vec4 ball;
        for( median_skeleton::atom_index i = 0; i < natoms; ++ i )
          {
            if( !get_next_relevant_line( input, lnumber, vertex_string ) )
              {
                LOG( error, "failed to get a relevant line for atom #" << i );
                return false;
              }
            std::istringstream tokenizer( vertex_string );
            tokenizer >> ball.x >> ball.y >> ball.z >> ball.w;
            if( tokenizer.fail() )
              {
                LOG( error, "something went wrong at line " << lnumber << " when reading atom #" << i << " from \"" << vertex_string << "\"");
                return false;
              }
            skeleton.add( ball );
          }
        return true;
      }

      bool read_faces( std::ifstream& input, size_t& lnumber, median_skeleton& skeleton )
      {
        auto nfaces = skeleton.get_faces_capacity();
        std::string face_string;
        std::vector< median_skeleton::atom_index > indices;
        median_skeleton::atom_index number_of_indices = 0;

        for( median_skeleton::face_index i = 0; i < nfaces; ++ i )
          {
            indices.clear();
            number_of_indices = 0;

            if( !get_next_relevant_line( input, lnumber, face_string ) )
              {
                LOG( error, "failed to get a relevant line for face #" << i );
                return false;
              }

            std::istringstream tokenizer( face_string );
            tokenizer >> number_of_indices;
            indices.resize( number_of_indices );
            for( median_skeleton::atom_index j = 0; j < number_of_indices ; ++ j )
              {
                tokenizer >> indices[ j ];
              }

            if( tokenizer.fail() )
              {
                LOG( error, "something went wrong at line " << lnumber
                     << " when reading face #" << i << " from \""
                     << face_string << "\"");
                return false;
              }
            if( number_of_indices > 1 )
              {
                auto last_index = indices.back();
                if( last_index >= skeleton.get_number_of_atoms() )
                  {
                    LOG( error, "wrong atom index " << last_index << " at line "
                        << lnumber << " when reading face #" << i << " from \""
                        << face_string << "\"");
                    return false;
                  }
                auto force_index = last_index;
                for( auto& current_index: indices )
                  {
                    if( current_index >= skeleton.get_number_of_atoms() )
                      {
                        LOG( error, "wrong atom index " << current_index << " at line "
                            << lnumber << " when reading face #" << i << " from \""
                            << face_string << "\"");
                        return false;
                      }
                    skeleton.add( current_index, last_index );
                    if( current_index != force_index && last_index != force_index )
                      {
                        skeleton.add( current_index, force_index );
                        skeleton.add( current_index, force_index, last_index );
                      }
                    last_index = current_index;
                  }
              }
          }
        return true;
      }
    };
  }
END_MP_NAMESPACE
# endif 
