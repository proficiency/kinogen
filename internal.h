#pragma once

namespace Internal
{
	static constexpr std::array< const char*, 41 > video_formats =
	{
		".gifv",
		".gif",
		".drc",

		".ogv",
		".ogg",

		".vob" ,
		".flv" ,
		".flv" ,
		".mkv" ,
		".amv" ,
		".asf" ,
		".rmvb" ,
		".rm" ,
		".yuv" ,
		".wmv" ,
		".mov" ,
		".qt" ,
		".TS" ,
		".M2TS" ,
		".MTS" ,
		".avi" ,
		".mng" ,
		".mp4" ,
		".m4p" ,
		".m4v" ,

		".mpg", ".mp2", ".mpeg", ".mpe", ".mpv",

		".mpg", ".mpeg", ".m2v",

		".svi",
		".3gp",
		".3g2",
		".mxf",
		".roq",
		".nsv",
		".flv", ".f4v" ".f4p" ".f4a" ".f4b"
	};

	inline bool has_video_extension( std::string_view extension )
	{
		for ( auto&& entry : video_formats )
		{
			if ( strstr( extension.data( ), entry ) )
				return true;
		}

		return false;
	}

	inline void open_in_media_player( std::string path )
	{
	#ifdef _WIN32
		// should open the default application associated with the extension or atleast open the context menu asking which application to use
		system( ( '"' + path + '"' ).c_str( ) );
	#endif
	}

	inline void open_on_imdb( const json& response )
	{
		auto iter = response.find( "imdbID" );

		if ( iter != response.end( ) )
		{
			const std::string url = "https://imdb.com/title/" + iter->get< std::string >( );
		#ifdef _WIN32
			ShellExecuteA( 0, 0, url.c_str( ), 0, 0, SW_SHOW );
		#endif
		}
	}

	inline std::vector< u8 > file( const std::experimental::filesystem::path& path )
	{
		std::basic_ifstream< u8 > file( path, std::ios::binary );
		const std::vector< u8 > buffer( std::istreambuf_iterator< u8 >( file ), {} );
		file.close( );
		return buffer;
	}

	inline Poster make_sdl_surface( const std::vector< u8 >& image )
	{
		Poster ret;

		int w, h, c;
		const u8* data = stbi_load_from_memory( image.data( ), image.size( ), &w, &h, &c, STBI_rgb_alpha );

		glGenTextures( 1, &ret.second );
		glBindTexture( GL_TEXTURE_2D, ret.second );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

		SDL_PixelFormat pf;
		pf.palette = 0;
		pf.BitsPerPixel = 32;
		pf.BytesPerPixel = 4;
		pf.Rshift = pf.Rloss = pf.Gloss = pf.Bloss = pf.Aloss = 0;
		pf.Rmask = 0x000000ff;
		pf.Gshift = 8;
		pf.Gmask = 0x0000ff00;
		pf.Bshift = 16;
		pf.Bmask = 0x00ff0000;
		pf.Ashift = 24;
		pf.Amask = 0xff000000;
		ret.first = SDL_ConvertSurface( SDL_CreateRGBSurfaceFrom( ( void* ) data, w, h, 32, w * 4, 0xff, 0xff00, 0xff0000, 0xff000000 ), &pf, SDL_SWSURFACE );

		if ( ret.first )
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, ret.first->w, ret.first->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ret.first->pixels );

		else
			return {};

		return ret;
	}

	inline Poster get_default_poster( )
	{
		static std::vector< u8 > buffer = file( "./assets/poster.png" );
		static Poster poster = make_sdl_surface( buffer );

		return poster;
	}
}