#pragma once

namespace Networking
{
	static u32 write_function( char* data, u32 size, u32 nmemb, std::vector< u8 >* writedata )
	{
		const u32 len = size * nmemb;

		for ( u32 i = 0; i < len; ++i )
			writedata->push_back( data[i] );

		return len;
	}

	class c_networking
	{
	public:
		void setup( )
		{
			if ( curl_global_init( CURL_GLOBAL_DEFAULT ) == CURLE_FAILED_INIT )
				return;			

			m_curl = curl_easy_init( );

			if ( !m_curl )
				return;
			

			curl_easy_setopt( m_curl, CURLOPT_WRITEFUNCTION, write_function );
			curl_easy_setopt( m_curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS );
			curl_easy_setopt( m_curl, CURLOPT_POSTREDIR, CURL_REDIR_GET_ALL );			
			curl_easy_setopt( m_curl, CURLOPT_SSL_VERIFYPEER, 0 );
		}

		void release( )
		{
			curl_easy_cleanup( m_curl );
		}

		int connect( std::string_view url, std::vector< u8 >* data )
		{
			int response;

			if ( !data )
				return -1;

			if ( !data->empty( ) )
				data->clear( );

			m_header = curl_slist_append( m_header, "User-Agent: Mozilla/5.0 (X11; U; Linux amd64; rv:5.0) Gecko/20100101 Firefox/5.0 (Debian)" );
			curl_easy_setopt( m_curl, CURLOPT_WRITEDATA, data );
			curl_easy_setopt( m_curl, CURLOPT_URL, url.data( ) );
			curl_easy_setopt( m_curl, CURLOPT_FOLLOWLOCATION, 1 );
			curl_easy_perform( m_curl );
			curl_easy_getinfo( m_curl, CURLINFO_RESPONSE_CODE, &response );

			return response;
		}

		CURL*			m_curl;
		curl_slist*		m_header;
	};

	inline c_networking curl;
}