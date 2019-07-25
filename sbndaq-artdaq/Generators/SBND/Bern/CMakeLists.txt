
include_directories( . )

SET_SOURCE_FILES_PROPERTIES(febdrv.cc PROPERTIES COMPILE_FLAGS "-Wno-unused-parameter -Wno-unused-but-set-variable -Wno-pointer-arith")

cet_make_exec(febdrv
		SOURCE febdrv.cc
		LIBRARIES zmq
		       )

cet_make_exec(febctl
		SOURCE febctl.cc
		LIBRARIES zmq
		       )

