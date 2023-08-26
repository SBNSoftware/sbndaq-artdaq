if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	cet_add_compiler_flags(CXX -Wno-maybe-uninitialized)
endif()
art_make_library(LIBRARY_NAME sbndaq_AFrameUtils
	LIBRARIES
	ROOT::Core
	ROOT::GenVector
	ROOT::Physics	
	)

install_headers()
#install_fhicl()
install_source()
