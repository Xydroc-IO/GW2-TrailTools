cmake_minimum_required(VERSION 3.16)
project(GW2-TrailTools VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32 AND NOT CMAKE_CROSSCOMPILING)
	message(WARNING "This is a Windows DLL. Prefer:")
	message(WARNING "  cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake")
endif()

file(GLOB_RECURSE TRAILTOOLS_SRC CONFIGURE_DEPENDS
	src/*.cpp
	src/app/*.cpp
	src/ui/*.cpp
	src/pathing/*.cpp
	src/pathing/packs/*.cpp
	src/pathing/world/*.cpp
	src/trailtools/*.cpp
)

add_library(GW2-TrailTools SHARED
	${TRAILTOOLS_SRC}
	deps/imgui/imgui.cpp
	deps/imgui/imgui_draw.cpp
	deps/imgui/imgui_tables.cpp
	deps/imgui/imgui_widgets.cpp
	deps/miniz/miniz.c
	deps/miniz/miniz_tdef.c
	deps/miniz/miniz_tinfl.c
	deps/miniz/miniz_zip.c
)

target_include_directories(GW2-TrailTools PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/app
	${CMAKE_CURRENT_SOURCE_DIR}/src/ui
	${CMAKE_CURRENT_SOURCE_DIR}/src/trailtools
	${CMAKE_CURRENT_SOURCE_DIR}/src/pathing
	${CMAKE_CURRENT_SOURCE_DIR}/src/pathing/packs
	${CMAKE_CURRENT_SOURCE_DIR}/src/pathing/world
	${CMAKE_CURRENT_SOURCE_DIR}/deps
	${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui
	${CMAKE_CURRENT_SOURCE_DIR}/deps/miniz
)

target_compile_definitions(GW2-TrailTools PRIVATE
	WIN32_LEAN_AND_MEAN
	NOMINMAX
	_CRT_SECURE_NO_WARNINGS
)

if(MINGW)
	target_link_libraries(GW2-TrailTools PRIVATE
		dxgi d3d11 d3dcompiler gdi32 user32 ole32 uuid shell32 comdlg32 advapi32 winhttp crypt32
	)
	target_link_options(GW2-TrailTools PRIVATE
		-static-libgcc
		-static-libstdc++
		-static
		-Wl,--image-base,0x180000000
	)
	set_target_properties(GW2-TrailTools PROPERTIES PREFIX "")
endif()

set_target_properties(GW2-TrailTools PROPERTIES
	OUTPUT_NAME "GW2-TrailTools"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS GW2-TrailTools RUNTIME DESTINATION .)
