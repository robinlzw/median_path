/* Created on: Mar 11, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_MEDIAN_FORMAT_H_
# define MEDIAN_PATH_MEDIAN_FORMAT_H_

# include "../io.h"
# include "io_utilities.h"

# include <graphics-origin/tools/filesystem.h>
# include <graphics-origin/tools/log.h>

# include "../../externals/rapidjson/reader.h"
# include "../../externals/rapidjson/filereadstream.h"
# include "../../externals/rapidjson/writer.h"
# include "../../externals/rapidjson/filewritestream.h"

BEGIN_MP_NAMESPACE
namespace io {

  static const std::string median_format_extension = ".median";

  struct median_saver
    : public saver {

    typedef rapidjson::Writer< rapidjson::FileWriteStream > json_writer;

    bool can_save_to( const std::string& filename ) override
    {
      return graphics_origin::tools::get_extension( filename ) == median_format_extension;
    }

    bool save( median_skeleton& skeleton, const std::string& filename ) override
    {
      std::FILE* pfile = std::fopen( filename.c_str(), "w" );
      char buffer[ 65536 ];
      rapidjson::FileWriteStream ws( pfile, buffer, sizeof(buffer) );
      json_writer writer( ws );

      writer.StartObject();

        write_header( skeleton, writer );

        write_atoms( skeleton, writer );
        write_links( skeleton, writer );
        write_faces( skeleton, writer );

        write_atom_properties( skeleton, writer );
        write_link_properties( skeleton, writer );
        write_face_properties( skeleton, writer );

      writer.EndObject();


      std::fclose( pfile );
      return true;
    }

    void write_header(
        median_skeleton& skeleton,
        json_writer& writer )
    {
      writer.Key( "header" );
      writer.StartObject();

        writer.Key( "author" );
        writer.String( "Dr. T. Delame" );

        writer.Key( "version" );
        writer.String( "1.0");

        writer.Key( "atoms" );
        writer.Uint64( skeleton.get_number_of_atoms() );

        writer.Key( "links" );
        writer.Uint64( skeleton.get_number_of_links() );

        writer.Key( "faces" );
        writer.Uint64( skeleton.get_number_of_faces() );

        writer.Key( "atom_properties" );
        writer.Uint64( skeleton.get_number_of_atom_properties() );

        writer.Key( "link_properties" );
        writer.Uint64( skeleton.get_number_of_link_properties() );

        writer.Key( "face_properties" );
        writer.Uint64( skeleton.get_number_of_face_properties() );

      writer.EndObject();
    }

    void write_atoms(
        median_skeleton& skeleton,
        json_writer& writer )
    {
      writer.Key( "atoms" );
      writer.StartArray();

        skeleton.process(
            [&writer]( median_skeleton::atom& a )
            {
              writer.Double( a.x );
              writer.Double( a.y );
              writer.Double( a.z );
              writer.Double( a.w );
            }, false );

      writer.EndArray();
    }

    void write_links(
        median_skeleton& skeleton,
        json_writer& writer )
    {
      writer.Key( "links" );
      writer.StartArray();

        skeleton.process(
            [&writer,&skeleton]( median_skeleton::link& l )
            {
              writer.Uint64( skeleton.get_index( l.h1 ) );
              writer.Uint64( skeleton.get_index( l.h2 ) );
            }, false );

      writer.EndArray();
    }

    void write_faces(
        median_skeleton& skeleton,
        json_writer& writer )
    {
      writer.Key( "faces" );
      writer.StartArray();

        skeleton.process(
            [&writer,&skeleton]( median_skeleton::face& f )
            {
              writer.Uint64( skeleton.get_index( f.atoms[0] ) );
              writer.Uint64( skeleton.get_index( f.atoms[1] ) );
              writer.Uint64( skeleton.get_index( f.atoms[2] ) );
            }, false );

      writer.EndArray();
    }

    void write_atom_properties(
        median_skeleton& skeleton,
        json_writer& writer )
    {
      writer.Key( "atom_properties" );
      writer.Null();
    }
    void write_link_properties(
        median_skeleton& skeleton,
        json_writer& writer )
    {
      writer.Key( "link_properties" );
      writer.Null();
    }
    void write_face_properties(
        median_skeleton& skeleton,
        json_writer& writer )
    {
      writer.Key( "face_properties" );
      writer.Null();
    }

  };

}
END_MP_NAMESPACE
# endif 
