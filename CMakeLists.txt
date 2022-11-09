#    Author: Andreas Rosenberg
#    License: LGPL

cmake_minimum_required(VERSION 3.10)

# set the project name
project(picoHttpd_)
set(OAUTHDLL picoHttpd)

#setup build options
#define the list of source modules C
set(SOURCES_C 
   src/http/httpd.c
   src/http/httpresponse.c
   src/utils/mimetypes.c
)

set(SOURCES_OAUTH_WIN
   src/http/dllmain.c
)

set(SOURCES_EXAMPLE
   src/http/main.c
)

set(SOURCES_DEF
   src/http/httpd.def
)

set(SOURCES_RC
   src/http/httpd.rc
)

#explicitly set the language for source modules (determines the translation tool)
set_source_files_properties(${SOURCES_C} PROPERTIES LANGUAGE C)
set_source_files_properties(${SOURCES_RC} PROPERTIES LANGUAGE RC)

if (MSVC)
# This is a workaround for a bug in cmake. Cmake adds a commandline option that is only valid
# for lib tool on *nix platforms and causes an error on windows. This line fixes the lib cmd tpl.
# needed if ASM modules are part of the project
   set(CMAKE_ASM_MASM_CREATE_STATIC_LIBRARY "<CMAKE_AR> /OUT:<TARGET> <LINK_FLAGS> <OBJECTS>")
endif()

#define the target
if(WIN32 AND NOT CYGWIN)
   add_library(${OAUTHDLL} SHARED ${SOURCES_C} ${SOURCES_OAUTH_WIN} ${SOURCES_RC} ${SOURCES_DEF})
   add_executable(${PROJECT_NAME} ${SOURCES_C} ${SOURCES_EXAMPLE})
else()
   add_executable(${PROJECT_NAME} ${SOURCES_C} ${SOURCES_EXAMPLE})
endif()
   
target_include_directories(${PROJECT_NAME} PRIVATE "./include")
target_include_directories(${OAUTHDLL} PRIVATE "./include")

# set compiler options for MS Visual C
if (MSVC)
   target_compile_options(${PROJECT_NAME} PRIVATE /arch:IA32)  # plain IA32 instr. set
   target_compile_options(${PROJECT_NAME} PRIVATE /Zp1)   # struct member align 1
   target_compile_options(${PROJECT_NAME} PRIVATE /TC)   # compile as C
   target_compile_options(${PROJECT_NAME} PRIVATE /Gd)   # C calling convention

   target_compile_options(${OAUTHDLL} PRIVATE /arch:IA32)  # plain IA32 instr. set
   target_compile_options(${OAUTHDLL} PRIVATE /Zp1)   # struct member align 1
   target_compile_options(${OAUTHDLL} PRIVATE /TC)   # compile as C
   target_compile_options(${OAUTHDLL} PRIVATE /Gd)   # C calling convention

   target_compile_definitions(${PROJECT_NAME} PRIVATE _LIB WIN32_LEAN_AND_MEAN)
   target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_DEPRECATE)
   target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_NONSTDC__NO_DEPRECATE)

   target_compile_definitions(${OAUTHDLL} PRIVATE _LIB WIN32_LEAN_AND_MEAN)
   target_compile_definitions(${OAUTHDLL} PRIVATE _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(${OAUTHDLL} PRIVATE _CRT_SECURE_NO_DEPRECATE)
   target_compile_definitions(${OAUTHDLL} PRIVATE _CRT_NONSTDC__NO_DEPRECATE)

   target_link_options(${PROJECT_NAME} PRIVATE "/SUBSYSTEM:CONSOLE")
   target_link_options(${OAUTHDLL} PRIVATE "/SUBSYSTEM:WINDOWS")
   
      # set other defines for compilation
      if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
         target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
         target_compile_options(${PROJECT_NAME} PRIVATE /MTd)   # Multi threaded debug
         target_link_libraries(${PROJECT_NAME} PRIVATE oldnames kernel32 ws2_32) 

         target_compile_definitions(${OAUTHDLL} PRIVATE DEBUG)
         target_compile_options(${OAUTHDLL} PRIVATE /MTd)   # Multi threaded debug
         target_link_libraries(${OAUTHDLL} PRIVATE oldnames kernel32 ws2_32) 
      else()
         target_compile_options(${PROJECT_NAME} PRIVATE /MT)    # Multi threaded / static CRT
         target_compile_options(${PROJECT_NAME} PRIVATE /GS-)   # disable buffer security checks
         target_link_options(${PROJECT_NAME} PRIVATE "/INCREMENTAL:NO")
         target_link_options(${PROJECT_NAME} PRIVATE "/DYNAMICBASE:NO")
         target_link_libraries(${PROJECT_NAME} PRIVATE oldnames kernel32 ws2_32)

         target_compile_options(${OAUTHDLL} PRIVATE /MT)    # Multi threaded / static CRT
         target_compile_options(${OAUTHDLL} PRIVATE /GS-)   # disable buffer security checks
         target_link_options(${OAUTHDLL} PRIVATE "/INCREMENTAL:NO")
         target_link_options(${OAUTHDLL} PRIVATE "/DYNAMICBASE:NO")
         target_link_libraries(${OAUTHDLL} PRIVATE oldnames kernel32 ws2_32)

      endif()
   # other C compilers (*nix platforms)
   else()
      target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
      target_compile_options(${PROJECT_NAME} PRIVATE -g3)
endif()

message(STATUS "${CMAKE_VS_PLATFORM_NAME}")
# other special option related to platform
if ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
   
elseif  ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
   
endif ()

# use C name mangling regarding linkage
set_target_properties(${PROJECT_NAME} PROPERTIES LANGUAGE C LINKER_LANGUAGE C)
set_target_properties(${OAUTHDLL} PROPERTIES LANGUAGE C LINKER_LANGUAGE C)
