set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(SOURCE_FILES
  logger.c
  mpool_dynamic.c
  mpool_grow.c
  mpool_static.c
  ntorus.c
  ringbuff.c
  stack.c
  threadpool.c
  voxtree.c
  )

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_STATIC_LINKER_FLAGS_DEBUG  "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fsanitize=address")

foreach(SOURCE_FILE ${SOURCE_FILES})
  get_filename_component(FILE ${SOURCE_FILE} NAME_WE)

  add_executable(${FILE} ${SOURCE_FILE})
  target_link_libraries(${FILE} ${LIBRARY_NAME_STATIC} m ${CMAKE_THREAD_LIBS_INIT})
  add_test(NAME ${FILE} COMMAND ${FILE})
endforeach(SOURCE_FILE)
