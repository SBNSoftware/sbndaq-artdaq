include_directories( ${CAENCOMM_INC} )
cet_find_library( CAENCOMM         NAMES CAENComm         PATHS ENV CAENCOMM_LIB        NO_DEFAULT_PATH )

cet_make_library( LIBRARY_NAME icarus-base_A2795Board
		  SOURCE Cudpux.cc A2795Board.cc veto.cc
		  LIBRARIES
		        ${CAENCOMM}
		)

cet_make_library( LIBRARY_NAME icarus-base_VetoUDP
		  SOURCE VetoUDP.cc
		  LIBRARIES			
		        icarus-base_A2795Board
		)
cet_make_library( LIBRARY_NAME icarus-base_PhysCrate
		  SOURCE PhysCrate.cc
		  LIBRARIES		        
		        icarus-base_A2795Board		        
		)
cet_make_exec(VetoUDP
  SOURCE VetoUDP_main.cc
  LIBRARIES
  icarus-base_VetoUDP
  )
  
	      
install_headers()

