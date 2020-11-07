#ifndef NET_UIO_H
#define NET_UIO_H

#include <stddef.h>

struct iovec {
    void *iov_base;     // Base address of a memory region for input or output. 
    size_t iov_len;     // The size of the memory pointed to by iov_base. 
};

static inline void iovec_import(struct iovec *iov, void *buf, size_t len) {
    iov->iov_base = buf;
    iov->iov_len = len;
}

#endif  /* !NET_UIO_H */
