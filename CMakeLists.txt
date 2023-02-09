# ======================================================================
#  sbndaq-artdaq main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/artdaq-demo/ups/setup_for_development
#  buildtool
#
# ======================================================================


cmake_minimum_required (VERSION 3.18.2)

find_package(cetmodules)
set(CET_CETBUILDTOOLS_COMPAT TRUE)
include(Compatibility)
set(CET_WARN_DEPRECATED)

project(sbndaq_artdaq VERSION 1.06.00)

set(CMAKE_MODULE_PATH
  $ENV{ARTDAQ_UTILITIES_DIR}/Modules
  ${CMAKE_MODULE_PATH})
include(CetCMakeEnv)

cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic -pthread -Wno-unused-variable -Wno-overloaded-virtual
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



if(DEFINED $ENV{MRB_QUALS})
  string(FIND $ENV{MRB_QUALS} "icarus" ICARUS_FOUND)
  IF (ICARUS_FOUND EQUAL -1)
     MESSAGE ("No ICARUS_BUILD?")
  ELSE()
     SET(ICARUS_BUILD 1)
     MESSAGE ("Performing ICARUS_BUILD")
  ENDIF()

  string(FIND $ENV{MRB_QUALS} "sbnd" SBND_FOUND)
  IF (SBND_FOUND EQUAL -1)
     MESSAGE ("No SBND_BUILD?")
  ELSE()
     SET(SBND_BUILD 1)
     MESSAGE ("Performing SBND_BUILD")
  ENDIF()
endif()

IF (NOT(SBND_BUILD OR ICARUS_BUILD))
   MESSAGE("No experiment-specific build specified. Building all.")
   SET(SBND_BUILD 1)
   SET(ICARUS_BUILD 1)
ENDIF()

# macros for artdaq_dictionary and basic_plugin
include(ArtMake)
include(BuildPlugins)

# these are minimum required versions, not the actual product versions
find_ups_product( artdaq v3_12_01 )
find_ups_product( artdaq_utilities v1_08_01)
find_ups_product( artdaq_core v3_09_03 )
find_ups_product( canvas v3_12_04 )
find_ups_product( art v3_09_03 )
find_ups_product( art_root_io v1_08_03 )
find_ups_product( fhiclcpp v4_15_03 )
find_ups_product( messagefacility v2_08_04)
find_ups_product( cetlib v3_13_04 )
find_ups_product( cetlib_except  v1_07_04)
find_ups_root(  v6_22_08d )
find_ups_boost( v1_75_0 filesystem
                        program_options
                        system
                        unit_test_framework
 )
find_ups_product( pqxx v6_2_5a )

find_ups_product( sqlite v3_34_01_00)
find_ups_product( libxml2 )

find_ups_product( caenvme v2_50 )
find_ups_product( caencomm v1_2a )
find_ups_product( caendigitizer v2_7_2 )

find_ups_product( sbndaq_artdaq_core v1_04_00 )

find_ups_product (jsoncpp v1_7_7c)

find_ups_product (TRACE v3_17_05)

find_ups_product( zmq v4_3_1 )
find_ups_product( redis v5_0_5 )


link_directories("$ENV{ROOTSYS}/lib")
#include_directories("$ENV{ARTDAQ_INC}")
#link_directories("$ENV{ARTDAQ_LIB}")
link_directories("$ENV{ARTDAQ_UTILITIES_LIB}")

string(REGEX MATCHALL "[0-9][0-9]?[0-9]?" ART_VERSION_LIST "$ENV{ART_VERSION}")
list(GET ART_VERSION_LIST 0 ART_MAJOR_VERSION)
list(GET ART_VERSION_LIST 1 ART_MINOR_VERSION)
list(GET ART_VERSION_LIST 2 ART_PATCH_VERSION)
message("-DART_HEX_VERSION=0x${ART_MAJOR_VERSION}${ART_MINOR_VERSION}${ART_PATCH_VERSION}")
set(ART_HEX_VERSION 0x${ART_MAJOR_VERSION}${ART_MINOR_VERSION}${ART_PATCH_VERSION})
add_definitions(-DART_HEX_VERSION=0x${ART_MAJOR_VERSION}${ART_MINOR_VERSION}${ART_PATCH_VERSION})
link_directories("$ENV{ART_LIB}")

string(REGEX MATCHALL "[0-9][0-9]?[0-9]?" MESSAGEFACILITY_VERSION_LIST "$ENV{MESSAGEFACILITY_VERSION}")
list(GET MESSAGEFACILITY_VERSION_LIST 0 MESSAGEFACILITY_MAJOR_VERSION)
list(GET MESSAGEFACILITY_VERSION_LIST 1 MESSAGEFACILITY_MINOR_VERSION)
list(GET MESSAGEFACILITY_VERSION_LIST 2 MESSAGEFACILITY_PATCH_VERSION)
message("-DMESSAGEFACILITY_HEX_VERSION=0x${MESSAGEFACILITY_MAJOR_VERSION}${MESSAGEFACILITY_MINOR_VERSION}${MESSAGEFACILITY_PATCH_VERSION}")
add_definitions(-DMESSAGEFACILITY_HEX_VERSION=0x${MESSAGEFACILITY_MAJOR_VERSION}${MESSAGEFACILITY_MINOR_VERSION}${MESSAGEFACILITY_PATCH_VERSION})
link_directories("$ENV{MESSAGEFACILITY_LIB}")

string(REGEX MATCHALL "[0-9][0-9]?[0-9]?" CANVAS_VERSION_LIST "$ENV{CANVAS_VERSION}")
list(GET CANVAS_VERSION_LIST 0 CANVAS_MAJOR_VERSION)
list(GET CANVAS_VERSION_LIST 1 CANVAS_MINOR_VERSION)
list(GET CANVAS_VERSION_LIST 2 CANVAS_PATCH_VERSION)
message("-DCANVAS_HEX_VERSION=0x${CANVAS_MAJOR_VERSION}${CANVAS_MINOR_VERSION}${CANVAS_PATCH_VERSION}")
add_definitions(-DCANVAS_HEX_VERSION=0x${CANVAS_MAJOR_VERSION}${CANVAS_MINOR_VERSION}${CANVAS_PATCH_VERSION})

string(REGEX MATCHALL "[0-9][0-9]?[0-9]?" FHICLCPP_VERSION_LIST "$ENV{FHICLCPP_VERSION}")
list(GET FHICLCPP_VERSION_LIST 0 FHICLCPP_MAJOR_VERSION)
list(GET FHICLCPP_VERSION_LIST 1 FHICLCPP_MINOR_VERSION)
list(GET FHICLCPP_VERSION_LIST 2 FHICLCPP_PATCH_VERSION)
message("-DFHICLCPP_HEX_VERSION=0x${FHICLCPP_MAJOR_VERSION}${FHICLCPP_MINOR_VERSION}${FHICLCPP_PATCH_VERSION}")
add_definitions(-DFHICLCPP_HEX_VERSION=0x${FHICLCPP_MAJOR_VERSION}${FHICLCPP_MINOR_VERSION}${FHICLCPP_PATCH_VERSION})

# macros for art_dictionary and basic_plugin
#include(ArtDictionary)
include(ArtMake)
include(BuildPlugins)
include(CetTest)

# source
add_subdirectory(sbndaq-artdaq)

cet_cmake_config()
