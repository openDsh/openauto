set (OMX_DIR /opt/vc/)

find_path(BCM_HOST_INCLUDE_DIR
	bcm_host.h
	PATHS
	${OMX_DIR}
	PATH_SUFFIXES
	include
)

find_path(BCM_HOST_LIB_DIR
	libbcm_host.so
	PATHS
	${OMX_DIR}
	PATH_SUFFIXES
	lib
)

find_path(ILCLIENT_INCLUDE_DIR
	ilclient.h
	PATHS
	${OMX_DIR}
	PATH_SUFFIXES
	src/hello_pi/libs/ilclient
)

find_path(ILCLIENT_LIB_DIR
	libilclient.a
	PATHS
	${OMX_DIR}
	PATH_SUFFIXES
	src/hello_pi/libs/ilclient
)



if (BCM_HOST_INCLUDE_DIR AND ILCLIENT_INCLUDE_DIR AND BCM_HOST_LIB_DIR AND ILCLIENT_LIB_DIR)
	set(libomx_FOUND TRUE)
endif()

if (libomx_FOUND)
	if (NOT libomx_FIND_QUIETLY)
		message(STATUS "Found omx:")
		message(STATUS " - Bcm Host: ${BCM_HOST_INCLUDE_DIR}")
		message(STATUS " - ilclient: ${ILCLIENT_INCLUDE_DIR}")
	endif()

	add_library(omx INTERFACE)

	target_include_directories(omx SYSTEM INTERFACE
		${BCM_HOST_INCLUDE_DIR}
		${ILCLIENT_INCLUDE_DIR}
	)

	target_link_libraries(omx INTERFACE
		${BCM_HOST_LIB_DIR}/libbcm_host.so
		${ILCLIENT_LIB_DIR}/libilclient.a
		${BCM_HOST_LIB_DIR}/libvcos.so
		${BCM_HOST_LIB_DIR}/libvcilcs.a
		${BCM_HOST_LIB_DIR}/libvchiq_arm.so
	)

	target_compile_definitions(omx INTERFACE
		-DUSE_OMX
		-DOMX_SKIP64BIT
		-DRASPBERRYPI3
	)

endif()

