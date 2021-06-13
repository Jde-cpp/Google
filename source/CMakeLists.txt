#export CXX=g++-8
cmake_minimum_required(VERSION 3.16)
project(Jde.Google)
set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../Framework/cmake )
include(SetUpConfigurations)
#######################################################
link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
add_library( Jde.Google SHARED GoogleApi.cpp GoogleDrive.cpp DriveTypes.cpp )
#######################################################
target_link_libraries(Jde.Google Threads::Threads)
target_link_libraries(Jde.Google Jde.Ssl)
target_link_libraries(Jde.Google ssl)
target_link_libraries(Jde.Google stdc++fs)
#######################################################
target_precompile_headers(Jde.Google
  PRIVATE
  <jde/TypeDefs.h>
	<boost/crc.hpp>
	<boost/noncopyable.hpp>
	<boost/system/error_code.hpp>
	<spdlog/spdlog.h>
	<spdlog/sinks/basic_file_sink.h>
	<spdlog/fmt/ostr.h>
	"../../Framework/source/DateTime.h"
	<jde/Assert.h>
	<jde/Log.h>
	"../../Framework/source/collections/UnorderedMap.h"
)
