set(SOURCE_FILES
  alloc.c
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

add_library(
  ${LIBRARY_NAME_STATIC}
  STATIC
  ${SOURCE_FILES}
  )

add_library(
  ${LIBRARY_NAME}
  SHARED
  ${SOURCE_FILES}
  )

install(
  TARGETS ${LIBRARY_NAME} ${LIBRARY_NAME_STATIC}
  EXPORT CDSSTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/cdss DESTINATION include)
