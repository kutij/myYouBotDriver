cmake_minimum_required(VERSION 3.15)
project(ContinentalTaskKJ
    DESCRIPTION "ContinentalTaskKJ"
    VERSION 0.0.1
    LANGUAGES CXX)

## ---------------------------------------------------------------------------------
## Set folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# Set MT flags for windows
if(WIN32)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

## ---------------------------------------------------------------------------------
## Names used in the CMake package
set(export_config_name "ContinentalTaskKJ")
set(export_targets ${export_config_name}Targets)
if(WIN32)
	set(export_targets_name ${export_config_name}Targets-${CMAKE_GENERATOR_PLATFORM}-${RUNTIME_INFO})
else()
	set(export_targets_name ${export_config_name}Targets)
endif()

## ---------------------------------------------------------------
## Macro to setup archive targets
macro(install_lib NAME HEADERS SOURCEDIR)
	set(LIBS_TO_INSTALL ${LIBS_TO_INSTALL} "${NAME}" PARENT_SCOPE) #for configuring Config.cmake in installing.cmake
	if(WIN32)
		set_target_properties(${NAME} PROPERTIES
			RELEASE_POSTFIX "-${CMAKE_VS_PLATFORM_TOOLSET}-${CMAKE_GENERATOR_PLATFORM}-${RUNTIME_INFO}"
			RELWITHDEBINFO_POSTFIX "-${CMAKE_VS_PLATFORM_TOOLSET}-${CMAKE_GENERATOR_PLATFORM}-${RUNTIME_INFO}-relwithdebinfo"
			MINSIZEREL_POSTFIX "-${CMAKE_VS_PLATFORM_TOOLSET}-${CMAKE_GENERATOR_PLATFORM}-${RUNTIME_INFO}-minsizerel"
			DEBUG_POSTFIX "-${CMAKE_VS_PLATFORM_TOOLSET}-${CMAKE_GENERATOR_PLATFORM}-${RUNTIME_INFO}-debug")
	endif()
	target_include_directories(${NAME} INTERFACE
		$<BUILD_INTERFACE:${SOURCEDIR}/>
		$<INSTALL_INTERFACE:include>)
	install(TARGETS ${NAME} EXPORT ${export_targets}
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
		INCLUDES DESTINATION include)
	install(FILES ${HEADERS} DESTINATION include)
endmacro(install_lib)

## ---------------------------------------------------------------
add_subdirectory(example)
add_subdirectory(lib)
option(BUILD_TESTS "Build tests." OFF)
if (${BUILD_TESTS})
  enable_testing()
  add_subdirectory(tests)
endif()

## ---------------------------------------------------------------------------------
## Additional install methods:
# Export the targets
install(EXPORT ${export_targets}
  FILE ${export_targets_name}.cmake
  DESTINATION "cmake"
  NAMESPACE ${export_config_name}::)
# Configure config file
install(CODE "
		set(LIBS_TO_INSTALL ${LIBS_TO_INSTALL})
		set(RUNTIME_INFO ${RUNTIME_INFO})
		set(export_config_name ${export_config_name})
		set(export_targets ${export_targets})

		configure_file(\"${CMAKE_CURRENT_LIST_DIR}/cmake/Config.cmake.in\"
		\"${CMAKE_INSTALL_PREFIX}/${export_config_name}Config.cmake\" @ONLY)
	")