# ======================================================================
#  sbndaq-artdaq main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/artdaq-demo/ups/setup_for_development
#  buildtool
# ======================================================================


# use cmake 3.7 or later
cmake_minimum_required (VERSION 3.7)


project(sbndaq-artdaq)

string(FIND $ENV{MRB_QUALS} "icarus" ICARUS_FOUND)
IF (ICARUS_FOUND EQUAL -1)
   MESSAGE ("No ICARUS_BUILD")
ELSE()
   SET(ICARUS_BUILD 1)
   MESSAGE ("Performing ICARUS_BUILD")
ENDIF()

string(FIND $ENV{MRB_QUALS} "sbnd" SBND_FOUND)
IF (SBND_FOUND EQUAL -1)
   MESSAGE ("No SBND_BUILD")
ELSE()
   SET(SBND_BUILD 1)
   MESSAGE ("Performing SBND_BUILD")
ENDIF()

IF (NOT(SBND_BUILD OR ICARUS_BUILD))
   MESSAGE("No experiment-specific build specified. Building all.")
   SET(SBND_BUILD 1)
   SET(ICARUS_BUILD 1)
ENDIF()

# cetbuildtools contains our cmake modules
SET ( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
IF (NOT CETBUILDTOOLS_VERSION)
    MESSAGE (FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules")
ENDIF()

set(CMAKE_MODULE_PATH 
        $ENV{ART_DIR}/Modules
        $ENV{CETBUILDTOOLS_DIR}/Modules
        ${CMAKE_MODULE_PATH})


include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT
 WERROR
 NO_UNDEFINED
 EXTRA_FLAGS -pedantic -Wall -fPIC -DLINUX -D_LINUX -Wno-unused-variable -Wno-overloaded-virtual
 )

SET ( SANITIZE_ADDRESS $ENV{SANITIZE_ADDRESS} )
IF ( SANITIZE_ADDRESS )
    MESSAGE (STATUS "Enable  -fsanitize=address -Wno-unused-command-line-argument -fno-omit-frame-pointer")
    cet_add_compiler_flags(-fsanitize=address  -Wno-unused-command-line-argument -fno-omit-frame-pointer)
ENDIF()

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_product( artdaq v2_03_00 )
find_ups_product( artdaq_utilities v1_03_00)
find_ups_product( artdaq_core v3_00_00 )
find_ups_product( artdaq_utilities v1_03_00)

find_ups_product( art v2_07_03 )
find_ups_product( fhiclcpp v4_05_01 )
find_ups_product( messagefacility v2_00_02 )
find_ups_product( cetlib v3_00_01 )
find_ups_product( cetlib_except v1_01_02 )
find_ups_root( v6_08_06 )
find_ups_boost( v1_63_0b filesystem
                        program_options
                        system
                        unit_test_framework
 )
find_ups_product( pqxx v6_2_5a )
find_ups_product( cetbuildtools v5_08_01 )
find_ups_product( sqlite v3_16_02_00a)
 
find_ups_product( caenvme v2_50 )
find_ups_product( caencomm v1_2a )
find_ups_product( caendigitizer v2_7_2 )

find_ups_product( sbndaq_artdaq_core v0_01_03 )

#include CAEN directories...
include_directories($ENV{CAENCOMMDIR}/include)

#experiment specific products needed
IF(ICARUS_BUILD)
  find_ups_product( icarus_artdaq_base v0_03_00 )
ENDIF()

IF(SBND_BUILD)
  find_ups_product( libxml2 v2_9_4b )
ENDIF()


# Needed if using old art with new cetbuildtools / CMake.
check_ups_version(art "$ENV{ART_VERSION}" v1_11_00 PRODUCT_OLDER_VAR have_old_art)
if (have_old_art)
  cmake_policy(SET CMP0046 OLD)
endif()
unset(have_old_art)

string(REGEX MATCHALL "[0-9][0-9]?[0-9]?" MESSAGEFACILITY_VERSION_LIST "$ENV{MESSAGEFACILITY_VERSION}")
list(GET MESSAGEFACILITY_VERSION_LIST 0 MESSAGEFACILITY_MAJOR_VERSION)
list(GET MESSAGEFACILITY_VERSION_LIST 1 MESSAGEFACILITY_MINOR_VERSION)
list(GET MESSAGEFACILITY_VERSION_LIST 2 MESSAGEFACILITY_PATCH_VERSION)

#special tracemf buggy fix thing
string(REGEX MATCHALL "[0-9][0-9]?[0-9]?" MESSAGEFACILITY_VERSION_LIST "$ENV{MESSAGEFACILITY_VERSION}")
list(GET MESSAGEFACILITY_VERSION_LIST 0 MESSAGEFACILITY_MAJOR_VERSION)
list(GET MESSAGEFACILITY_VERSION_LIST 1 MESSAGEFACILITY_MINOR_VERSION)
list(GET MESSAGEFACILITY_VERSION_LIST 2 MESSAGEFACILITY_PATCH_VERSION)
message("-DMESSAGEFACILITY_HEX_VERSION=0x${MESSAGEFACILITY_MAJOR_VERSION}${MESSAGEFACILITY_MINOR_VERSION}${MESSAGEFACILITY_PATCH_VERSION}")
add_definitions(-DMESSAGEFACILITY_HEX_VERSION=0x${MESSAGEFACILITY_MAJOR_VERSION}${MESSAGEFACILITY_MINOR_VERSION}${MESSAGEFACILITY_PATCH_VERSION})

#message("-DMESSAGEFACILITY_HEX_VERSION=0x${MESSAGEFACILITY_MAJOR_VERSION}${MESSAGEFACILITY_MINOR_VERSION}${MESSAGEFACILITY_PATCH_VERSION}")
#add_definitions(-DMESSAGEFACILITY_HEX_VERSION=0x${MESSAGEFACILITY_MAJOR_VERSION}${MESSAGEFACILITY_MINOR_VERSION}${MESSAGEFACILITY_PATCH_VERSION})

# macros for art_dictionary and simple_plugin
include(ArtDictionary)
include(ArtMake)
include(BuildPlugins)
include(CetTest)

# source
add_subdirectory(sbndaq-artdaq)

# testing
#add_subdirectory(test)

# tools
add_subdirectory(tools)

# examples
#FILE(COPY examples DESTINATION .)
#INSTALL(DIRECTORY examples DESTINATION ${product}/${version})

# ups - table and config files
add_subdirectory(ups)

# packaging utility
include(UseCPack)
