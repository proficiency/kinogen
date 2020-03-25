#pragma once
#define OMDB_API_KEY "fe36d3db"
#define OMDB_API_URL "http://www.omdbapi.com/?apikey=" 

// todo: refactor all this later

namespace Api
{
	class c_film
	{
	public:
		// https://en.wikipedia.org/wiki/Standard_(warez)#Naming
		struct Warez
		{
			std::string filename;
			std::string path;
			std::map< std::string, std::string > keys;

			Warez( const std::experimental::filesystem::path& p )
			{
				filename = p.filename( ).generic_string( );
				path = p.generic_string( );

				fill_regex( filename );
			}

			Warez( ) = default;

			inline bool valid( ) const
			{
				return keys.size( ) > 1;
			}

			inline void fill_regex( const std::string filename )
			{
				std::vector< std::pair< std::string, std::regex > > token_regexes =
				{
					{ "Year", std::regex( "(?!^)[1,2]\\d{3}" ) },
					{ "Source", std::regex( "\\d{3,4}p", std::regex_constants::icase ) }, // source(1080p, 720p, etc)
					{ "Audio Codec", std::regex( "AAC2[\\.\s]0|AAC|AC3|DTS|DD5[\\.\s]1", std::regex_constants::icase ) }, // audio codec
					{ "Group", std::regex( "[A-Za-z0-9]+$" ) }, // group, last token				
				};

				std::smatch matches;

				for ( u32 i = 0; i < token_regexes.size( ); ++i )
				{
					const auto& regex = token_regexes[i];

					if ( std::regex_search( filename, matches, regex.second ) )
					{
						for ( auto&& match : matches )
						{
							if ( !i )
							{
								const u32 pos = filename.find( match.str( ) );

								if ( pos )
								{
									std::string title_sanitized = filename.substr( 0, pos - 1 );
									std::replace( title_sanitized.begin( ), title_sanitized.end( ), '.', ' ' );
									
									while ( !title_sanitized.empty( ) && iswspace( title_sanitized.front( ) ) )
										title_sanitized.erase( title_sanitized.begin( ) );

									while ( !title_sanitized.empty( ) && iswspace( title_sanitized.back( ) ) )
										title_sanitized.erase( title_sanitized.end( ) - 1 );

									keys["Title"] = title_sanitized;
								}
							}

							keys[regex.first] = matches.str( );
						}
					}
				}
			}
		};

		c_film( ) : m_valid( false ) { };
		c_film( const std::experimental::filesystem::path& path ) : m_warez( path )	{ }

		c_film( const json& entry )
		{
			fill( entry );
		}

		c_film fill( const json& page )
		{
			m_response = page;

			if ( m_response.is_object( ) || m_response.is_array( ) )
			{
				for ( auto it = m_response.begin( ); it != m_response.end( ); ++it )
				{
					if ( !it->empty( ) && it->type( ) == json::value_t::string )
						m_keys[it.key( )] = it->get< std::string >( );
				}
			}

			auto path = m_keys.find( "Path" );
			if ( path != m_keys.end( ) && !path->second.empty( ) )
				m_warez.path = path->second;

			else if ( !m_warez.path.empty( ) )
				m_response["Path"] = m_warez.path;

			auto watched = m_keys.find( "Watched" );
			if ( watched == m_keys.end( ) )
				m_response["Watched"] = "";

			m_valid = !m_keys.empty( ) && !m_keys["Title"].empty( );
			return *this;
		}

		bool valid( ) const
		{
			return m_valid;
		}

		std::string find( const std::string& val ) const
		{
			for ( auto&& key : m_keys )
			{
				if ( key.first == val )
					return key.second;
			}

			return {};
		}

		std::vector< u8 > poster( ) const
		{
			auto iter = m_keys.find( "Poster" );

			if ( iter == m_keys.end( ) )
				return {};

			std::string url = iter->second;

			std::vector< u8 > ret;
			Networking::curl.connect( url, &ret );
			return ret;
		}

		json response( ) const
		{
			return m_response;
		}

		std::string path( ) const
		{
			return m_warez.path;
		}

		Warez& warez( )
		{
			return m_warez;
		}

	private:
		std::unordered_map< std::string, std::string > m_keys;
		bool m_valid;
		json m_response;
		Warez m_warez;
	};

	/*static inline void iterate_pages_recursively( std::string_view title, u32& page_nr, std::vector< c_film >& result )
	{
		std::vector< u8 >	buf;
		std::string			url;

		url.resize( 256 );
		std::sprintf( url.data( ), "%s%s&s=%s&page=%i", OMDB_API_URL, OMDB_API_KEY, title.data( ), page_nr++ );

		Networking::curl.connect( url, &buf );

		if ( buf.empty( ) )
			return;

		const json response = json::parse( buf );

		if ( response.empty( ) || response.find( "Error" ) != response.end( ) || response["Response"].get< std::string >( ) != "True" )
			return;

		const std::string result_nr = response["totalResults"].get< std::string >( );
		if ( !result_nr.empty( ) && result.size( ) >= std::atoi( result_nr.c_str( ) ) )
			return;

		if ( response["Search"].is_array( ) )
		{
			const json page = response["Search"];

			for ( auto&& entry : page )
				result.push_back( c_film( page ) );			
		}

		iterate_pages_recursively( title, page_nr, result );
	}

	// queries entire database and returns anything matching title
	static inline std::vector< c_film > query_search( std::string_view title )
	{
		u32 page_nr = 1;
		std::vector< c_film > ret;
		iterate_pages_recursively( title, page_nr, ret );
		return ret;
	}*/

	// queries a single title
	inline json query( std::string title, std::string year = {} )
	{
		std::vector< u8 >	buf;
		std::string			url;

		url.resize( 512 );

		if ( year.empty( ) )
			std::sprintf( url.data( ), "%s%s&t=%s&plot=full", OMDB_API_URL, OMDB_API_KEY, title.data( ) );

		else
			std::sprintf( url.data( ), "%s%s&t=%s&y=%s&plot=full", OMDB_API_URL, OMDB_API_KEY, title.data( ), year.c_str( ) );

		std::replace( url.begin( ), url.end( ), ' ', '+' );

		int resp = Networking::curl.connect( url, &buf );

		if ( resp != 200 || buf.empty( ) )
			return {};

		const json response = json::parse( buf );

		if ( response.empty( ) || response.find( "Error" ) != response.end( ) || response["Response"].get< std::string >( ) != "True" )
			return {};
		
		return response;
	}

	// queries a single title, use this when building from file after populating warez struct
	inline c_film query( c_film& film )
	{
		auto title = film.warez( ).keys.find( "Title" );
		auto year = film.warez( ).keys.find( "Year" );

		if ( title == film.warez( ).keys.end( ) )
			return {};

		const json response = query( title->second, year != film.warez( ).keys.end( ) ? year->second : std::string( ) );
		
		if ( response.empty( ) )
			return {};

		return film.fill( response );
	}

	// query from file
	inline c_film query( const std::experimental::filesystem::path& path )
	{
		c_film film( path );
	
		return query( film );
	}
}
