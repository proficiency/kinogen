#pragma once

class c_interface_imgui
{
public:
	void init( )
	{
		SDL_SetHint( SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0" );
		SDL_Init( SDL_INIT_VIDEO );
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
		SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
		m_window = SDL_CreateWindow( "kinogen",
									 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
									 m_size[0], m_size[1], SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI );

		m_gl_ctx = SDL_GL_CreateContext( m_window );
		SDL_GetWindowSize( m_window, &m_size[0], &m_size[1] );
		SDL_GL_MakeCurrent( m_window, m_gl_ctx );
		SDL_GL_SetSwapInterval( 1 );

		IMGUI_CHECKVERSION( );
		ImGui::CreateContext( );
		ImGuiIO& io = ImGui::GetIO( );;
		{
			ImGui::StyleColorsDark( );
		}

		ImGui_ImplSDL2_InitForOpenGL( m_window, m_gl_ctx );
		ImGui_ImplOpenGL2_Init( );

		/* Load Fonts: if none of these are loaded a default font will be used  */
		/* Load Cursor: if you uncomment cursor loading please hide the cursor */
		{
			m_roboto = io.Fonts->AddFontFromFileTTF( "./assets/Roboto-Regular.ttf", 16.0f, 0, io.Fonts->GetGlyphRangesDefault( ) );
			m_roboto_light = io.Fonts->AddFontFromFileTTF( "./assets/Roboto-Thin.ttf", 18.0f, 0, io.Fonts->GetGlyphRangesDefault( ) );
			m_roboto_medium = io.Fonts->AddFontFromFileTTF( "./assets/Roboto-Regular.ttf", 18.0f, 0, io.Fonts->GetGlyphRangesDefault( ) );
			m_roboto_large = io.Fonts->AddFontFromFileTTF( "./assets/Roboto-Regular.ttf", 24.0f, 0, io.Fonts->GetGlyphRangesDefault( ) );
			
			ImGuiStyle* style = &ImGui::GetStyle( );

			ImGui::StyleColorsDark( style );

			style->WindowBorderSize = 0.0f;
			style->WindowPadding = ImVec2( 15, 15 );
			style->WindowRounding = 0.0f;
			//style->FramePadding = ImVec2( 5, 5 );
			//style->FrameRounding = 5.0f;
			//style->ItemSpacing = ImVec2( 3, 8 );
			//style->ItemInnerSpacing = ImVec2( 8, 6 );
			//style->IndentSpacing = 25.0f;
			style->ScrollbarSize = 10.0f;
			style->ScrollbarRounding = 9.0f;
			//style->GrabMinSize = 5.0f;
			//style->GrabRounding = 3.0f;

			style->Colors[ImGuiCol_Text] = { 0.95f, 0.95f, 0.95f, 0.97f };

			const ImVec4 base = { 0.98f, 0.784f, 1.0f, 0.75f };
			{
				style->Colors[ImGuiCol_Button] = { base.x * 0.75f, base.y * 0.75f, base.z * 0.75f, base.w };
				style->Colors[ImGuiCol_ButtonHovered] = base;
				style->Colors[ImGuiCol_ButtonActive] = { base.x * 0.9f, base.y * 0.9f, base.z * 0.9f, base.w };

				style->Colors[ImGuiCol_Header] = style->Colors[ImGuiCol_Button];
				style->Colors[ImGuiCol_HeaderHovered] = style->Colors[ImGuiCol_ButtonHovered];
				style->Colors[ImGuiCol_HeaderActive] = style->Colors[ImGuiCol_ButtonActive];

				style->Colors[ImGuiCol_FrameBg] = { base.x * 0.75f, base.y * 0.75f, base.z * 0.75f, 0.45f };
				//style->Colors[ImGuiCol_FrameBgHovered] = style->Colors[ImGuiCol_ButtonHovered];
				//style->Colors[ImGuiCol_FrameBgActive] = style->Colors[ImGuiCol_ButtonActive];
			}

			const ImVec4 bg_base = style->Colors[ImGuiCol_WindowBg];
			style->Colors[ImGuiCol_MenuBarBg] = { bg_base.x * 0.95f, bg_base.y * 0.95f, bg_base.z * 0.95f, 1.0f };
		}

		frame( );
	}

	bool frame( )
	{
		SDL_Event event;
		while ( SDL_PollEvent( &event ) )
		{
			ImGui_ImplSDL2_ProcessEvent( &event );
			if ( event.type == SDL_QUIT )
				return false;
		}

		ImGui_ImplOpenGL2_NewFrame( );
		ImGui_ImplSDL2_NewFrame( m_window );
		ImGui::NewFrame( );

		ImGui::SetNextWindowPos( ImVec2( -3, 0 ) );
		ImGui::SetNextWindowSize( ImVec2( ( 1280 / 3 ) + 3, 960 ) ); // 960 TOTAL

		static int selected = -1;
		auto library = g_library.entries( );

		if ( ImGui::Begin( "##kinogen", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysVerticalScrollbar ) )
		{
			if ( ImGui::BeginMenuBar( ) )
			{
				auto&& io = ImGui::GetIO( );
				bool save = io.KeyCtrl && io.KeysDown[22];
				bool load = io.KeyCtrl && io.KeysDown[18];

				if ( ImGui::BeginMenu( "library" ) )
				{
					if ( ImGui::MenuItem( "save", "ctrl + s" ) )
						save = true;

					if ( ImGui::MenuItem( "load", "ctrl + o" ) )
						load = true;

					// todo
					//if ( ImGui::MenuItem( "import library" ) )

					ImGui::EndMenu( );
				}
				
			#ifdef _DEBUG
				if ( ImGui::BeginMenu( std::to_string( ( int ) std::ceil( ImGui::GetIO( ).Framerate ) ).c_str( ) ) )				
					ImGui::EndMenu( );				
			#endif

				ImGui::EndMenuBar( );
				
				if ( save )
					g_library.save_to_file( );

				if ( load )
					g_library.load_from_path( );
			}

			if ( library.empty( ) )
			{
				ImGui::Text( "the library is empty... " );
				
				ImGui::SameLine( );

				if ( ImGui::Selectable( "search folder?" ) )
					g_library.load_from_path( );
			}
			
			else
			{
				ImGui::Columns( 1 );

				static ImGuiTextFilter filter;
				static char buffer[256]{};

				if ( ImGui::InputTextWithHint( "", "...", buffer, 256, ImGuiInputTextFlags_None ) )
				{
					memcpy( filter.InputBuf, buffer, sizeof( filter.InputBuf ) );
					filter.Build( );
				}

				for ( u32 i = 0; i < library.size( ); ++i )
				{
					auto&& entry = library.at( i );
					auto&& metadata = entry.metadata( );

					const std::string title = metadata.find( "Title" );
					const std::string director = metadata.find( "Director" );
					const std::string genre = metadata.find( "Genre" );
					const std::string year = metadata.find( "Year" );
					const std::string cast = metadata.find( "Actors" );

					if ( !filter.PassFilter( title.c_str( ) ) && !filter.PassFilter( director.c_str( ) ) && !filter.PassFilter( genre.c_str( ) ) && !filter.PassFilter( year.c_str( ) ) && !filter.PassFilter( cast.c_str( ) ) )
						continue;

					// some textures(default for example) will have the same id
					ImGui::PushID( i );
					if ( ImGui::ImageButton( ( ImTextureID ) g_library.get_poster_from_entry( entry ).second, { 184, 276 } ) )
						selected = i;
					ImGui::PopID( );

					ImGui::SameLine( );
					ImGui::TextWrapped( "%s\n%s | %s | %s", title.c_str( ), year.c_str( ), metadata.find( "Rated" ).c_str( ), metadata.find( "Runtime" ).c_str( ) );
				}
			}
		}
		ImGui::End( );

		ImGui::SetNextWindowPos( ImVec2( ( 1280 / 3 ), 0 ) );
		ImGui::SetNextWindowSize( ImVec2( ( ( 1280 / 3 ) * 2 ) + 3, 960 ) );

		if ( ImGui::Begin( "##kinogen_description", 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration ) )
		{
			if ( selected != -1 )
			{
				auto&& entry = library.at( selected );
				auto&& metadata = entry.metadata( );

				if ( metadata.valid( ) )
				{
					if ( ImGui::BeginMenuBar( ) )
					{
						static bool pressed_play = false;
						if ( ImGui::BeginMenu( "play" ) )
						{
							if ( !pressed_play )
							{
								Internal::open_in_media_player( entry.path( ) );
								pressed_play = true;
							}

							ImGui::EndMenu( );
						}

						else
							pressed_play = false;

						static bool pressed_imdb = false;
						if ( ImGui::BeginMenu( "imdb" ) )
						{
							if ( !pressed_imdb )
							{
								Internal::open_on_imdb( entry.response( ) );
								pressed_imdb = true;
							}

							ImGui::EndMenu( );
						}

						else
							pressed_imdb = false;

						ImGui::EndMenuBar( );
					}

					ImGui::PushFont( m_roboto_large );
					ImGui::Text( metadata.find( "Title" ).c_str( ) );
					ImGui::PopFont( );
					ImGui::Separator( );

					ImGui::PushFont( m_roboto_medium );
					ImGui::Text( metadata.find( "Genre" ).c_str( ) );
					ImGui::Text( "Released on %s", metadata.find( "Released" ).c_str( ) );
					ImGui::Text( "Directed by: %s", metadata.find( "Director" ).c_str( ) );
					ImGui::Text( "Written by: %s", metadata.find( "Writer" ).c_str( ) );
					ImGui::Text( "Starring: %s", metadata.find( "Actors" ).c_str( ) );
					ImGui::Text( "Produced by %s in %s", metadata.find( "Production" ).c_str( ), metadata.find( "Country" ).c_str( ) );
					ImGui::Text( "%s", metadata.find( "Awards" ).c_str( ) );

					ImGui::Text( "" );
					ImGui::Text( "IMDb: %s, %s votes", metadata.find( "imdbRating" ).c_str( ), metadata.find( "imdbVotes" ).c_str( ) );

					const auto response = entry.response( );
					for ( auto& rating : response["Ratings"] )					
						ImGui::Text( "%s: %s\n", rating["Source"].get < std::string >( ).c_str( ), rating["Value"].get < std::string >( ).c_str( ) );
					
					ImGui::Text( "" );
					ImGui::TextWrapped( metadata.find( "Plot" ).c_str( ) );
					ImGui::PopFont( );
				}
			}
		}
		ImGui::End( );

		ImGui::Render( );
		glViewport( 0, 0, ( int ) ImGui::GetIO( ).DisplaySize.x, ( int ) ImGui::GetIO( ).DisplaySize.y );
		glClearColor( 0.45f, 0.55f, 0.60f, 0.0f );
		glClear( GL_COLOR_BUFFER_BIT );
		ImGui_ImplOpenGL2_RenderDrawData( ImGui::GetDrawData( ) );
		SDL_GL_SwapWindow( m_window );

		return true;
	}

	void release( )
	{
		ImGui_ImplOpenGL2_Shutdown( );
		ImGui_ImplSDL2_Shutdown( );
		ImGui::DestroyContext( );

		SDL_GL_DeleteContext( m_gl_ctx );
		SDL_DestroyWindow( m_window );
		SDL_Quit( );
	}

private:
	SDL_Window* m_window;
	SDL_GLContext m_gl_ctx;
	int m_size[2] = { 1280, 960 };

	ImFont* m_roboto;
	ImFont* m_roboto_medium;
	ImFont* m_roboto_large;
	ImFont* m_roboto_light;
};
inline c_interface_imgui g_interface_imgui;