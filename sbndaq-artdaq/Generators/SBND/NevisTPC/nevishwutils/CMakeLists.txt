
include_directories ( $ENV{WINDRIVER_INC} )
set( WINDRIVERLIB "-L$ENV{WINDRIVER_LIB} -lwdapi1260 -lnevispci")
list(APPEND COMMON_PROJECT_LIBS  $ENV{WINDRIVER_LIB})
set(COMMON_PROJECT_LIBS ${COMMON_PROJECT_LIBS} PARENT_SCOPE)

SET_SOURCE_FILES_PROPERTIES(NevisPCIeCard-generated.cpp PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(NevisPCIeCard.cpp PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(NevisReadoutPCIeCard.cpp PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(NevisControllerPCIeCard.cpp PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)

SET_SOURCE_FILES_PROPERTIES(CrateControllerModule.cpp PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(ControllerModule.cpp PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(Crate.cpp PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(NevisTPCFEM.cpp PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(TriggerModule.cpp PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(XMITModule.cpp PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)
SET_SOURCE_FILES_PROPERTIES(XMITReader.cpp PROPERTIES COMPILE_FLAGS -Wno-error=unused-function)


cet_make( LIBRARY_NAME sbndaq-artdaq_nevishwutils
                  SOURCE NevisPCIeCard-generated.cpp
		         ControlDataPacket.cpp
			 StatusPacket.cpp
			 daqExceptions.cpp
			 NevisPCIeCard.cpp
			 NevisReadoutPCIeCard.cpp
			 NevisControllerPCIeCard.cpp
			 Crate.cpp
			 CrateControllerModule.cpp
			 ControllerModule.cpp
			 NevisTPCFEM.cpp			
			 TriggerModule.cpp
			 XMITModule.cpp
			 XMITReader.cpp
                  LIBRARIES
                        ${CETLIB}
                        ${CETLIB_EXCEPT}
			${WINDRIVERLIB}
			${TRACE}
			${FHICLCPP}
                        ${MF_MESSAGELOGGER}
                        ${MF_UTILITIES}
	)


install_source()
install_headers()
