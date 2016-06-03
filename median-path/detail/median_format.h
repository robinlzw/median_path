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
# include "../../externals/rapidjson/error/en.h"

BEGIN_MP_NAMESPACE
namespace io {

  static const std::string median_format_extension = ".median";

  struct median_reader_handler
    : public rapidjson::BaseReaderHandler< rapidjson::UTF8<>, median_reader_handler >
  {
    struct status {
      uint32_t expect_object_start      : 1;
      uint32_t expect_name_or_object_end: 1;

      uint32_t reading_header           : 1;
      uint32_t reading_atoms            : 1;
      uint32_t reading_links            : 1;
      uint32_t reading_faces            : 1;
      uint32_t reading_atom_properties  : 1;
      uint32_t reading_link_properties  : 1;
      uint32_t reading_face_properties  : 1;

      status()
        : expect_object_start{1},
          expect_name_or_object_end{0},
          reading_header{0},
          reading_atoms{0}, reading_links{0}, reading_faces{0},
          reading_atom_properties{0}, reading_link_properties{0}, reading_face_properties{0}
      {}
    };

    median_skeleton& m_skeleton;
    status m_status;
    uint8_t m_atom_index;
    median_skeleton::atom m_atom;
    uint8_t m_link_index;
    median_skeleton::atom_index m_link[2];
    uint8_t m_face_index;
    median_skeleton::atom_index m_face[3];

    median_reader_handler( median_skeleton& skeleton )
      : m_skeleton{ skeleton }, m_status{},
        m_atom_index{0},
        m_link_index{0}, m_link{ 0, 0 },
        m_face_index{0}, m_face{ 0, 0, 0 }
    {}


    bool Null()
    {
      return true;
    }
    bool Bool(bool b)
    {
      return true;

    }
    bool Int(int i)
    {
      if( m_status.reading_header )
        {
          if( m_status.reading_atoms )
            {
              m_skeleton.reserve_atoms( i );
              m_status.reading_atoms = 0;
            }
          else if( m_status.reading_links )
            {
              m_skeleton.reserve_links( i );
              m_status.reading_links = 0;
            }
          else if( m_status.reading_faces )
            {
              m_skeleton.reserve_faces( i );
              m_status.reading_faces = 0;
            }
        }
      else
        {
          if( m_status.reading_links )
            {
              m_link[ m_link_index ] = i;
              if( m_link_index == 1 )
                {
                  m_skeleton.add( m_link[0], m_link[1] );
                  m_link_index = 0;
                }
              else m_link_index = 1;
            }
          else if( m_status.reading_faces )
            {
              m_face[ m_face_index ] = i;
              if( m_face_index == 2 )
                {
                  m_skeleton.add( m_face[0], m_face[1], m_face[2] );
                  m_face_index = 0;
                }
              else ++ m_face_index;
            }
        }
      return true;
    }
    bool Uint(unsigned i)
    {
      if( m_status.reading_header )
        {
          if( m_status.reading_atoms )
            {
              m_skeleton.reserve_atoms( i );
              m_status.reading_atoms = 0;
            }
          else if( m_status.reading_links )
            {
              m_skeleton.reserve_links( i );
              m_status.reading_links = 0;
            }
          else if( m_status.reading_faces )
            {
              m_skeleton.reserve_faces( i );
              m_status.reading_faces = 0;
            }
        }
      else
        {
          if( m_status.reading_links )
            {
              m_link[ m_link_index ] = i;
              if( m_link_index == 1 )
                {
                  m_skeleton.add( m_link[0], m_link[1] );
                  m_link_index = 0;
                }
              else m_link_index = 1;
            }
          else if( m_status.reading_faces )
            {
              m_face[ m_face_index ] = i;
              if( m_face_index == 2 )
                {
                  m_skeleton.add( m_face[0], m_face[1], m_face[2] );
                  m_face_index = 0;
                }
              else ++ m_face_index;
            }
        }
      return true;
    }
    bool Int64(int64_t i)
    {
      if( m_status.reading_header )
        {
          if( m_status.reading_atoms )
            {
              m_skeleton.reserve_atoms( i );
              m_status.reading_atoms = 0;
            }
          else if( m_status.reading_links )
            {
              m_skeleton.reserve_links( i );
              m_status.reading_links = 0;
            }
          else if( m_status.reading_faces )
            {
              m_skeleton.reserve_faces( i );
              m_status.reading_faces = 0;
            }
        }
      else
        {
          if( m_status.reading_links )
            {
              m_link[ m_link_index ] = i;
              if( m_link_index == 1 )
                {
                  m_skeleton.add( m_link[0], m_link[1] );
                  m_link_index = 0;
                }
              else m_link_index = 1;
            }
          else if( m_status.reading_faces )
            {
              m_face[ m_face_index ] = i;
              if( m_face_index == 2 )
                {
                  m_skeleton.add( m_face[0], m_face[1], m_face[2] );
                  m_face_index = 0;
                }
              else ++ m_face_index;
            }
        }
      return true;
    }
    bool Uint64(uint64_t i)
    {
      if( m_status.reading_header )
        {
          if( m_status.reading_atoms )
            {
              m_skeleton.reserve_atoms( i );
              m_status.reading_atoms = 0;
            }
          else if( m_status.reading_links )
            {
              m_skeleton.reserve_links( i );
              m_status.reading_links = 0;
            }
          else if( m_status.reading_faces )
            {
              m_skeleton.reserve_faces( i );
              m_status.reading_faces = 0;
            }
        }
      else
        {
          if( m_status.reading_links )
            {
              m_link[ m_link_index ] = i;
              if( m_link_index == 1 )
                {
                  m_skeleton.add( m_link[0], m_link[1] );
                  m_link_index = 0;
                }
              else m_link_index = 1;
            }
          else if( m_status.reading_faces )
            {
              m_face[ m_face_index ] = i;
              if( m_face_index == 2 )
                {
                  m_skeleton.add( m_face[0], m_face[1], m_face[2] );
                  m_face_index = 0;
                }
              else ++ m_face_index;
            }
        }
      return true;
    }
    bool Double(double d)
    {
      if( !m_status.reading_header && m_status.reading_atoms )
        {
          m_atom[ m_atom_index ] = d;
          if( m_atom_index == 3 )
            {
              m_skeleton.add( m_atom );
              m_atom_index = 0;
            }
          else ++ m_atom_index;
        }
      return true;
    }

    bool String(const Ch* str, rapidjson::SizeType length, bool copy)
    {
      return true;
    }

    /**There are two objects
     *
     */
    bool StartObject()
    {
      if( m_status.expect_object_start )
        {
          m_status.expect_object_start = 0;
          m_status.expect_name_or_object_end = 1;
          return true;
        }
      LOG( error, "unexpected start of object");
      return false;
    }

    bool EndObject(rapidjson::SizeType memberCount)
    {
      if( m_status.expect_name_or_object_end )
        {
          if(m_status.reading_header )
            m_status.reading_header = 0;
        }
      return true;
    }


    bool Key(const Ch* str, rapidjson::SizeType length, bool copy)
    {
      std::string s( str, length );
      if( s == "header" )
        {
          m_status.reading_header = 1;
          m_status.expect_object_start = 1;
        }
      else if( s == "atoms" )
        m_status.reading_atoms = 1;
      else if( s == "links" )
        m_status.reading_links = 1;
      else if( s == "faces" )
        m_status.reading_faces = 1;
      return true;
    }

    bool StartArray()
    {
      return true;
    }
    bool EndArray(rapidjson::SizeType elementCount)
    {
      (void)elementCount;
      if( !m_status.reading_header )
        {
          if( m_status.reading_atoms )
            {
              if( m_atom_index )
                {
                  LOG( error, "unfinished atom");
                  return false;
                }
              m_status.reading_atoms = 0;
            }
          else if( m_status.reading_links )
            {
              if( m_link_index )
                {
                  LOG( error, "unfinished link");
                  return false;
                }
              m_status.reading_links = 0;
            }
          else if( m_status.reading_faces )
            {
              if( m_face_index )
                {
                  LOG( error, "unfinished face");
                  return false;
                }
              m_status.reading_faces = 0;
            }
        }
      return true;
    }
  };

  struct median_loader
    : public loader,
      public rapidjson::BaseReaderHandler< rapidjson::UTF8<>, median_loader >{

    bool can_load_from( const std::string& filename ) override
    {
      return graphics_origin::tools::get_extension( filename ) == median_format_extension;
    }

    bool load( median_skeleton& skeleton, const std::string& filename ) override
    {
      std::FILE* pfile = std::fopen( filename.c_str(), "r" );
      char buffer[ 65536 ];
      rapidjson::FileReadStream rs( pfile, buffer, sizeof(buffer) );
      rapidjson::Reader reader;
      median_reader_handler handler(skeleton);

      bool result = reader.Parse( rs, handler );

      if( !result )
        {
          rapidjson::ParseErrorCode e = reader.GetParseErrorCode();
          size_t o = reader.GetErrorOffset();
          LOG( error, "parse error of file [" << filename <<"] " << rapidjson::GetParseError_En(e) << " at offset " << o );
        }

      std::fclose( pfile );
      return result;
    }
  };

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

        skeleton.process_atoms(
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

        skeleton.process_links(
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

        skeleton.process_faces(
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
