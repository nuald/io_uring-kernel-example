// open
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

// atoi
#include <stdlib.h>

// pwritev
#include <sys/uio.h>

// fprintf
#include <stdio.h>

// close
#include <unistd.h>

// memset
#include <string.h>

// io_uring
#include <liburing.h>
#define QD 2

int sync_rw(int fd, const struct iovec *iov, int iovcnt, int offset)
{
  printf("Performing sync RW\n");

  ssize_t count = preadv(fd, iov, iovcnt, offset);
  if (count < 0) {
    perror("Unable to preadv in /dev/rw_iter\n");
    return EXIT_FAILURE;
  }

  count = pwritev(fd, iov, iovcnt, offset);
  if (count < 0) {
    perror("Unable to pwritev in /dev/rw_iter\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int async_rw(int fd, const struct iovec *iov, int iovcnt, int offset)
{
  struct io_uring_cqe *cqe;
  static int const_read = 1, const_write = 2;

  printf("Performing async RW\n");

  struct io_uring ring;
  int err = io_uring_queue_init(QD, &ring, 0);
  if (err) {
    fprintf(stderr, "queue_init: %s\n", strerror(-err));
    return EXIT_FAILURE;
  }

  int ret = EXIT_SUCCESS;
  struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
  if (!sqe) {
    fprintf(stderr, "Can't get SQE\n");
    ret = EXIT_FAILURE;
    goto done;
  }
  io_uring_prep_readv(sqe, fd, iov, iovcnt, offset);  
  io_uring_sqe_set_data(sqe, &const_read);

  sqe = io_uring_get_sqe(&ring);
  if (!sqe) {
    fprintf(stderr, "Can't get SQE\n");
    ret = EXIT_FAILURE;
    goto done;
  }
  io_uring_prep_writev(sqe, fd, iov, iovcnt, offset);  
  io_uring_sqe_set_data(sqe, &const_write);

  err = io_uring_submit(&ring);
  if (err != QD) {
    fprintf(stderr, "Can't do submit: %s\n", strerror(-err));
    ret = EXIT_FAILURE;
    goto done;
  }

  for (int i = 0; i < 2; i++) {
    err = io_uring_wait_cqe_nr(&ring, &cqe, 1);
    if (err) {
      fprintf(stderr, "Can't wait cqe: %s\n", strerror(-err));
      ret = EXIT_FAILURE;
      goto done;
    }

    if (io_uring_cqe_get_data(cqe) == &const_write) {
      printf("Write completed (%d bytes)\n", cqe->res);
    }

    if (io_uring_cqe_get_data(cqe) == &const_read) {
      printf("Read completed (%d bytes)\n", cqe->res);
    }

    io_uring_cqe_seen(&ring, cqe);
  }

 done:
  io_uring_queue_exit(&ring);
  return ret;
}

int main(int argc, const char *argv[]) {
  const size_t iovcnt = 4;
  const size_t block_size = 512;

  char *buffer = aligned_alloc(block_size, iovcnt * block_size);
  if (!buffer) {
    perror("Unable to aligned_alloc");
    return EXIT_FAILURE;
  }
  memset(buffer, 1, iovcnt * block_size);

  struct iovec iov[iovcnt];
  for (size_t i = 0; i < iovcnt; i++) {
    char *entry = &buffer[i * block_size];

    printf("entry[%lu] at %p\n", i, entry);
    sprintf(entry, "Elias-%lu\n", i);
    iov[i].iov_base = entry;
    iov[i].iov_len = block_size;
  }

  int ret = EXIT_SUCCESS;
  int fd = open("/dev/rw_iter", O_RDWR);
  if (fd < 0) {
    perror("Unable to open /dev/rw_iter");
    ret = EXIT_FAILURE;
    goto done;
  }

  if (argc == 2 && atoi(argv[1])) {
    ret = async_rw(fd, iov, iovcnt, 0);
  } else {
    ret = sync_rw(fd, iov, iovcnt, 0);
  }

 done:
  if (fd >= 0) {
    close(fd);
  }

  if (buffer) {
    free(buffer);
  }

  return ret;
}
