project(bottomCRTreadout C)

#add_compile_options(-m32 -g -I -lm -lrt)

#string(REPLACE "-Wno-overloaded-virtual" "" COMPILE_OPTIONS "${COMPILE_OPTIONS}")
SET_SOURCE_FILES_PROPERTIES( main.c PROPERTIES COMPILE_FLAGS "-Wno-pointer-sign -Wno-unused-parameter -Wno-unused-label -Wno-unused-but-set-variable -Wno-unused-function -Wno-return-type -Wno-format")

cet_make_exec(bottomCRTreadout
    SOURCE
      main.c
    LIBRARIES
      libusb-1.0.so.0.1.0
)

set_target_properties(bottomCRTreadout PROPERTIES COMPILE_FLAGS "-m32 -g -I -lm -lrt" LINK_FLAGS "-m32 -g -I -lm -lrt")

install_source()
install_headers()
