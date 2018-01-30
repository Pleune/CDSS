set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(SOURCE_FILES
  logger.c
  main.c
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

add_executable(${TESTER_NAME} ${SOURCE_FILES})
target_link_libraries(${TESTER_NAME} cdss m ${CMAKE_THREAD_LIBS_INIT})

add_test(CDSS_TESTER ${TESTER_NAME})
