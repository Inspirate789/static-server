#ifndef FS_H
#define FS_H

#define FILE_COPY_BUFFER_SIZE 1024

typedef enum file_type {
    REGULAR,
    DIRECTORY,
    UNKNOWN,
} file_type_t;

file_type_t detect_file_type(char *path);
int copy_file(int src_fd, int dst_fd);

#endif //FS_H
