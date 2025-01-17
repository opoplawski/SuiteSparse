#-------------------------------------------------------------------------------
# SuiteSparse/CHOLMOD/CMakeLists.txt:  cmake for CHOLMOD
#-------------------------------------------------------------------------------

# CHOLMOD: Copyright (c) 2005-2022, Timothy A. Davis.
# Copyright and license varies by module.

#-------------------------------------------------------------------------------
# get the version
#-------------------------------------------------------------------------------

# cmake 3.22 is required to find the BLAS/LAPACK
cmake_minimum_required ( VERSION 3.22 )

set ( CHOLMOD_DATE "Nov 4, 2022" )
set ( CHOLMOD_VERSION_MAJOR 4 )
set ( CHOLMOD_VERSION_MINOR 0 )
set ( CHOLMOD_VERSION_SUB   0 )

message ( STATUS "Building CHOLMOD version: v"
    ${CHOLMOD_VERSION_MAJOR}.
    ${CHOLMOD_VERSION_MINOR}.
    ${CHOLMOD_VERSION_SUB} " (" ${CHOLMOD_DATE} ")" )

#-------------------------------------------------------------------------------
# SuiteSparse policies
#-------------------------------------------------------------------------------

set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/cmake_modules
    ${CMAKE_SOURCE_DIR}/../COLAMD/cmake_modules
    ${CMAKE_SOURCE_DIR}/../AMD/cmake_modules
    ${CMAKE_SOURCE_DIR}/../CAMD/cmake_modules
    ${CMAKE_SOURCE_DIR}/../CCOLAMD/cmake_modules
    ${CMAKE_SOURCE_DIR}/../SuiteSparse_metis/cmake_modules
    ${CMAKE_SOURCE_DIR}/../SuiteSparse_config/cmake_modules )

if ( NOT DEFINED ENABLE_CUDA )
    # default is to use CUDA, if available
    set ( ENABLE_CUDA true )
endif ( )

include ( SuiteSparsePolicy )

#-------------------------------------------------------------------------------
# define the project
#-------------------------------------------------------------------------------

if ( SUITESPARSE_CUDA )
    # CHOLMOD with CUDA
    project ( cholmod
            VERSION "${CHOLMOD_VERSION_MAJOR}.${CHOLMOD_VERSION_MINOR}.${CHOLMOD_VERSION_SUB}"
            LANGUAGES CUDA C CXX )
else ( )
    # CHOLMOD without CUDA
    project ( cholmod
            VERSION "${CHOLMOD_VERSION_MAJOR}.${CHOLMOD_VERSION_MINOR}.${CHOLMOD_VERSION_SUB}"
            LANGUAGES C )
endif ( )

#-------------------------------------------------------------------------------
# find library dependencies
#-------------------------------------------------------------------------------

enable_language ( C Fortran )
find_package ( OpenMP )
find_package ( SuiteSparse_config 6.0.0 REQUIRED )
find_package ( COLAMD 3.0.0 REQUIRED )
find_package ( AMD 3.0.0 REQUIRED )
find_package ( CAMD 3.0.0 )
find_package ( CCOLAMD 3.0.0 )
find_package ( SuiteSparse_metis 5.1.0 )

include ( SuiteSparseBLAS )     # requires cmake 3.22

if ( NOT SUITESPARSE_METIS_FOUND )
    set ( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DNPARTITION " )
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNPARTITION " )
endif ( )

if ( SUITESPARSE_CUDA )
    # with CUDA
    set ( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DSUITESPARSE_CUDA " )
    add_subdirectory ( GPU )
    set ( CHOLMOD_CUDA cholmod_cuda ${CUDA_LIBRARIES} )
    include_directories ( GPU ${CUDAToolkit_INCLUDE_DIRS} )
    link_directories ( "GPU" "${CUDA_LIBRARIES}" "/usr/local/cuda/lib64/stubs" "/usr/local/cuda/lib64" )
else ( )
    # without CUDA and RMM
    set ( CHOLMOD_CUDA )
endif ( )


#-------------------------------------------------------------------------------
# configure files
#-------------------------------------------------------------------------------

# determine which modules of CHOLMOD to include in the library
if ( NGPL )
    add_compile_definitions ( NGPL )
endif ( )

configure_file ( "Config/cholmod.h.in" "${PROJECT_SOURCE_DIR}/Include/cholmod.h")
configure_file ( "Config/cholmod_version.tex.in" "${PROJECT_SOURCE_DIR}/Doc/cholmod_version.tex")

#-------------------------------------------------------------------------------
# include directories
#-------------------------------------------------------------------------------

include_directories ( Check Cholesky Core MatrixOps Modify Partition
    Supernodal Include ${SUITESPARSE_CONFIG_INCLUDE_DIR} ${AMD_INCLUDE_DIR}
    ${COLAMD_INCLUDE_DIR} )

#-------------------------------------------------------------------------------
# dynamic cholmod library properties
#-------------------------------------------------------------------------------

file ( GLOB CHOLMOD_SOURCES "Check/cholmod_*.c" "Cholesky/cholmod_*.c"
    "Core/cholmod_*.c" "MatrixOps/cholmod_*.c" "Modify/cholmod_*.c"
    "Partition/cholmod_*.c" "Supernodal/cholmod_*.c" )

add_library ( cholmod SHARED ${CHOLMOD_SOURCES} )

set_target_properties ( cholmod PROPERTIES
    VERSION ${CHOLMOD_VERSION_MAJOR}.${CHOLMOD_VERSION_MINOR}.${CHOLMOD_VERSION_SUB}
    C_STANDARD_REQUIRED 11
    SOVERSION ${CHOLMOD_VERSION_MAJOR}
    PUBLIC_HEADER "Include/cholmod.h" )

if ( SUITESPARSE_CUDA )
    set_target_properties ( cholmod PROPERTIES CUDA_SEPARABLE_COMPILATION on )
    set_target_properties ( cholmod PROPERTIES POSITION_INDEPENDENT_CODE on )
endif ( )

#-------------------------------------------------------------------------------
# static cholmod library properties
#-------------------------------------------------------------------------------

if ( NOT NSTATIC )
add_library ( cholmod_static STATIC ${CHOLMOD_SOURCES} )

set_target_properties ( cholmod_static PROPERTIES
    VERSION ${CHOLMOD_VERSION_MAJOR}.${CHOLMOD_VERSION_MINOR}.${CHOLMOD_VERSION_SUB}
    C_STANDARD_REQUIRED 11
    OUTPUT_NAME cholmod
    SOVERSION ${CHOLMOD_VERSION_MAJOR} )

if ( SUITESPARSE_CUDA )
    set_target_properties ( cholmod_static PROPERTIES CUDA_SEPARABLE_COMPILATION on )
    set_target_properties ( cholmod_static PROPERTIES POSITION_INDEPENDENT_CODE on )
endif ( )
endif ( )

#-------------------------------------------------------------------------------
# add the library dependencies
#-------------------------------------------------------------------------------

# suitesparseconfig:
target_link_libraries ( cholmod PUBLIC ${SUITESPARSE_CONFIG_LIBRARIES} )
if ( NOT NSTATIC )
target_link_libraries ( cholmod_static PUBLIC ${SUITESPARSE_CONFIG_LIBRARIES} )
endif ( )

# OpenMP:
if ( OPENMP_FOUND )
    message ( STATUS "OpenMP C libraries:      ${OpenMP_C_LIBRARIES} ")
    message ( STATUS "OpenMP C include:        ${OpenMP_C_INCLUDE_DIRS} ")
    message ( STATUS "OpenMP C flags:          ${OpenMP_C_FLAGS} ")
    target_link_libraries ( cholmod PUBLIC ${OpenMP_C_LIBRARIES} )
    if ( NOT NSTATIC )
    target_link_libraries ( cholmod_static PUBLIC ${OpenMP_C_LIBRARIES} )
    endif ( )
    set ( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${OpenMP_C_FLAGS} " )
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_C_FLAGS} " )
    include_directories ( ${OpenMP_C_INCLUDE_DIRS} )
endif ( )

# libm:
if ( NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC" )
    target_link_libraries ( cholmod PUBLIC m )
    if ( NOT NSTATIC )
    target_link_libraries ( cholmod_static PUBLIC m )
    endif ( )
endif ( )

# amd:
target_link_libraries ( cholmod PUBLIC ${AMD_LIBRARIES} )
if ( NOT NSTATIC )
target_link_libraries ( cholmod_static PUBLIC ${AMD_LIBRARIES} )
endif ( )

# colamd:
target_link_libraries ( cholmod PUBLIC ${COLAMD_LIBRARIES} )
if ( NOT NSTATIC )
target_link_libraries ( cholmod_static PUBLIC ${COLAMD_LIBRARIES} )
endif ( )

# BLAS:
message ( STATUS "BLAS libraries:      ${BLAS_LIBRARIES} ")
message ( STATUS "BLAS linker flags:   ${BLAS_LINKER_FLAGS} ")
target_link_libraries ( cholmod PUBLIC ${BLAS_LIBRARIES} )
if ( NOT NSTATIC )
target_link_libraries ( cholmod_static PUBLIC ${BLAS_LIBRARIES} )
endif ( )
include_directories ( ${BLAS_INCLUDE_DIRS} )

# LAPACK:
message ( STATUS "LAPACK libraries:    ${LAPACK_LIBRARIES} ")
message ( STATUS "LAPACK linker flags: ${LAPACK_LINKER_FLAGS} ")
target_link_libraries ( cholmod PUBLIC ${LAPACK_LIBRARIES} )
if ( NOT NSTATIC )
target_link_libraries ( cholmod_static PUBLIC ${LAPACK_LIBRARIES} )
endif ( )

# camd:
if ( CAMD_FOUND )
    include_directories ( ${CAMD_INCLUDE_DIR} )
    target_link_libraries ( cholmod PUBLIC ${CAMD_LIBRARIES} )
    if ( NOT NSTATIC )
    target_link_libraries ( cholmod_static PUBLIC ${CAMD_LIBRARIES} )
    endif ( )
endif ( )

# ccolamd:
if ( CCOLAMD_FOUND )
    include_directories ( ${CCOLAMD_INCLUDE_DIR} )
    target_link_libraries ( cholmod PUBLIC ${CCOLAMD_LIBRARIES} )
    if ( NOT NSTATIC )
    target_link_libraries ( cholmod_static PUBLIC ${CCOLAMD_LIBRARIES} )
    endif ( )
endif ( )

# metis:
if ( SUITESPARSE_METIS_FOUND )
    include_directories ( ${SUITESPARSE_METIS_INCLUDE_DIR} )
    target_link_libraries ( cholmod PUBLIC ${SUITESPARSE_METIS_LIBRARIES} )
    if ( NOT NSTATIC )
    target_link_libraries ( cholmod_static PUBLIC ${SUITESPARSE_METIS_LIBRARIES} )
    endif ( )
endif ( )

# CUDA
if ( SUITESPARSE_CUDA )
    message ( STATUS "CHOLMOD cuda: " ${CHOLMOD_CUDA} )
    target_link_libraries ( cholmod PUBLIC ${CHOLMOD_CUDA} ${CUDA_LIBRARIES} )
    target_link_libraries ( cholmod_static PUBLIC ${CHOLMOD_CUDA} ${CUDA_LIBRARIES} )
endif ( )

#-------------------------------------------------------------------------------
# CHOLMOD installation location
#-------------------------------------------------------------------------------

if ( GLOBAL_INSTALL )
    # install in /usr/local/lib and /usr/local/include.
    # requires "sudo make install"
    message ( STATUS "Installation will be system-wide (requires 'sudo make install')" )
    install ( TARGETS cholmod
        LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )
    file ( GLOB ${CMAKE_MODULES} "cmake_modules/FindCHOLMOD*.cmake" )
    install ( FILES 
        cmake_modules/FindCHOLMOD.cmake
        cmake_modules/FindCHOLMOD_CUDA.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SuiteSparse 
        COMPONENT Development )
    if ( NOT NSTATIC )
    install ( TARGETS cholmod_static
        ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR} )
    endif ( )
endif ( )

if ( INSIDE_SUITESPARSE )
    # also install in SuiteSparse/lib and SuiteSparse/include;
    # does not require "sudo make install", just "make install"
    message ( STATUS "Installation in ../lib and ../include only," )
    message ( STATUS "  with 'make install'. No 'sudo' required." )
    install ( TARGETS cholmod
        LIBRARY       DESTINATION ${SUITESPARSE_LIBDIR}
        PUBLIC_HEADER DESTINATION ${SUITESPARSE_INCLUDEDIR} )
    install ( FILES 
        cmake_modules/FindCHOLMOD.cmake
        cmake_modules/FindCHOLMOD_CUDA.cmake
        DESTINATION ${SUITESPARSE_LIBDIR}/cmake/SuiteSparse 
        COMPONENT Development )
    if ( NOT NSTATIC )
    install ( TARGETS cholmod_static
        ARCHIVE       DESTINATION ${SUITESPARSE_LIBDIR} )
    endif ( )
endif ( )

#-------------------------------------------------------------------------------
# Demo library and programs
#-------------------------------------------------------------------------------

if ( DEMO )

    #---------------------------------------------------------------------------
    # demo library
    #---------------------------------------------------------------------------

    message ( STATUS "Also compiling the demos in CHOLMOD/Demo" )

    #---------------------------------------------------------------------------
    # Demo programs
    #---------------------------------------------------------------------------

    add_executable ( cholmod_demo   "Demo/cholmod_demo.c" )
    add_executable ( cholmod_l_demo "Demo/cholmod_l_demo.c" )
    add_executable ( cholmod_simple "Demo/cholmod_simple.c" )
    add_executable ( readhb         "Demo/readhb.f" )
    add_executable ( readhb2        "Demo/readhb2.f" )
    add_executable ( reade          "Demo/reade.f" )

    # Libraries required for Demo programs
    target_link_libraries ( cholmod_demo   PUBLIC cholmod ${CHOLMOD_CUDA} )
    target_link_libraries ( cholmod_l_demo PUBLIC cholmod ${CHOLMOD_CUDA} )
    target_link_libraries ( cholmod_simple PUBLIC cholmod ${CHOLMOD_CUDA} )

else ( )

    message ( STATUS "Skipping the demos in CHOLMOD/Demo" )

endif ( )

#-------------------------------------------------------------------------------
# report status
#-------------------------------------------------------------------------------

include ( SuiteSparseReport )


