#add_subdirectory(Common)

#add_subdirectory(DQM)

IF(ICARUS_BUILD)
add_subdirectory(ICARUS)
ENDIF()

#IF(SBND_BUILD)
#add_subdirectory(SBND)
#ENDIF()
