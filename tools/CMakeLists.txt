# ----------------------------------------------------------------------
# Build this project's executables:
find_ups_root( v5_34_20 )
include(ProcessSmc)

include_directories($ENV{ARTDAQ_DIR}/source)

# Nominally a test exec (no MPI) but we want this to be available to other packages.
art_make_exec(NAME artdaqDriver
	SOURCE
	$ENV{ARTDAQ_DIR}/source/proto/driver.cc
  LIBRARIES
  artdaq_Generators
  artdaq_Application # needed for messagefacility
  artdaq_DAQrate
  artdaq_DAQdata
	artdaq-core_Core
  artdaq-core_Utilities
  artdaq-core_Generators
  artdaq-utilities_Plugins
	${FHICLCPP}
  ${CETLIB}
	${CETLIB_EXCEPT}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${ROOT_CORE}
  ${MF_MESSAGELOGGER}
  )
 
art_make_exec(NAME boardreader
	SOURCE
	$ENV{ARTDAQ_DIR}/source/proto/boardreader.cc
  LIBRARIES
  artdaq_Generators
  artdaq_Application # needed for messagefacility
  artdaq_BuildInfo
  artdaq_DAQrate
  artdaq_DAQdata
	artdaq-core_Core
  artdaq-core_Utilities
  artdaq-core_Generators
  artdaq-utilities_Plugins
	artdaq_ExternalComms
	canvas
	${FHICLCPP}
  ${CETLIB}
	${CETLIB_EXCEPT}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${ROOT_CORE}
  ${MF_MESSAGELOGGER}

 )


