add_subdirectory(Common)

IF(ICARUS_BUILD)
add_subdirectory(ICARUS)
ENDIF()

IF(SBND_BUILD)
add_subdirectory(SBND)
ENDIF()
