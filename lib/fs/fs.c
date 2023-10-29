#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include "fs.h"
#include "../log/log.h"

file_type_t detect_file_type(char *path) {
    struct stat s;
    int rc = stat(path, &s);
    if (rc != 0) {
        log_error("stat %s: %s", path, strerror(errno));
        return UNKNOWN;
    }

    if (S_ISREG(s.st_mode)) {
        return REGULAR;
    } else if (S_ISDIR(s.st_mode)) {
        return DIRECTORY;
    } else {
        return UNKNOWN;
    }
}

int copy_file(int src_fd, int dst_fd) {
    char buf[FILE_COPY_BUFFER_SIZE];
    ssize_t n;
    while ((n = read(src_fd, buf, FILE_COPY_BUFFER_SIZE)) != 0) {
        if (n == -1) {
            log_error("copy_file read from fd %d: %s", src_fd, strerror(errno));
            return EXIT_FAILURE;
        }
        if (write(dst_fd, buf, FILE_COPY_BUFFER_SIZE) != n) {
            log_error("copy_file write to fd %d: %s", dst_fd, strerror(errno));
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
