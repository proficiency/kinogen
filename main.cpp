#include "context.h"

int main( )
{
	Networking::curl.setup( );
	g_interface_imgui.init( );

	g_library.init( );

	while ( true )
	{
		if ( !g_interface_imgui.frame( ) )
			break;
	}
	
	g_interface_imgui.release( );
}