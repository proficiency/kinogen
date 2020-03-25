#pragma once

#include <cmath>
#include <experimental\filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

#ifdef _WIN32
#pragma comment( lib, "opengl32.lib" )
#pragma comment( lib, "GLu32.lib" )
#pragma comment( lib, "glew32s.lib" )
#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "libeay32.lib" )
#pragma comment( lib, "ssleay32.lib" )
#pragma comment( lib, "SDL2.lib" )
#pragma comment( lib, "crypt32.lib" )
#pragma comment( lib, "Normaliz.lib" )
#pragma comment( lib, "Wldap32.lib" )
#pragma comment( lib, "libcurl.lib" )

#define NOMINMAX
#include <Windows.h>
#include <shobjidl_core.h>
#endif

#include <nlohmann\json.hpp>

#define CURL_STATICLIB
#include <curl.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl2.h>

#include <gl\GLU.h>
#include <gl\GL.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_opengl.h>

#define STB_IMAGE_IMPLEMENTATION    
#include <stb_image.h>

using namespace nlohmann;

#include "types.h"
#include "log.h"
#include "internal.h"
#include "networking.h"
#include "omdb_api.h"
#include "library.h"
#include "interface.h"