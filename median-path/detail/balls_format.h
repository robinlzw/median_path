/* Created on: Mar 11, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_BALLS_FORMAT_H_
# define MEDIAN_PATH_BALLS_FORMAT_H_

# include "../io.h"
# include "io_utilities.h"

# include <graphics-origin/tools/filesystem.h>
# include <graphics-origin/tools/log.h>

# include <iomanip>

BEGIN_MP_NAMESPACE
namespace io {

    static const std::string balls_format_extension = ".balls";

    struct balls_saver
      : public saver {
      bool can_save_to( const std::string& filename ) override
      {
        return graphics_origin::tools::get_extension( filename ) == balls_format_extension;
      }
      bool save( median_skeleton& skeleton, const std::string& filename ) override
      {
        std::ofstream output( filename );

        output << skeleton.get_number_of_atoms() << "\n";
        output.precision( 10 );
        skeleton.process( [&output]( median_skeleton::atom& atom )
          {
            output << std::setw( 13 ) << atom.x << " "
                   << std::setw( 13 ) << atom.y << " "
                   << std::setw( 13 ) << atom.z << " "
                   << std::setw( 13 ) << atom.w << "\n";
          }
        , false );
        output.close();
        return true;
      }
    };

    struct balls_loader
      : public loader {
      bool can_load_from( const std::string& filename ) override
      {
        return graphics_origin::tools::get_extension( filename ) == balls_format_extension;
      }
      bool load( median_skeleton& skeleton, const std::string& filename ) override
      {
        std::ifstream input( filename );
        size_t lnumber = 0;
        std::string file_line = "";
        median_skeleton::atom_index natoms = 0;

        if( !get_next_relevant_line( input, lnumber, file_line ) )
          {
            LOG( error, "failed to get the number of atoms in BALLS file [" << filename << "]");
            return false;
          }
        else
          {
            std::istringstream tokenizer( file_line );
            tokenizer >> natoms;
            if( tokenizer.fail() )
              {
                LOG( error, "failed to recognize the number of balls at line " << lnumber
                     << " of file [" << filename << "]: \"" << file_line << "\"");
                return false;
              }
          }

        bool result = true;
        std::vector< real > balls_data( natoms * 4, 0 );
        size_t i = 0;
        // fetch the relevant lines to fill the balls data
        while( get_next_relevant_line( input, lnumber, file_line ) )
          {
            // stop when an error occurred or when we read enough data
            if( !result || i == natoms * 4 ) break;
            std::istringstream tokenizer( file_line );
            // read this line
            do
              {
                tokenizer >> balls_data[i];
                if( tokenizer.eof() )
                  {
                    ++i;
                    break;
                  }
                if( tokenizer.fail() )
                  {
                    LOG( error, "failed to read component " << i % 4 << " of atom #"
                       << (i >> 2) << " at line " << lnumber << " \"" << file_line << "\"");
                    result = false;
                    break;
                  }
                ++ i;
              }
            while( 1 );
          }

        if( i != natoms * 4 )
          {
            LOG( error, "failed to read enough data in the file to setup " << natoms << " balls");
            result = false;
          }

        if( result )
          {
            skeleton.clear( natoms, 0, 0 );
            for( median_skeleton::atom_index i = 0; i < natoms; ++ i )
              {
                real* data = &balls_data[ 4 * i ];
                skeleton.add( vec4{ data[0], data[1], data[2], data[2] });
              }
          }
        else
          {
            skeleton.clear( 0, 0, 0 );
            LOG( error, "failed to load skeleton from BALLS file [" << filename << "]");
          }
        return result;
      }
    };
}

END_MP_NAMESPACE
# endif 
