if( NOT TARGET Cinder-RtAudio )
	get_filename_component( CINDER_RT_AUDIO_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../src" ABSOLUTE )
	get_filename_component( CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE )

	include( ${CMAKE_CURRENT_LIST_DIR}/modules/RtAudioConfig.cmake )
	
	add_library( Cinder-RtAudio ${CINDER_RT_AUDIO_SOURCE_PATH}/CinderRtAudio.cpp )
	add_dependencies( Cinder-RtAudio rtaudio_static )

	target_include_directories( Cinder-RtAudio PUBLIC "${CINDER_RT_AUDIO_SOURCE_PATH}" )
	target_include_directories( Cinder-RtAudio SYSTEM BEFORE PUBLIC "${CINDER_PATH}/include" )

	if( NOT TARGET cinder )
		    include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
		    find_package( cinder REQUIRED PATHS
		        "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
		        "$ENV{CINDER_PATH}/${CINDER_LIB_DIRECTORY}" )
	endif()
	target_link_libraries( Cinder-RtAudio PRIVATE cinder rtaudio_static )
	
endif()
