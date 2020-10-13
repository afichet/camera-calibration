project(lever)

SET(LINSOLVERS_RETAIN_MEMORY 1 CACHE BOOL "Should linear solvers retain working memory between calls? (non-reentrant!)")
SET(LM_DBL_PREC 1 CACHE BOOL "Build double precision routines?")
SET(LM_SNGL_PREC 1 CACHE BOOL "Build single precision routines?")
#SET(NEED_F2C 1 CACHE BOOL "Do we need either f2c or F77/I77?")
OPTION(BUILD_DEMO "Build demo program?" FALSE)

set(PUBLIC_HEADERS
  levmar.h
  misc.h
  compiler.h
  )

add_library(levmar STATIC
  lm.c 
  Axb.c 
  misc.c 
  lmlec.c 
  lmbc.c 
  lmblec.c 
  lmbleic.c
)

set_target_properties(levmar PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
target_include_directories(levmar PUBLIC .)

find_package(BLAS)
find_package(LAPACK)

if(LAPACK_FOUND AND BLAS_FOUND)
  set(HAVE_LAPACK 1)
  set(HAVE_BLAS 1)
  target_link_libraries(levmar PUBLIC BLAS::BLAS LAPACK::LAPACK)
endif()

find_package(PLASMA QUIET)

if(PLASMA_FOUND)
  set(HAVE_PLASMA 1)
  set_source_files_properties(Axb.c PROPERTIES LANGUAGE CXX)
  target_link_libraries(levmar PUBLIC PLASMA::PLASMA)
endif()

if (UNIX)
  target_link_libraries(levmar PUBLIC m)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/levmar.h.in ${CMAKE_CURRENT_SOURCE_DIR}/levmar.h)

# compiler flags
#ADD_DEFINITIONS(-DLINSOLVERS_RETAIN_MEMORY) # do not free memory between linear solvers calls
#REMOVE_DEFINITIONS(-DLINSOLVERS_RETAIN_MEMORY) # free memory between calls

include_directories(${CMAKE_SOURCE_DIR})

