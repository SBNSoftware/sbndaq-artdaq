# ======================================================================
#  sbndaq-artdaq main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/artdaq-demo/ups/setup_for_development
#  buildtool
#
# ======================================================================


cmake_minimum_required (VERSION 3.19 FATAL_ERROR)

find_package(cetmodules 3.16.00 REQUIRED)

project(sbndaq_artdaq VERSION 1.10.00)

include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic -pthread -Wno-unused-variable
)

string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE_UC)
if(BTYPE_UC STREQUAL DEBUG)
  add_definitions(-DDEBUGME)
  if(DEFINED ENV{USE_ASAN})
    if($ENV{USE_ASAN} GREATER 0)
      message("!!! Address Sanitizer Activated!!!")
      add_compile_options(-fsanitize=address -Wno-unused-command-line-argument)
      add_link_options(-lasan)
    endif()
  endif()
  if(DEFINED ENV{USE_TSAN})
    if($ENV{USE_TSAN} GREATER 0)
      message("!!! Thread Sanitizer Activated!!!")
      add_compile_options(-fsanitize=thread -Wno-unused-command-line-argument)
      add_link_options(-ltsan)
    endif()
  endif()
  if(DEFINED ENV{USE_GCOV})
    add_compile_options(-fprofile-arcs -ftest-coverage --coverage)
    add_link_options(--coverage)
  endif()
endif()

#exeption stack trace
if(DEFINED ENV{USE_EXCEPTIONSTACKTRACE})
  if($ENV{USE_EXCEPTIONSTACKTRACE} GREATER 0)
    message("!!! Exception StackTrace Activated!!!")
    add_definitions(-D EXCEPTIONSTACKTRACE)
    set( EXCEPTIONSTACKTRACE_LIB  artdaq-core_Utilities_ExceptionStackTrace )
    add_compile_options( -rdynamic -Wno-unused-command-line-argument )
    add_link_options( -rdynamic )
  endif()
endif()

# Debug streamer.
string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE_UC)
if(BTYPE_UC STREQUAL DEBUG)
  add_definitions(-DDEBUGME)
endif()

message("Searching ${sbndaq_artdaq_EXEC_PREFIX} for icarus or sbnd qualifiers")
string(FIND "x${sbndaq_artdaq_EXEC_PREFIX}" "icarus" ICARUS_FOUND)
string(FIND "x${sbndaq_artdaq_EXEC_PREFIX}" "sbnd" SBND_FOUND)

IF (ICARUS_FOUND EQUAL -1)
   MESSAGE ("No ICARUS_BUILD?")
ELSE()
   SET(ICARUS_BUILD 1)
   MESSAGE ("Performing ICARUS_BUILD")
ENDIF()

IF (SBND_FOUND EQUAL -1)
   MESSAGE ("No SBND_BUILD?")
ELSE()
   SET(SBND_BUILD 1)
   MESSAGE ("Performing SBND_BUILD")
ENDIF()

IF (NOT(SBND_BUILD OR ICARUS_BUILD))
   MESSAGE("No experiment-specific build specified. Building all.")
   SET(SBND_BUILD 1)
   SET(ICARUS_BUILD 1)
ENDIF()

# these are minimum required versions, not the actual product versions
find_package( artdaq 3.12.05 REQUIRED EXPORT)
find_package( artdaq_utilities 1.08.04 REQUIRED EXPORT)
find_package( artdaq_core 3.09.13 REQUIRED EXPORT )
find_package( canvas 3.12.04  REQUIRED EXPORT)
find_package( art 3.09.03  REQUIRED EXPORT)
find_package( art_root_io 1.08.03  REQUIRED EXPORT)
find_package( fhiclcpp 4.15.03 REQUIRED EXPORT )
find_package( messagefacility 2.08.04 REQUIRED EXPORT)
find_package( cetlib 3.13.04  REQUIRED EXPORT)
find_package( cetlib_except  1.07.04 REQUIRED EXPORT)
find_package( Boost 1.75.0 QUIET COMPONENTS filesystem
                        program_options
                        system
                        unit_test_framework REQUIRED EXPORT
 )
#find_package( pqxx 6.2.5  REQUIRED EXPORT)

#find_package( sqlite 3.34.01.00 REQUIRED EXPORT)
find_package( libxml2 REQUIRED EXPORT )

#find_package( caenvme 2.50 REQUIRED EXPORT )
#find_package( caencomm 1.2  REQUIRED EXPORT)
#find_package( caendigitizer 2.7.2  REQUIRED EXPORT)

find_package( sbndaq_artdaq_core 1.04.00  REQUIRED EXPORT)

#find_package (jsoncpp 1.7.7 REQUIRED EXPORT)

find_package (TRACE 3.17.09 REQUIRED EXPORT)

if( DEFINED SPACK_BUILD )
find_package( cppzmq 4.3.1  REQUIRED EXPORT)
else()
find_package( zmq 4.3.1  REQUIRED EXPORT)
endif()
#find_package( redis 5.0.5 REQUIRED EXPORT )

# macros for art_dictionary and basic_plugin
include(BuildPlugins)

# source
add_subdirectory(sbndaq-artdaq)

cet_cmake_config()
