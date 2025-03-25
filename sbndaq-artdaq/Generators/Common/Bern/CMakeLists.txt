
SET_SOURCE_FILES_PROPERTIES(febdrv.cc PROPERTIES COMPILE_FLAGS "-Wno-unused-parameter -Wno-unused-but-set-variable -Wno-pointer-arith")

if(DEFINED ENV{ZMQ_DIR})
    include_directories($ENV{ZMQ_INC})
    link_directories($ENV{ZMQ_LIB})
endif()

cet_make_exec(NAME febdrv_standalone
    SOURCE
      febdrv.cc
    LIBRARIES
      sbndaq_artdaq_core::sbndaq-artdaq-core_Overlays_Common
      zmq
)

cet_make_exec(NAME febctl
    SOURCE
      febctl.cc
    LIBRARIES
      zmq
)

cet_make_exec(NAME febconf
    SOURCE
      febconf.cc
    LIBRARIES
      zmq
)

cet_make_exec(NAME febmon
    SOURCE
      febmon.cc
    LIBRARIES
      zmq
)

cet_make_exec(NAME febpeek
    SOURCE
      febpeek.cc
    LIBRARIES
      zmq
)

cet_make_exec(NAME confsetbias
    SOURCE
      confsetbias.cc
)

cet_make_exec(NAME confsetgain
    SOURCE
      confsetgain.cc
)

install_source()
install_headers()
