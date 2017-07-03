###########################################################################
# This file is part of LImA, a Library for Image Acquisition
#
#  Copyright (C) : 2009-2017
#  European Synchrotron Radiation Facility
#  CS40220 38043 Grenoble Cedex 9 
#  FRANCE
# 
#  Contact: lima@esrf.fr
# 
#  This is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
# 
#  This software is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################

#HAVE TO BE UNDER WINDOWS
if(WIN32)
	set(NAME "perkinelmer")

	set(${NAME}_srcs src/PerkinElmerInterface.cpp src/PerkinElmerDetInfoCtrlObj.cpp
					src/PerkinElmerSyncCtrlObj.cpp src/PerkinElmerBinCtrlObj.cpp)

	add_library(limap${NAME} SHARED ${${NAME}_srcs})

	target_include_directories(lima${NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
	target_include_directories(lima${NAME} PUBLIC "C:/XIS/SDK32")
	
	target_compile_definitions(lima${NAME} PRIVATE LIBPERKINELMER_EXPORTS)
	
	find_library(PERKIN_XISL_LIB XISL "C:/XIS/SDK32")

	if("${PERKIN_XISL_LIB}" STREQUAL "PERKIN_XISL_LIB-NOTFOUND")
	  message(FATAL_ERROR "${NAME} : XISL lib not found, you can search it in cmake-gui or try to install it if it isn't here.")
	endif()



	target_link_libraries(lima${NAME} limacore)
	target_link_libraries(lima${NAME} ${PERKIN_LIB_XISL})

	limatools_set_library_soversion(lima${NAME} "VERSION")
    set_target_properties(lima${NAME} PROPERTIES PREFIX "lib")
    install(TARGETS lima${NAME} DESTINATION lib)

	if(LIMA_ENABLE_PYTHON)
		limatools_run_sip_for_camera(${NAME})
		install(FILES python/__init__.py DESTINATION "${PYTHON_SITE_PACKAGES_DIR}/Lima/PerkinElmer")
	endif()
else()
	message(FATAL_ERROR "This camera has to be compiled on Windows")
endif()
