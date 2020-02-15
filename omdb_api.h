#pragma once
#define OMDB_API_KEY "fe36d3db" // we gotta parse this from a file, 1k limit unless i'm a patron
#define OMDB_API_URL "http://www.omdbapi.com/?apikey=" 

using namespace nlohmann;

namespace Api
{
	class c_metadata
	{
	public:
		c_metadata( ) = default;
		c_metadata( const json& entry )
		{
			fill( entry );
		}

		c_metadata fill( const json& entry )
		{
			m_keys["Title"] = entry["Title"].get< std::string >( );
			m_keys["Year"] = entry["Year"].get< std::string >( );
			m_keys["Rated"] = entry["Rated"].get< std::string >( );
			m_keys["Released"] = entry["Released"].get< std::string >( );
			m_keys["Runtime"] = entry["Runtime"].get< std::string >( );
			m_keys["Genre"] = entry["Genre"].get< std::string >( );
			m_keys["Director"] = entry["Director"].get< std::string >( );
			m_keys["Writer"] = entry["Writer"].get< std::string >( );
			m_keys["Actors"] = entry["Actors"].get< std::string >( );
			m_keys["Plot"] = entry["Plot"].get< std::string >( );
			m_keys["Language"] = entry["Language"].get< std::string >( );
			m_keys["Country"] = entry["Country"].get< std::string >( );
			m_keys["Awards"] = entry["Awards"].get< std::string >( );
			m_keys["Poster"] = entry["Poster"].get< std::string >( );
			m_keys["Metascore"] = entry["Metascore"].get< std::string >( );
			m_keys["imdbRating"] = entry["imdbRating"].get< std::string >( );
			m_keys["imdbID"] = entry["imdbID"].get< std::string >( );
			m_keys["DVD"] = entry["DVD"].get< std::string >( );
			m_keys["BoxOffice"] = entry["BoxOffice"].get< std::string >( );
			m_keys["Production"] = entry["Production"].get< std::string >( );
			m_keys["Website"] = entry["Website"].get< std::string >( );

			return *this;
		}

		// apparently unordered map doesn't support std::string_view
		std::string find( const std::string& val )
		{
			return m_keys[val];
		}

	private:
		std::unordered_map< std::string, std::string > m_keys;
	};

	struct api_request_t
	{
		// while both the imdb_id and title are optional, atleast one must be present in the request

		// * optional * 8 char imdb id
		std::string imdb_id;

		// * optional * movie title
		std::string title;

		// <empty>, movie, series, or episode
		std::string type;

		// release year
		std::string year;

		// length of description, short or full
		std::string plot;

		// json or xml
		std::string response_type;

		// jsonp callback
		std::string callback;

		// always 1
		std::string api_version;
	};

	static inline void iterate_pages_recursively( std::string_view title, u32& page_nr, std::vector< c_metadata >& result )
	{
		std::vector< u8 >	buf;
		std::string			url;

		// shrug
		url.resize( 256 );
		std::sprintf( url.data( ), "%s%s&s=%s&page=%i", OMDB_API_URL, OMDB_API_KEY, title.data( ), page_nr++ );

		Networking::curl.connect( url, &buf );

		if ( buf.empty( ) )
			return;

		const json response = json::parse( buf );

		if ( response.empty( ) || response.find( "Error" ) != response.end( ) || response["Response"].get< std::string >( ) != "True" )
			return;

		// note: this could cause segfault if the last entry in the list is empty
		const std::string result_nr = response["totalResults"].get< std::string >( );
		if ( !result_nr.empty( ) && result.size( ) >= std::atoi( result_nr.c_str( ) ) )
			return;

		if ( response["Search"].is_array( ) )
		{
			const json page = response["Search"];

			for ( auto&& entry : page )
			{
				result.push_back( c_metadata( entry ) );
				printf( "$> %s (%s)\n", entry["Title"].get< std::string >( ).c_str( ), entry["Year"].get< std::string >( ).c_str( ) );
			}
		}

		iterate_pages_recursively( title, page_nr, result );
	}

	// queries a single title and returns a LOT of metadata
	static inline c_metadata query_single( std::string title )
	{
		std::vector< u8 >	buf;
		std::string			url;
		c_metadata			ret;

		// parse year from title (e.g. 1970 from "Scars Of Dracula (1970).mp4")
		std::string year;
		for ( u32 i = 0; i < title.length( ); ++i )
		{
			if ( std::isdigit( title[i] ) && ( std::isdigit( title[std::min( i + 1, ( u32 ) title.length( ) - 1 )] ) || std::isdigit( title[i - 1] ) ) )
				 year.push_back( title[i] );

			if ( year.length( ) == 4 )
			{
				// 1. https://en.wikipedia.org/wiki/List_of_years_in_film lol 2. i won't be alive then so idc
				if ( std::atoi( year.c_str( ) ) < 1888 || std::atoi( year.c_str( ) ) > 2050 )
				{
					year.clear( );
					break;
				}

				else
				{
					u32 pos = title.find( year );

					if ( pos != std::string::npos )
						title.erase( pos, pos + title.length( ) );
					
				}
			}
		}

		if ( year.length( ) != 4 )
			year.clear( );

		/*
			sanitization todo:
			move to own routine
			handle strings that may be in copies of (((dubious) legality like 720p.BluRay.x264.anoXmous_.mp4
		*/
		// shrug
		url.resize( 256 );

		if ( year.empty( ) )
			std::sprintf( url.data( ), "%s%s&t=%s", OMDB_API_URL, OMDB_API_KEY, title.data( ) );

		else
			std::sprintf( url.data( ), "%s%s&t=%s&year=%s", OMDB_API_URL, OMDB_API_KEY, title.data( ), year.c_str( ) );

		std::replace( url.begin( ), url.end( ), ' ', '+' );

		int resp = Networking::curl.connect( url, &buf );

		if ( resp != 200 || buf.empty( ) )
			return {};

		const json response = json::parse( buf );

		if ( response.empty( ) || response.find( "Error" ) != response.end( ) || response["Response"].get< std::string >( ) != "True" )
			return {};
		
		return ret.fill( response );
	}

	// queries entire database and returns anything matching title
	static inline std::vector< c_metadata > query( std::string_view title )
	{
		u32 page_nr = 1;
		std::vector< c_metadata > ret;
		iterate_pages_recursively( title, page_nr, ret );
		printf( "\n$> found %u results\n", ret.size( ) );
		return ret;
	}
}
