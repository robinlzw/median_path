/* Created on: Mar 11, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# include "../median-path/io.h"
# include "../median-path/median_skeleton.h"
# include "../median-path/detail/balls_format.h"
# include "../median-path/detail/moff_format.h"
# include "../median-path/detail/median_format.h"

# include <graphics-origin/tools/filesystem.h>
# include <graphics-origin/tools/log.h>

# include <list>
# include <fstream>
# include <mutex>

BEGIN_MP_NAMESPACE

  namespace io {

    static
    std::list<saver* >&
    get_savers()
    {
      static std::list< saver*> instance;
      return instance;
    }

    static
    std::list<loader* >&
    get_loaders()
    {
      static std::list< loader* > instance;
      return instance;
    }

    static std::mutex loaders_mutex;
    static std::mutex savers_mutex;

    bool can_load_from( const std::string& filename )
    {
      if( !graphics_origin::tools::file_exist( filename ) )
        {
          LOG( error, "cannot load skeleton file [" << filename << "]: file does not exist");
          return false;
        }
      bool result = false;
      loaders_mutex.lock();
        for( auto& pldr : get_loaders() )
          {
            if( pldr->can_load_from( filename ) )
              {
                result = true;
                break;
              }
          }
      loaders_mutex.unlock();
      return result;
    }

    bool can_save_to( const std::string& filename )
    {
      bool result = false;
      savers_mutex.lock();
        for( auto& psvr : get_savers() )
          {
            if( psvr->can_save_to( filename ) )
              {
                result = true;
                break;
              }
          }
      savers_mutex.unlock();
      return result;
    }

    bool load( median_skeleton& skeleton, const std::string& filename )
    {
      if( !graphics_origin::tools::file_exist( filename ) )
        {
          LOG( error, "cannot load skeleton file [" << filename << "]: file does not exist");
          return false;
        }
      bool result = false;
      loaders_mutex.lock();
        for( auto& pldr : get_loaders() )
          {
            if( pldr->load( skeleton, filename ) )
              {
                result = true;
                break;
              }
          }
      loaders_mutex.unlock();
      return result;
    }

    bool save( median_skeleton& skeleton, const std::string& filename )
    {
      bool result = false;
      savers_mutex.lock();
        for( auto& psvr : get_savers() )
          {
            if( psvr->save( skeleton, filename ) )
              {
                result = true;
                break;
              }
          }
      savers_mutex.unlock();
      return result;
    }

    void add_loader( loader* ldr )
    {
      loaders_mutex.lock();
        get_loaders().push_back( ldr );
      loaders_mutex.unlock();
    }

    void add_saver( saver* svr )
    {
      savers_mutex.lock();
        get_savers().push_back( svr );
      savers_mutex.unlock();
    }

    static void init_default_loaders_and_savers() __attribute__((constructor));
    static void init_default_loaders_and_savers()
    {
      add_loader( new moff_loader );
//      add_loader( new balls_loader );
//      add_loader( new median_loader );

      add_saver( new moff_saver );
//      add_saver( new balls_saver );
//      add_saver( new median_loader );
    }
    static void release_loaders_and_savers() __attribute__((constructor));
    static void release_loaders_and_savers()
    {
      while( !get_loaders().empty() )
        {
          delete get_loaders().front();
          get_loaders().pop_front();
        }
      while( !get_savers().empty() )
        {
          delete get_savers().front();
          get_savers().pop_front();
        }
    }
  }

END_MP_NAMESPACE
