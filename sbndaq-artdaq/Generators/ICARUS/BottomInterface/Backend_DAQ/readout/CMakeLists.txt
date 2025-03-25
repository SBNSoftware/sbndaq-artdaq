project(bottomCRTreadout C)

#string(REPLACE "-Wno-overloaded-virtual" "" COMPILE_OPTIONS "${COMPILE_OPTIONS}")
SET_SOURCE_FILES_PROPERTIES( main.c PROPERTIES COMPILE_FLAGS "-Wno-pointer-sign -Wno-unused-parameter -Wno-unused-label -Wno-unused-but-set-variable -Wno-unused-function -Wno-return-type -Wno-format")

cet_make_exec(NAME bottomCRTreadout
    SOURCE
      main.c
    LIBRARIES
      TRACE::TRACE
      -lusb-1.0
      -lm
      -lrt
)

install_source()
install_headers()
