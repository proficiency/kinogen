#pragma once

// todo: refactor this all

class c_entry
{
public:
	c_entry( const json& response ) : m_poster( )
	{
		m_metadata.fill( response );
	}

	c_entry( Api::c_film response ) : m_metadata( response ), m_poster( )
	{
	}

	void init( )
	{
		create_poster( );
	}

	bool has_poster( ) const
	{
		return !m_poster_buffer.empty( );
	}

	// surface, texture
	Poster poster( ) const
	{
		return m_poster;
	}

	const Api::c_film& metadata( ) const
	{
		return m_metadata;
	}

	json response( ) const
	{
		return m_metadata.response( );
	}

	std::string path( )
	{
		return m_metadata.path( );
	}

private:
	void create_poster( )
	{
		m_poster_buffer = m_metadata.poster( );

		if ( m_poster_buffer.empty( ) )
			m_poster = Internal::get_default_poster( );

		else
			m_poster = Internal::make_sdl_surface( m_poster_buffer );
	}

	Api::c_film m_metadata;
	Poster m_poster;
	std::vector< u8 > m_poster_buffer;
};

class c_library
{
public:
	void init( )
	{
		// if we've already build a library file
		if ( has_library_file( ) )
			load_from_file( );
	}

	void add( Api::c_film& data )
	{
		// try to handle duplicates
		for ( auto&& entry : m_entries )
		{
			auto title = data.warez( ).keys.find( "Title" );
			auto year = data.warez( ).keys.find( "Year" );

			if ( title == data.warez( ).keys.end( ) )
				return;

			const std::string entry_title = entry.metadata( ).find( "Title" );

			if ( title->second == entry_title )
			{
				if ( year != data.warez( ).keys.end( ) )
				{
					if ( title->second == entry_title && year->second == entry.metadata( ).find( "Year" ) )
						return;
				}

				else
					return;
			}
		}

		if ( !data.valid( ) )
			data = Api::query( data );

		if ( data.valid( ) && !data.find( "Title" ).empty( ) )
		{
			m_entries.push_back( c_entry( data ) );
			m_entries.back( ).init( );
		}
	}

	bool has_library_file( )
	{
		std::ifstream file( "library.json", std::ifstream::in );
		const bool ret = file.good( ) && file.peek( ) != std::ifstream::traits_type::eof( );
		file.close( );
		return ret;
	}

	void load_from_file( )
	{
		std::ifstream file( "library.json" );
		json library;
		
		file >> library;

		for ( auto&& entry : library.back( ) )
		{
			m_entries.push_back( c_entry( entry ) );
			m_entries.back( ).init( );
		}

		file.close( );
	}

	void save_to_file( )
	{
		std::ofstream file( "library.json" );

		json array;
		json objects;

		for ( u32 i = 0; i < m_entries.size( ); ++i )		
			objects[i] = m_entries[i].response( );		

		array["library"] = objects;

		file << array.dump( 4 );
		file.close( );
	}

	void parse_disk( const std::experimental::filesystem::v1::path& path, bool recursively, const std::experimental::filesystem::v1::path& parent = {} )
	{
		const auto handle_entry = [&]( const std::experimental::filesystem::v1::directory_entry& file )
		{
			Api::c_film film( file );

			if ( !film.warez( ).valid( ) )
				return;

			add( film );			
		};

		for ( auto& file : std::experimental::filesystem::directory_iterator( path ) )
		{
			if ( std::experimental::filesystem::is_empty( file ) )
				continue;

			// directory isn't empty & has children directories, iterate them
			else if ( std::experimental::filesystem::is_directory( file ) && parent != file )
				parse_disk( file, recursively, file );

			if ( file.path( ).has_extension( ) && Internal::has_video_extension( file.path( ).extension( ).generic_string( ) ) )			
				handle_entry( file );			
		}
	}

	void load_from_path( )
	{
	#ifdef _WIN32
		IFileDialog*	dialog;
		IShellItem*		item;
		unsigned long	options;
		wchar_t*		path = 0;
		
		if ( CoCreateInstance( CLSID_FileOpenDialog, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &dialog ) ) >= 0 )
		{
			if ( dialog && dialog->GetOptions( &options ) >= 0 )
			{
				dialog->SetOptions( options | FOS_PICKFOLDERS );

				if ( dialog->Show( 0 ) >= 0 )
				{
					if ( dialog->GetResult( &item ) >= 0 && item )
					{
						if ( item->GetDisplayName( SIGDN_DESKTOPABSOLUTEPARSING, &path ) >= 0 )
							parse_disk( std::experimental::filesystem::path( path ), true );

						item->Release( );
					}
				}

				dialog->Release( );
			}
		}
	#endif
	}

	Poster get_poster_from_entry( c_entry& entry )
	{
		const std::string index = entry.metadata( ).find( "imdbID" );

		if ( m_postermap.find( index ) == m_postermap.end( ) )
			m_postermap[index] = entry.poster( );

		return m_postermap[index];
	}

	std::vector< c_entry > entries( ) const
	{
		return m_entries;
	}

private:
	std::vector< c_entry > m_entries;
	std::unordered_map< std::string, Poster > m_postermap;
};

inline c_library g_library;