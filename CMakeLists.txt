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

	set(perkinelmer_srcs src/PerkinElmerInterface.cpp src/PerkinElmerDetInfoCtrlObj.cpp
					src/PerkinElmerSyncCtrlObj.cpp src/PerkinElmerBinCtrlObj.cpp)

	file(STRINGS "VERSION" perkinelmer_ver)
	add_library(limaperkinelmer SHARED ${perkinelmer_srcs})

	target_include_directories(limaperkinelmer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
	target_include_directories(limaperkinelmer PUBLIC "C:/XIS/SDK32")
	
	target_compile_definitions(limaperkinelmer PRIVATE LIBPERKINELMER_EXPORTS)
	
	find_library(PERKIN_LIB_XISL XISL "C:/XIS/SDK32")

	if("${PERKIN_LIB_XISL}" STREQUAL "PERKIN_LIB_XISL-NOTFOUND")
	  message(FATAL_ERROR "perkin elmer : XISL lib not found, you can search it in cmake-gui or try to install it if it isn't here.")
	endif()



	target_link_libraries(limaperkinelmer limacore)
	target_link_libraries(limaperkinelmer ${PERKIN_LIB_XISL})


	set_target_properties(limaperkinelmer PROPERTIES VERSION ${perkinelmer_ver} SOVERSION "1")
    set_target_properties(limaperkinelmer PROPERTIES PREFIX "lib")
    install(TARGETS limaperkinelmer DESTINATION lib)

	if(COMPILE_SIP)
		set(NAME "perkinelmer")
		include(CameraRunSIP)
		install(FILES python/__init__.py DESTINATION "${PYTHON_SITE_PACKAGES_DIR}/Lima/PerkinElmer")
	endif()
else()
	message(FATAL_ERROR "This camera has to be compiled on Windows")
endif()
