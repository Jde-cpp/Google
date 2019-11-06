cmake_minimum_required (VERSION 3.10)
set( CMAKE_MODULE_PATH "../../../../modules" )
include(cotire)
project (Jde.IO.Drive.Google)

#add_custom_target(pch COMMAND ./pc.h -o ${CMAKE_CURRENT_BINARY_DIR}/stdafx.h.gch )
add_executable(Jde.IO.Drive.Google GoogleDrive.cpp GoogleApi.cpp)
add_dependencies(Jde.IO.Drive.Google pch)