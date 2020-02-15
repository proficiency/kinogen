#include "context.h"

int main( )
{
	Networking::curl.setup( );

	const char* movies[] = { "dracula prince", "risen from the grave", "scars of dracula 1970", "lifeboat", "star wars", "plan 9 from", "maltese falcon" };

	for ( u32 i = 0; i < ARRAYSIZE( movies ); ++i )
	{
		auto x = Api::query_single( movies[i] );
		printf( "%s (%s)\n  %s\n  %s\n\n", x.find( "Title" ).c_str( ), x.find( "Year" ).c_str( ), x.find( "Director" ).c_str( ), x.find( "Actors" ).c_str( ) );
	}

	system( "pause" );
}