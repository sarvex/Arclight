﻿cmake_minimum_required (VERSION 3.21)

set(PROJECT_NAME arclight)
project (${PROJECT_NAME})


#######################
#### PROJECT SETUP ####
#######################

	# Define Debug/Release
	add_compile_definitions(ARC_DEBUG=$<CONFIG:Debug>)
	add_compile_definitions(ARC_RELEASE=$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>)

	# Enable C++23
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_CXX_EXTENSIONS off)
	set(CMAKE_CXX_STANDARD_REQUIRED true)

	# Enable C99
	set(CMAKE_C_STANDARD 99)
	set(CMAKE_C_EXTENSIONS off)
	set(CMAKE_C_STANDARD_REQUIRED true)

	# Set output directories
	set(BINARY_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR})

	### URI TRANSITION
	# Set root path
	add_compile_definitions(ARC_PATH_ROOT="../../")

	message("Building '${PROJECT_NAME}'")


######################
#### VECTOR SETUP ####
######################

	set(BUILDSYS_FILENAME ${CMAKE_CURRENT_SOURCE_DIR}/buildsys.txt)

	if(EXISTS ${BUILDSYS_FILENAME})

		file(STRINGS buildsys.txt BUILDSYS_STRINGS)
		list(GET BUILDSYS_STRINGS 0 SUPPORTED_CPU_EXTENSION_STRING)
		string(REPLACE "," ";" SUPPORTED_CPU_EXTENSIONS ${SUPPORTED_CPU_EXTENSION_STRING})

		set(ARC_SUPPORTED_CPU_EXTENSIONS "sse" "sse2" "sse3" "ssse3" "sse4_1" "sse4_2" "avx" "avx2")
		set(ARC_CPU_EXTENSION_DEFINITIONS ARC_TARGET_HAS_SSE ARC_TARGET_HAS_SSE2 ARC_TARGET_HAS_SSE3 ARC_TARGET_HAS_SSSE3 ARC_TARGET_HAS_SSE4_1 ARC_TARGET_HAS_SSE4_2 ARC_TARGET_HAS_AVX ARC_TARGET_HAS_AVX2)

		list(LENGTH ARC_SUPPORTED_CPU_EXTENSIONS ARC_SUPPORTED_CPU_EXTENSION_COUNT)
		math(EXPR ARC_SUPPORTED_CPU_EXTENSION_COUNT "${ARC_SUPPORTED_CPU_EXTENSION_COUNT} - 1")

		foreach(i RANGE ${ARC_SUPPORTED_CPU_EXTENSION_COUNT})

			list(GET ARC_SUPPORTED_CPU_EXTENSIONS ${i} EXTENSION)

			if(EXTENSION IN_LIST SUPPORTED_CPU_EXTENSIONS)

				list(GET ARC_CPU_EXTENSION_DEFINITIONS ${i} EXTENSION_DEFINITION)
				add_compile_definitions(${EXTENSION_DEFINITION})
				list(APPEND BUILD_CPU_EXTENSIONS ${EXTENSION})

			endif()

		endforeach()

	endif()

	if(BUILD_CPU_EXTENSIONS)
		message("Enabled CPU extensions: ${BUILD_CPU_EXTENSIONS}")
	else()
		message("Enabled CPU extensions: none")
	endif()


######################
### COMPILER SETUP ###
######################

	if(MSVC)

		if("avx2" IN_LIST BUILD_CPU_EXTENSIONS)
			set(MSVC_ARCH_FLAGS "/arch:AVX2")
		elseif("avx" IN_LIST BUILD_CPU_EXTENSIONS)
			set(MSVC_ARCH_FLAGS "/arch:AVX")
		endif()

		set(MSVC_CXX_BASE_FLAGS "${MSVC_ARCH_FLAGS} /MP /permissive /utf-8 /DWIN32 /D_WINDOWS /EHsc")
		set(MSVC_C_BASE_FLAGS 	"${MSVC_ARCH_FLAGS} /MP /permissive /utf-8 /DWIN32 /D_WINDOWS")

		set(CMAKE_CXX_FLAGS_DEBUG 			"${MSVC_CXX_BASE_FLAGS} /Zi /Ob0 /Od /RTC1")
		set(CMAKE_CXX_FLAGS_RELEASE 		"${MSVC_CXX_BASE_FLAGS} /GS- /O2 /Ob3 /DNDEBUG")
		set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 	"${MSVC_CXX_BASE_FLAGS} /GS- /Zi /O2 /Ob3 /DNDEBUG")
		set(CMAKE_CXX_FLAGS_MINSIZEREL 		"${MSVC_CXX_BASE_FLAGS} /GS- /O1 /Ob1 /DNDEBUG")
		set(CMAKE_C_FLAGS_DEBUG 			${CMAKE_CXX_FLAGS_DEBUG})
		set(CMAKE_C_FLAGS_RELEASE 			${CMAKE_CXX_FLAGS_RELEASE})
		set(CMAKE_C_FLAGS_RELWITHDEBINFO 	${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
		set(CMAKE_C_FLAGS_MINSIZEREL 		${CMAKE_CXX_FLAGS_MINSIZEREL})

		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /MANIFESTUAC:NO")

		add_compile_options($<$<CONFIG:>:/MT>$<$<CONFIG:Debug>:/MTd>$<$<CONFIG:Release>:/MT>)

	endif()


#######################
#### PLATFORM SCAN ####
#######################

	if(WIN32)
		list(APPEND ARCLIGHT_PLATFORMS "win32")
	elseif(UNIX)
		list(APPEND ARCLIGHT_PLATFORMS "linux")
	endif()

	list(APPEND ARCLIGHT_PLATFORMS "desktop")

	message("Compiling for the following targets:")

	foreach(Platform ${ARCLIGHT_PLATFORMS})
		message("\t ${Platform}")
	endforeach()


######################
### MODULE PARSING ###
######################

	set(MODULE_FILENAME ${CMAKE_CURRENT_SOURCE_DIR}/arclight.modules)

	if(EXISTS ${MODULE_FILENAME})

		file(STRINGS arclight.modules MODULE_LIST)

		# Remove comments
		list(FILTER MODULE_LIST EXCLUDE REGEX "^#")

		foreach(ModulePath ${MODULE_LIST})

			string(REPLACE "." "/" ModulePath ${ModulePath})

			if(ModulePath MATCHES "^arclight")
				string(SUBSTRING ${ModulePath} 8 -1 ModulePath)
			endif()

			list(APPEND MODULE_PATHS ${ModulePath})

		endforeach()

		message("Following modules will be activated:")

		foreach(Module ${MODULE_LIST})
			message("\t ${Module}")
		endforeach()

	else()

		message(FATAL_ERROR "Failed to find module file ${MODULE_FILENAME}")

	endif()


######################
### MODULE LOADING ###
######################

	set(ARCLIGHT_MODULE_CORE_PATH "src/arclight/core/")
	set(ARCLIGHT_MODULE_PLATFORM_PATH "src/arclight/platform/")
	set(APPLICATION_MAIN_PATH "src/main/")

	# Includes
	include_directories("${APPLICATION_MAIN_PATH}")
	include_directories("${ARCLIGHT_MODULE_CORE_PATH}")

	foreach(Platform ${ARCLIGHT_PLATFORMS})
		file(GLOB SOURCES RELATIVE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${ARCLIGHT_MODULE_PLATFORM_PATH}${Platform}/*.cpp)
		list(APPEND APPLICATION_SOURCES ${SOURCES})
		include_directories(${ARCLIGHT_MODULE_PLATFORM_PATH}${Platform})
	endforeach()

	# Sources
	file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${APPLICATION_MAIN_PATH}/*.cpp)
	list(APPEND APPLICATION_SOURCES ${SOURCES})
	file(GLOB SOURCES RELATIVE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${ARCLIGHT_MODULE_CORE_PATH}/*.cpp)
	list(APPEND APPLICATION_SOURCES ${SOURCES})

	foreach(ModulePath ${MODULE_PATHS})

		file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${ARCLIGHT_MODULE_CORE_PATH}/${ModulePath}/*.cpp)
		list(APPEND APPLICATION_SOURCES ${SOURCES})

		foreach(Platform ${ARCLIGHT_PLATFORMS})
			file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${ARCLIGHT_MODULE_PLATFORM_PATH}/${Platform}/${ModulePath}/*.cpp)
			list(APPEND APPLICATION_SOURCES ${SOURCES})
		endforeach()

	endforeach()


#######################
##### MODULE DEPS #####
#######################

	message("Looking for dependencies")

	# GLFW for input/window
	if ("arclight.input" IN_LIST MODULE_LIST OR "arclight.window" IN_LIST MODULE_LIST)

		# Build GLFW along the project
		set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
		set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
		set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
		add_subdirectory("lib/GLFW")

		include_directories("lib/GLFW/include")
		list(APPEND APPLICATION_LIBS glfw)

		add_compile_definitions(ARC_WINDOW_MODULE)

		message("\tAdded dependency 'GLFW'")

	endif()

	# GLEW for window/render
	if("arclight.window" IN_LIST MODULE_LIST OR "arclight.render" IN_LIST MODULE_LIST OR "arclight.render.gle" IN_LIST MODULE_LIST)

		# Find GLEW
		list(APPEND CMAKE_PREFIX_PATH "lib/GLEW")
		find_package(GLEW REQUIRED)

		include_directories(${GLEW_INCLUDE_DIR})

		if (WIN32)
			list(APPEND APPLICATION_LIBS opengl32)
		elseif(UNIX)
			list(APPEND APPLICATION_LIBS GL)
		endif()

		list(APPEND APPLICATION_LIBS GLEW::GLEW)

		add_compile_definitions(GLEW_NO_GLU)

		message("\tAdded dependency 'GLEW'")

	endif()

	# FreeType for font
	if("arclight.font" IN_LIST MODULE_LIST)

		# Use the correct .lib
		set(FREETYPE_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeType/bin/x64/$<IF:$<CONFIG:Debug>,DebugStatic,ReleaseStatic>/freetype.lib)
		add_compile_definitions(ARC_FONT_MODULE)

		include_directories("lib/FreeType/include")
		list(APPEND APPLICATION_LIBS ${FREETYPE_LIBRARY})

		message("\tAdded dependency 'freetype'")

	endif()

	# Bullet for physics
	if("arclight.physics" IN_LIST MODULE_LIST)

		# Use the correct bullet .libs
		set(BULLET_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/bullet3/lib/$<IF:$<CONFIG:Debug>,Debug,Release>)
		set(BULLET_LIBRARIES ${BULLET_LIB_DIR}/BulletCollision_Debug.lib ${BULLET_LIB_DIR}/BulletDynamics$<$<CONFIG:Debug>,_Debug>.lib ${BULLET_LIB_DIR}/LinearMath$<$<CONFIG:Debug>,_Debug>.lib)

		include_directories("lib/bullet3/include/bullet")
		list(APPEND APPLICATION_LIBS ${BULLET_LIBRARIES})

		message("\tAdded dependency 'Bullet'")

	endif()

	# Assimp and stb for render.model
	if("arclight.render" IN_LIST MODULE_LIST OR "arclight.render.model" IN_LIST MODULE_LIST)

		# Use the correct assimp .lib
		set(ASSIMP_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp/lib/$<IF:$<CONFIG:Debug>,Debug/assimp-vc142-mtd.lib,Release/assimp-vc142-mt.lib>)
		
		include_directories("lib/assimp/include")
		list(APPEND APPLICATION_LIBS ${ASSIMP_LIBRARY})

		# stb_image
		include_directories("lib/stb_image")

		message("\tAdded dependency 'Assimp'")
		message("\tAdded dependency 'stb_image'")

	endif()

	# ImGui for render.gui
	if("arclight.render" IN_LIST MODULE_LIST OR "arclight.render.gui" IN_LIST MODULE_LIST)

		if(${ARC_BUILD_IMGUI})
			add_compile_definitions(ARC_USE_IMGUI=1)

			file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "lib/imgui/source/*.cpp" "lib/imgui/backends/*.cpp")
			list(APPEND APPLICATION_SOURCES ${SOURCES})
		else()
			add_compile_definitions(ARC_USE_IMGUI=0)
		endif()

		include_directories("lib/imgui/include")
		include_directories("lib/imgui/backends")

		message("\tAdded dependency 'ImGui'")

	endif()

	# FMOD for sound
	if("arclight.sound" IN_LIST MODULE_LIST)

		include_directories("lib/FMOD/include")

		list(APPEND APPLICATION_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/lib/FMOD/fmodL_vc.lib)

		message("\tAdded dependency 'FMOD'")

	endif()


######################
###### PLATFORM ######
######################

	set(WIN_NO_CONSOLE FALSE)

	if(WIN32)

		# We only support windows icons yet
		list(APPEND APPLICATION_SOURCES "win_resource.rc" "arclight.exe.manifest")

		# Force Unicode
		add_compile_definitions(UNICODE _UNICODE)

		# Add libraries
		list(APPEND APPLICATION_LIBS ComCtl32.lib Bcrypt.lib)

		# Disables the console
		if(WIN_NO_CONSOLE)

			add_compile_definitions(ARC_WIN_DISABLE_CONSOLE)

			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
			add_executable (${PROJECT_NAME} WIN32 ${APPLICATION_SOURCES})

		else()

			add_executable (${PROJECT_NAME} ${APPLICATION_SOURCES})

		endif()

	else()

		# Create the executable and link
		add_executable (${PROJECT_NAME} ${APPLICATION_SOURCES})

	endif()

######################
#### FINALIZATION ####
######################

	# Link libraries
	target_link_libraries(${PROJECT_NAME} ${APPLICATION_LIBS})