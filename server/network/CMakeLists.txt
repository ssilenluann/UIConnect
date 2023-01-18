aux_source_directory(./socket/ DIR_LIB_SRCS)
aux_source_directory(./epoll/ DIR_LIB_SRCS)
aux_source_directory(./event/ DIR_LIB_SRCS)
aux_source_directory(./ DIR_LIB_SRCS)

add_library(network ${DIR_LIB_SRCS})

