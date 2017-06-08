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
	#what will be paths under windows ?
	#FIND_LIBRARY(PERKIN_LIB_NEXUSCPP NexusCPP PathToBeDefine)
	#FIND_LIBRARY(PERKIN_LIB_YAT yat)
	#FIND_LIBRARY(PERKIN_LIB_HDF5 HDF5.dll PathToBeDefine)
	IF("${PERKIN_LIB_XISL}" STREQUAL "PERKIN_LIB_XISL-NOTFOUND")
	  MESSAGE(FATAL_ERROR "perkin elmer : XISL lib not found, you can search it in cmake-gui or try to install it if it isn't here.")
	ENDIF()
	#IF("${PERKIN_LIB_NEXUSCPP}" STREQUAL "PERKIN_LIB_NEXUSCPP-NOTFOUND")
	#  MESSAGE(FATAL_ERROR "perkin elmer : NexusCPP not found, you can search it in cmake-gui or try to install it if it isn't here.")
	#ENDIF()
	#IF("${PERKIN_LIB_YAT}" STREQUAL "PERKIN_LIB_YAT-NOTFOUND")
	#  MESSAGE(FATAL_ERROR "perkin elmer : libyat not found, you can search it in cmake-gui or try to install it if it isn't here.")
	#ENDIF()
	#IF("${PERKIN_LIB_HDF5}" STREQUAL "PERKIN_LIB_HDF5-NOTFOUND")
	#  MESSAGE(FATAL_ERROR "perkin elmer : HDF5.dll not found, you can search it in cmake-gui or try to install it if it isn't here.")
	#ENDIF()


	TARGET_LINK_LIBRARIES(limaperkinelmer limacore)
	TARGET_LINK_LIBRARIES(limaperkinelmer ${PERKIN_LIB_XISL})
	#TARGET_LINK_LIBRARIES(limaperkinelmer ${PERKIN_LIB_NEXUSCPP})
	#TARGET_LINK_LIBRARIES(limaperkinelmer ${PERKIN_LIB_YAT})
	#TARGET_LINK_LIBRARIES(limaperkinelmer ${PERKIN_LIB_HDF5})

	SET_TARGET_PROPERTIES(limaperkinelmer PROPERTIES VERSION ${perkinelmer_ver} SOVERSION "1")
    SET_TARGET_PROPERTIES(limaperkinelmer PROPERTIES PREFIX "lib")
    INSTALL(TARGETS limaperkinelmer DESTINATION lib)

	IF(COMPILE_SIP)
		SET(NAME "perkinelmer")
		SET(INCLUDES)
		FILE(GLOB sipfiles
			 RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/sip"
			 "${CMAKE_CURRENT_SOURCE_DIR}/sip/*.sip")
		FOREACH(sipfile ${sipfiles})
			SET(INCLUDES
	"${INCLUDES}
	%Include ${sipfile}")
		ENDFOREACH()

		CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/sip/limamodules.sip.in sip/limaperkinelmer.sip)
		SET(SIP_CONCAT_PARTS 1)
		SET(SIP_INCLUDES ${SIP_INCLUDES}
			"${CMAKE_SOURCE_DIR}/third-party/Processlib/sip"
			"${CMAKE_BINARY_DIR}/sip/core"
			"${CMAKE_SOURCE_DIR}/third-party/Processlib/tasks/sip"
			"${CMAKE_SOURCE_DIR}/common/sip"
			"${CMAKE_SOURCE_DIR}/hardware/sip"
			"${CMAKE_SOURCE_DIR}/control/sip"
			"${CMAKE_SOURCE_DIR}/control/software_operation/sip"
			"${CMAKE_CURRENT_SOURCE_DIR}/sip")
		set(SIP_DISABLE_FEATURES WITH_CONFIG)
		set(SIP_CHECK_EXC ON)
		ADD_SIP_PYTHON_MODULE(limaperkinelmer ${CMAKE_CURRENT_BINARY_DIR}/sip/limaperkinelmer.sip)
		TARGET_INCLUDE_DIRECTORIES(python_module_limaperkinelmer PRIVATE
								   ${PYTHON_INCLUDE_DIRS}
								   "${CMAKE_SOURCE_DIR}/sip"
								   "${CMAKE_SOURCE_DIR}/sip/core")
		TARGET_LINK_LIBRARIES(python_module_limaperkinelmer limaperkinelmer)

		INSTALL(FILES python/__init__.py DESTINATION "${PYTHON_SITE_PACKAGES_DIR}/Lima/PerkinElmer")
	ENDIF()
ELSE()
	MESSAGE(FATAL_ERROR "This camera has to be compiled on Windows")
ENDIF()
