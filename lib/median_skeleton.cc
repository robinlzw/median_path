/* Created on: Mar 4, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# include <median-path/median_skeleton.h>
# include <graphics-origin/tools/log.h>

# include <thrust/scan.h>

BEGIN_MP_NAMESPACE

  static const uint8_t atom_links_property_index = 0;
  static const uint8_t atom_faces_property_index = 1;
  static const uint8_t link_faces_property_index = 0;

  void
  median_skeleton::remove_link_to_face( link_index idx, face_handle handle )
  {
    auto& face_elements = m_impl->m_link_properties[ link_faces_property_index ]->get<link_faces_property>( idx );
    auto size = face_elements.size();
    if( size <= 1 )
      link_faces_property().swap( face_elements );
    else
      {
        --size;
        for( size_t i = 0; i < size; ++ i )
          {
            if( face_elements[i].face == handle )
              {
                face_elements[i] = std::move( face_elements.back() );
                break;
              }
          }
        face_elements.pop_back();
      }
  }

  void
  median_skeleton::remove_atom_to_face( atom_index idx, face_handle handle )
  {
    auto& face_elements = m_impl->m_atom_properties[ atom_faces_property_index ]->get<atom_faces_property>( idx );
    auto size = face_elements.size();
    if( size <= 1 )
      atom_faces_property().swap( face_elements );
    else
      {
        --size;
        for( size_t i = 0; i < size; ++ i )
          {
            if( face_elements[i].face == handle )
              {
                face_elements[i] = std::move( face_elements.back() );
                break;
              }
          }
        face_elements.pop_back();
      }
  }

  void
  median_skeleton::remove_atom_to_link( atom_index idx, link_handle handle )
  {
    auto& link_elements = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( idx );
    auto size = link_elements.size();
    if( size <= 1 )
      atom_links_property().swap( link_elements );
    else
      {
        --size;
        for( size_t i = 0; i < size; ++ i )
          {
            if( link_elements[i].first == handle )
              {
                link_elements[i] = std::move( link_elements.back() );
                break;
              }
          }
        link_elements.pop_back();
      }
  }

  median_skeleton::median_skeleton(
      atom_index atom_capacity,
      link_index link_capacity,
      face_index face_capacity)
    : m_impl{ new datastructure{ atom_capacity, link_capacity, face_capacity } }
  {
    m_impl->add_atom_property<atom_links_property>("links");
    m_impl->add_atom_property<atom_faces_property>("faces");
    m_impl->add_link_property<link_faces_property>("faces");
  }

  median_skeleton::median_skeleton( median_skeleton&& other )
    : m_impl{ other.m_impl }
  {
    other.m_impl = new datastructure{};
  }

  median_skeleton::median_skeleton( const std::string& filename )
  {
    load( filename );
  }

  median_skeleton::~median_skeleton()
  {
    delete m_impl;
  }

  void median_skeleton::clear(
      atom_index atom_capacity,
      uint64_t link_capacity,
      uint64_t face_capacity )
  {
    m_impl->clear( atom_capacity, link_capacity, face_capacity );
  }

  bool median_skeleton::load( const std::string& filename )
  {
    LOG( debug, "TODO: load a file not implemented");
    return false;
  }

  bool median_skeleton::save( const std::string& filename )
  {
    LOG( debug, "TODO: save a file not implemented");
    return false;
  }

  median_skeleton::atom_index median_skeleton::get_number_of_atoms() const noexcept
  {
    return m_impl->m_atoms_size;
  }
  uint64_t median_skeleton::get_number_of_links() const noexcept
  {
    return m_impl->m_links_size;
  }
  uint64_t median_skeleton::get_number_of_faces() const noexcept
  {
    return m_impl->m_faces_size;
  }

  median_skeleton::atom_handle
  median_skeleton::add( const vec3& position, const real& radius )
  {
    auto pair = m_impl->create_atom();
    pair.second = atom{ position, radius };
    return pair.first;
  }

  median_skeleton::atom_handle
  median_skeleton::add( const vec4& ball )
  {
    auto pair = m_impl->create_atom();
    pair.second = ball;
    return pair.first;
  }

  void median_skeleton::remove( atom_handle handle )
  {
    if( handle.index < m_impl->m_atoms_capacity )
      {
        const auto entry = m_impl->m_atom_handles + handle.index;
        if( entry->status == datastructure::STATUS_ALLOCATED && entry->counter == handle.counter )
          {
            auto index = entry->atom_index;

            auto& faces = m_impl->m_atom_properties[ atom_faces_property_index ]->get<atom_faces_property>( index );
            // remove all the faces passing by this atom
            for( auto& fh : faces )
              {
                // remove the mapping atom -> face for the two other atoms
                for( int i = 0; i < 2; ++ i )
                  {
                    remove_atom_to_face(
                        m_impl->m_atom_handles[ fh.atoms[i].index ].atom_index,
                        fh.face );
                  }
                // remove the mapping link -> face
                for( int i = 0; i < 3; ++ i )
                  {
                    remove_link_to_face(
                        m_impl->m_link_handles[ fh.links[i].index ].link_index,
                        fh.face );
                  }
                // remove this face from the tight buffer
                m_impl->remove( fh.face ); // this removes all properties of the face
              }
            atom_faces_property().swap( faces ); // all mappings atom -> face for this atom are removed

            auto& links = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( index );
            // remove all the links having this atom as end point
            for( auto& lh : links )
              {
                // remove the mapping atom -> link for the other end point
                remove_atom_to_link(
                    m_impl->m_atom_handles[ lh.second ].atom_index,
                    lh.first );

                // if this link was part of a face, the mapping link -> face is already removed

                // remove this link from the tight buffer
                m_impl->remove( lh.first ); // this removes all other properties of the link
              }
            atom_links_property().swap( links );

            // other properties are automatically removed during this call:
            m_impl->remove( handle );
          }
      }
  }

  void median_skeleton::remove( atom& e )
  {
    auto index = m_impl->get_index( e );
    auto& faces = m_impl->m_atom_properties[ atom_faces_property_index ]->get<atom_faces_property>( index );
    // remove all the faces passing by this atom
    for( auto& fh : faces )
      {
        // remove the mapping atom -> face for the two other atoms
        for( int i = 0; i < 2; ++ i )
          {
            remove_atom_to_face(
                m_impl->m_atom_handles[ fh.atoms[i].index ].atom_index,
                fh.face );
          }
        // remove the mapping link -> face
        for( int i = 0; i < 3; ++ i )
          {
            remove_link_to_face(
                m_impl->m_link_handles[ fh.links[i].index ].link_index,
                fh.face );
          }
        // remove this face from the tight buffer
        m_impl->remove( fh.face ); // this removes all properties of the face
      }
    atom_faces_property().swap( faces ); // all mappings atom -> face for this atom are removed

    auto& links = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( index );
    // remove all the links having this atom as end point
    for( auto& lh : links )
      {
        // remove the mapping atom -> link for the other end point
        remove_atom_to_link(
            m_impl->m_atom_handles[ lh.second ].atom_index,
            lh.first );

        // if this link was part of a face, the mapping link -> face is already removed

        // remove this link from the tight buffer
        m_impl->remove( lh.first ); // this removes all other properties of the link
      }
    atom_links_property().swap( links );

    // other properties are automatically removed during this call:
    m_impl->remove( e );
  }

  median_skeleton::atom&
  median_skeleton::get( atom_handle handle ) const
  {
    return m_impl->get(handle);
  }

  median_skeleton::atom&
  median_skeleton::get_atom_by_index( atom_index index ) const
  {
    return m_impl->get_atom_by_index( index );
  }

  median_skeleton::atom_index
  median_skeleton::get_index( atom_handle handle ) const
  {
    return m_impl->get_index( handle );
  }

  median_skeleton::atom_handle
  median_skeleton::get_handle( atom&e ) const
  {
    return m_impl->get_handle( e );
  }

  bool
  median_skeleton::is_valid( atom_handle handle ) const
  {
    if( handle.index < m_impl->m_atoms_capacity )
      {
        const auto entry = m_impl->m_atom_handles + handle.index;
        return entry->status == datastructure::STATUS_ALLOCATED && entry->counter == handle.counter;
      }
    return false;
  }

  void
  median_skeleton::process( atom_processer&& function, bool parallel )
  {
    // TODO: use arguments in the pragma to use or not threads
    if( parallel )
      {
        # pragma omp parallel for schedule(dynamic)
        for( atom_index i = 0; i < m_impl->m_atoms_size; ++ i )
          {
            function( m_impl->m_atoms[i]);
          }
      }
    else
      {
        for( atom_index i = 0; i < m_impl->m_atoms_size; ++ i )
          {
            function( m_impl->m_atoms[i]);
          }
      }
  }

  void
  median_skeleton::remove( atom_filter&& filter, bool parallel )
  {
    const atom_index size = m_impl->m_atoms_size;
    /* a flag buffer is necessary if the filter function relies on reference
     * to atoms or atom index. For example, if the filter function remove
     * atoms with indices 1 and 3, with 4 atoms in total in the buffer. When
     * atom #1 is removed, the atom #3 is move into it. Thus, when the former
     * atom #3 (now #1) is tested, the filter function will not recognize it. */
    bool* flags = new bool[ size ];
    if( parallel )
      {
        # pragma omp parallel for schedule(dynamic)
        for( atom_index i = 0; i < size; ++ i )
          {
            flags[ i ] = filter( m_impl->m_atoms[i] );
          }
      }
    else
      {
        for( atom_index i = 0; i < size; ++ i )
          {
            flags[ i ] = filter( m_impl->m_atoms[i] );
          }
      }

    /* INVARIANT:
     * - all atoms with indices in [[0, left[[ are not removed
     * - all atoms with indices in [[left, right[[ need to be checked
     * - all atoms with indices in [[right, size[[ should be cleaned (all
     * objects referring to an atom should be destroyed)
     * Thus, in the end (i.e. when left == right ), we have:
     * - atoms with indices in [[0,left[[ are clean and tight
     * - atoms with indices in [[left, size[[ should be cleaned
     * - left is the new size of atoms
     */
    atom_index next_free_slot = m_impl->m_atoms_next_free_handle_slot;
    atom_index left = 0, right = size;
    while( left < right )
      {
        if( flags[ left ] )
          {
            // look for a valid atom in the end of the buffer
            --right;
            while( left < right && flags[ right ] )
              {
                const auto entry_index = m_impl->m_atom_index_to_handle_index[ right ];
                auto entry = m_impl->m_atom_handles + entry_index;
                entry->next_free_index = next_free_slot;
                entry->status = datastructure::STATUS_FREE;
                next_free_slot = entry_index;
                --right;
              }

            if( left != right )
              {
                // update the left_entry
                const auto left_entry_index = m_impl->m_atom_index_to_handle_index[ left ];
                auto left_entry = m_impl->m_atom_handles + left_entry_index;
                left_entry->next_free_index = next_free_slot;
                left_entry->status = datastructure::STATUS_FREE;
                next_free_slot = left_entry_index;

                // move the last atom into this one to keep the buffer tight
                m_impl->m_atoms[left] = std::move( m_impl->m_atoms[right] );
                for( auto& property : m_impl->m_atom_properties )
                  property->move( right, left );

                const auto right_entry_index = m_impl->m_atom_index_to_handle_index[ right ];
                auto right_entry = m_impl->m_atom_handles + right_entry_index;
                right_entry->atom_index = left;
                m_impl->m_atom_index_to_handle_index[ left ] = right_entry_index;
              }
          }
        else ++ left;
      }
    delete[] flags;

    // cleaning atoms in [[left, size[[
    for ( atom_index index = left; index < size; ++ index )
      {
        auto& faces = m_impl->m_atom_properties[ atom_faces_property_index ]->get<atom_faces_property>( index );
        // remove all the faces passing by this atom
        for( auto& fh : faces )
          {
            // remove the mapping atom -> face for the two other atoms
            for( int i = 0; i < 2; ++ i )
              {
                remove_atom_to_face(
                    m_impl->m_atom_handles[ fh.atoms[i].index ].atom_index,
                    fh.face );
              }
             // remove the mapping link -> face
             for( int i = 0; i < 3; ++ i )
               {
                 remove_link_to_face(
                     m_impl->m_link_handles[ fh.links[i].index ].link_index,
                     fh.face );
               }
             // remove this face from the tight buffer
             m_impl->remove( fh.face ); // this removes all properties of the face
           }
         atom_faces_property().swap( faces ); // all mappings atom -> face for this atom are removed

         auto& links = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( index );
         // remove all the links having this atom as end point
         for( auto& lh : links )
           {
             // remove the mapping atom -> link for the other end point
             remove_atom_to_link(
                 m_impl->m_atom_handles[ lh.second ].atom_index,
                 lh.first );

             // if this link was part of a face, the mapping link -> face is already removed

             // remove this link from the tight buffer
             m_impl->remove( lh.first ); // this removes all other properties of the link
           }
         atom_links_property().swap( links );
      }
    // destroy properties of atoms in [[left, size[[
    for( auto& property : m_impl->m_atom_properties )
      property->destroy( left, size );

    m_impl->m_atoms_size = left;
    m_impl->m_atoms_next_free_handle_slot = next_free_slot;
  }

  median_skeleton::link_handle
  median_skeleton::add( atom_handle handle1, atom_handle handle2 )
  {
    if( handle1.index < m_impl->m_atoms_capacity && handle2.index < m_impl->m_atoms_capacity )
      {
        const auto entry1 = m_impl->m_atom_handles + handle1.index;
        const auto entry2 = m_impl->m_atom_handles + handle2.index;
        if(( entry1->status == datastructure::STATUS_ALLOCATED && entry1->counter == handle1.counter )
         &&( entry2->status == datastructure::STATUS_ALLOCATED && entry2->counter == handle2.counter ) )
          {
            auto& links1 = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( entry1->atom_index );
            // check for existing link
            for( auto& link : links1 )
              {
                if( link.second == handle2 )
                  return link.first;
              }
            auto& links2 = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( entry2->atom_index );

            // set the link data
            auto result = m_impl->create_link();
            result.second.h1 = handle1;
            result.second.h2 = handle2;

            // set the link property for corresponding atoms
            links1.push_back( std::make_pair( result.first, handle2 ) );
            links2.push_back( std::make_pair( result.first, handle1 ) );

            return result.first;
          }
      }
    MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
  }

  median_skeleton::link_handle
  median_skeleton::add( atom& atom1, atom& atom2 )
  {
    if( &atom1 >= m_impl->m_atoms && &atom2 >= m_impl->m_atoms
     && &atom1 <  m_impl->m_atoms + m_impl->m_atoms_size && &atom2 < m_impl->m_atoms + m_impl->m_atoms_size )
      {
        atom_index idx1 = std::distance( m_impl->m_atoms, &atom1 );
        atom_index idx2 = std::distance( m_impl->m_atoms, &atom2 );
        if( m_impl->m_atoms + idx1 == &atom1 && m_impl->m_atoms + idx2 == &atom2 )
          {
            auto entry_index2 = m_impl->m_atom_index_to_handle_index[ idx2 ];
            atom_handle handle2( entry_index2, m_impl->m_atom_handles[ entry_index2 ].counter );
            auto& links1 = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( idx1 );
            // check for existing link
            for( auto& link : links1 )
              {
                if( link.second == handle2 )
                  return link.first;
              }
            auto entry_index1 = m_impl->m_atom_index_to_handle_index[ idx1 ];
            atom_handle handle1( entry_index1, m_impl->m_atom_handles[ entry_index1 ].counter );
            auto& links2 = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( idx2 );

            // set the link data
            auto result = m_impl->create_link();
            result.second.h1 = handle1;
            result.second.h2 = handle2;

            // set the link property for corresponding atoms
            links1.push_back( std::make_pair( result.first, handle2 ) );
            links2.push_back( std::make_pair( result.first, handle1 ) );

            return result.first;
          }
      }
    MP_THROW_EXCEPTION( skeleton_invalid_atom_handle );
  }

  median_skeleton::link_handle
  median_skeleton::add( atom_index idx1, atom_index idx2 )
  {
    if( idx1 < m_impl->m_atoms_size && idx2 < m_impl->m_atoms_size )
      {
        auto entry_index2 = m_impl->m_atom_index_to_handle_index[ idx2 ];
        atom_handle handle2( entry_index2, m_impl->m_atom_handles[ entry_index2 ].counter );
        auto& links1 = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( idx1 );
        // check for existing link
        for( auto& link : links1 )
          {
            if( link.second == handle2 )
              return link.first;
          }
        auto entry_index1 = m_impl->m_atom_index_to_handle_index[ idx1 ];
        atom_handle handle1( entry_index1, m_impl->m_atom_handles[ entry_index1 ].counter );
        auto& links2 = m_impl->m_atom_properties[ atom_links_property_index ]->get<atom_links_property>( idx2 );

        // set the link data
        auto result = m_impl->create_link();
        result.second.h1 = handle1;
        result.second.h2 = handle2;

        // set the link property for corresponding atoms
        links1.push_back( std::make_pair( result.first, handle2 ) );
        links2.push_back( std::make_pair( result.first, handle1 ) );

        return result.first;
      }
    MP_THROW_EXCEPTION( skeleton_invalid_atom_index );
  }

  void
  median_skeleton::remove( link_handle handle )
  {
    if( handle.index < m_impl->m_links_capacity )
      {
        const auto link_entry =  m_impl->m_link_handles + handle.index;
        if( link_entry->status == datastructure::STATUS_ALLOCATED && link_entry->counter == handle.counter )
          {
            const auto link_index = link_entry->link_index;
            auto& link = m_impl->m_links[ link_index ];
            atom_index idx1 = m_impl->m_atom_handles[ link.h1.index ].atom_index;
            atom_index idx2 = m_impl->m_atom_handles[ link.h2.index ].atom_index;


            // we start by destroying all the faces built with this link
            auto& faces = m_impl->m_link_properties[ link_faces_property_index ]->get<link_faces_property>( link_index );
            for( auto& face : faces )
              {
                // remove mapping link -> face from the two other links
                for( int i = 0; i < 2; ++ i )
                  {
                    remove_link_to_face(
                        m_impl->m_link_handles[ face.links[i].index ].link_index,
                        face.face );
                  }
                // remove mapping atom -> face from the three atoms
                remove_atom_to_face( idx1, face.face );
                remove_atom_to_face( idx2, face.face );
                remove_atom_to_face(
                    m_impl->m_atom_handles[ face.opposite.index ].atom_index,
                    face.face );
                // remove the face from the tight buffer
                m_impl->remove( face.face ); // this removes all other properties of that face
              }
            link_faces_property().swap( faces ); // all mappings link -> face for this link are removed

            // remove that link
            remove_atom_to_link( idx1, handle );
            remove_atom_to_link( idx2, handle );
            m_impl->remove( handle ); // this removes all other properties of that link
          }
      }
  }

  void
  median_skeleton::remove( link& e )
  {
    auto link_index = m_impl->get_index( e );

    atom_index idx1 = m_impl->m_atom_handles[ e.h1.index ].atom_index;
    atom_index idx2 = m_impl->m_atom_handles[ e.h2.index ].atom_index;


    // we start by destroying all the faces built with this link
    auto& faces = m_impl->m_link_properties[ link_faces_property_index ]->get<link_faces_property>( link_index );
    for( auto& face : faces )
      {
        // remove mapping link -> face from the two other links
        for( int i = 0; i < 2; ++ i )
          {
            remove_link_to_face(
                m_impl->m_link_handles[ face.links[i].index ].link_index,
                face.face );
          }
        // remove mapping atom -> face from the three atoms
        remove_atom_to_face( idx1, face.face );
        remove_atom_to_face( idx2, face.face );
        remove_atom_to_face(
            m_impl->m_atom_handles[ face.opposite.index ].atom_index,
            face.face );
        // remove the face from the tight buffer
        m_impl->remove( face.face ); // this removes all other properties of that face
      }
    link_faces_property().swap( faces ); // all mappings link -> face for this link are removed

    // remove that link
    auto handle = link_handle( m_impl->m_link_index_to_handle_index[ link_index ], 0 );
    handle.counter = m_impl->m_link_handles[ handle.index ].counter;
    remove_atom_to_link( idx1, handle );
    remove_atom_to_link( idx2, handle );
    m_impl->remove( e ); // this removes all other properties of that link
  }

  median_skeleton::link&
  median_skeleton::get( link_handle handle ) const
  {
    return m_impl->get( handle );
  }

  median_skeleton::link&
  median_skeleton::get_link_by_index( link_index index ) const
  {
    return m_impl->get_link_by_index( index );
  }

  median_skeleton::link_index
  median_skeleton::get_index( link_handle handle ) const
  {
    return m_impl->get_index( handle );
  }

  median_skeleton::link_handle
  median_skeleton::get_handle( link& e ) const
  {
    return m_impl->get_handle( e );
  }

  bool
  median_skeleton::is_valid( link_handle handle ) const
  {
    if( handle.index < m_impl->m_links_capacity )
      {
        const auto entry = m_impl->m_link_handles + handle.index;
        return entry->status == datastructure::STATUS_ALLOCATED && entry->counter == handle.counter;
      }
    return false;
  }

  void
  median_skeleton::process( link_processer&& function, bool parallel )
  {
    // TODO: use arguments in the pragma to use or not threads
    if( parallel )
      {
        # pragma omp parallel for schedule(dynamic)
        for( link_index i = 0; i < m_impl->m_links_size; ++ i )
          {
            function( m_impl->m_links[i]);
          }
      }
    else
      {
        for( link_index i = 0; i < m_impl->m_links_size; ++ i )
          {
            function( m_impl->m_links[i]);
          }
      }
  }


  void
  median_skeleton::remove( link_filter&& filter, bool parallel )
  {
    const link_index size = m_impl->m_links_size;
    /* a flag buffer is necessary if the filter function relies on reference
     * to links or link index. For example, if the filter function remove
     * links with indices 1 and 3, with 4 links in total in the buffer. When
     * link #1 is removed, the link #3 is move into it. Thus, when the former
     * link #3 (now #1) is tested, the filter function will not recognize it. */
    bool* flags = new bool[ size ];
    if( parallel )
      {
        # pragma omp parallel for schedule(dynamic)
        for( link_index i = 0; i < size; ++ i )
          {
            flags[ i ] = filter( m_impl->m_links[i] );
          }
      }
    else
      {
        for( link_index i = 0; i < size; ++ i )
          {
            flags[ i ] = filter( m_impl->m_links[i] );
          }
      }

    /* INVARIANT:
     * - all links with indices in [[0, left[[ are not removed
     * - all links with indices in [[left, right[[ need to be checked
     * - all links with indices in [[right, size[[ should be cleaned (all
     * objects referring to an link should be destroyed)
     * Thus, in the end (i.e. when left == right ), we have:
     * - links with indices in [[0,left[[ are clean and tight
     * - links with indices in [[left, size[[ should be cleaned
     * - left is the new size of links
     */
    link_index next_free_slot = m_impl->m_links_next_free_handle_slot;
    link_index left = 0, right = size;
    while( left < right )
      {
        if( flags[ left ] )
          {
            // look for a valid link in the end of the buffer
            --right;
            while( left < right && flags[ right ] )
              {
                const auto entry_index = m_impl->m_link_index_to_handle_index[ right ];
                auto entry = m_impl->m_link_handles + entry_index;
                entry->next_free_index = next_free_slot;
                entry->status = datastructure::STATUS_FREE;
                next_free_slot = entry_index;
                --right;
              }

            if( left != right )
              {
                // update the left_entry
                const auto left_entry_index = m_impl->m_link_index_to_handle_index[ left ];
                auto left_entry = m_impl->m_link_handles + left_entry_index;
                left_entry->next_free_index = next_free_slot;
                left_entry->status = datastructure::STATUS_FREE;
                next_free_slot = left_entry_index;

                // move the last link into this one to keep the buffer tight
                m_impl->m_links[left] = std::move( m_impl->m_links[right] );
                for( auto& property : m_impl->m_link_properties )
                  property->move( right, left );

                const auto right_entry_index = m_impl->m_link_index_to_handle_index[ right ];
                auto right_entry = m_impl->m_link_handles + right_entry_index;
                right_entry->link_index = left;
                m_impl->m_link_index_to_handle_index[ left ] = right_entry_index;
              }
          }
        else ++ left;
      }
    delete[] flags;

    // cleaning links in [[left, size[[
    for ( link_index index = left; index < size; ++ index )
      {
        link& e = m_impl->m_links[ index ];
        atom_index idx1 = m_impl->m_atom_handles[ e.h1.index ].atom_index;
        atom_index idx2 = m_impl->m_atom_handles[ e.h2.index ].atom_index;

        // we start by destroying all the faces built with this link
        auto& faces = m_impl->m_link_properties[ link_faces_property_index ]->get<link_faces_property>( index );
        for( auto& face : faces )
          {
            // remove mapping link -> face from the two other links
            for( int i = 0; i < 2; ++ i )
              {
                remove_link_to_face(
                    m_impl->m_link_handles[ face.links[i].index ].link_index,
                    face.face );
              }
            // remove mapping atom -> face from the three atoms
            remove_atom_to_face( idx1, face.face );
            remove_atom_to_face( idx2, face.face );
            remove_atom_to_face(
                m_impl->m_atom_handles[ face.opposite.index ].atom_index,
                face.face );
            // remove the face from the tight buffer
            m_impl->remove( face.face ); // this removes all other properties of that face
          }
        link_faces_property().swap( faces ); // all mappings link -> face for this link are removed

        // remove that link
        auto handle = link_handle( m_impl->m_link_index_to_handle_index[ index ], 0 );
        handle.counter = m_impl->m_link_handles[ handle.index ].counter;
        remove_atom_to_link( idx1, handle );
        remove_atom_to_link( idx2, handle );
      }
    // destroy properties of atoms in [[left, size[[
    for( auto& property : m_impl->m_atom_properties )
      property->destroy( left, size );

    m_impl->m_atoms_size = left;
    m_impl->m_atoms_next_free_handle_slot = next_free_slot;
  }

END_MP_NAMESPACE
