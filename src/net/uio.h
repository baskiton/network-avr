#ifndef NET_UIO_H
#define NET_UIO_H

#include <stddef.h>

struct iovec {
    void *iov_base;     // Base address of a memory region for input or output. 
    size_t iov_len;     // The size of the memory pointed to by iov_base. 
};

#endif  /* !NET_UIO_H */
