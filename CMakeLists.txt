############################################################################
# This file is part of LImA, a Library for Image Acquisition
#
# Copyright (C) : 2009-2017
# European Synchrotron Radiation Facility
# BP 220, Grenoble 38043
# FRANCE
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################
#CMakeLists.txt for Perkin Elmer camera

#HAVE TO BE UNDER WINDOWS
IF(WIN32)

	SET(perkinelmer_srcs src/PerkinElmerInterface.cpp src/PerkinElmerDetInfoCtrlObj.cpp
					src/PerkinElmerSyncCtrlObj.cpp src/PerkinElmerBinCtrlObj.cpp)

	FILE(STRINGS "VERSION" perkinelmer_ver)
	ADD_LIBRARY(limaperkinelmer SHARED ${perkinelmer_srcs})

	TARGET_INCLUDE_DIRECTORIES(limaperkinelmer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
	TARGET_INCLUDE_DIRECTORIES(limaperkinelmer PUBLIC "C:/XIS/SDK32")
	
	TARGET_COMPILE_DEFINITIONS(limaperkinelmer PRIVATE LIBPERKINELMER_EXPORTS)
	
	FIND_LIBRARY(PERKIN_LIB_XISL XISL "C:/XIS/SDK32")

	IF("${PERKIN_LIB_XISL}" STREQUAL "PERKIN_LIB_XISL-NOTFOUND")
	  MESSAGE(FATAL_ERROR "perkin elmer : XISL lib not found, you can search it in cmake-gui or try to install it if it isn't here.")
	ENDIF()



	TARGET_LINK_LIBRARIES(limaperkinelmer limacore)
	TARGET_LINK_LIBRARIES(limaperkinelmer ${PERKIN_LIB_XISL})


	SET_TARGET_PROPERTIES(limaperkinelmer PROPERTIES VERSION ${perkinelmer_ver} SOVERSION "1")
    SET_TARGET_PROPERTIES(limaperkinelmer PROPERTIES PREFIX "lib")
    INSTALL(TARGETS limaperkinelmer DESTINATION lib)

	IF(COMPILE_SIP)
		SET(NAME "perkinelmer")
		include(CameraRunSIP)
		INSTALL(FILES python/__init__.py DESTINATION "${PYTHON_SITE_PACKAGES_DIR}/Lima/PerkinElmer")
	ENDIF()
ELSE()
	MESSAGE(FATAL_ERROR "This camera has to be compiled on Windows")
ENDIF()
