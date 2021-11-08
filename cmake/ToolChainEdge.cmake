SET(CMAKE_SYSTEM_NAME Linux)

SET(BIN_DIR $ENV{BIN_DIR_GCC_Linaro})
SET(CROSS_PREFIX ${BIN_DIR}/aarch64-linux-gnu-)
SET(CMAKE_C_COMPILER  ${BIN_DIR}/aarch64-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER ${BIN_DIR}/aarch64-linux-gnu-g++)
SET(CMAKE_SYSTEM_PROCESSOR aarch64)
SET(OPT_DIR_WORK $ENV{BIN_DIR_WORK})
SET(CMAKE_FIND_ROOT_PATH  ${BIN_DIR} ${OPT_DIR_WORK}/gcc-linaro-6.2.1-2016.11-x86_64_aarch64-linux-gnu/aarch64-linux-gnu/lib64)
SET(Boost_INCLUDE_DIR  ${OPT_DIR_WORK}/gcc-linaro-6.2.1-2016.11-x86_64_aarch64-linux-gnu/include)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
