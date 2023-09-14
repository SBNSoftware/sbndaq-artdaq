if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	cet_add_compiler_flags(CXX -Wno-maybe-uninitialized)
endif()

#include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

#FILE( GLOB src_files *.cxx )

#cet_make_library( LIBRARY_NAME Aframe_Objects
#                  SOURCE       ${src_files}
#                )

#build_dictionary( sbnanaobj_StandardRecord
#                  DICTIONARY_LIBRARIES sbnanaobj_StandardRecord
#                )

#ROOT_GENERATE_DICTIONARY( Aframe_Objects 
#			  DICTIONARY_LIBRARIES Aframe_Objects
#			)

#ROOT_GENERATE_DICTIONARY(Aframe_dict Event.h Cluster.h AframeTrack.h TrackMaker.h LINKDEF Aframe_linkdef.h)

art_make_library(LIBRARY_NAME sbndaq_AFrameUtils
	LIBRARIES
	ROOT::HistPainter
	ROOT::Core
	ROOT::GenVector
	ROOT::Physics	
	)

install_headers()
#install_fhicl()
install_source()
