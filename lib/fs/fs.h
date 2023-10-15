#ifndef FS_H
#define FS_H

typedef enum file_type {
    REGULAR,
    DIRECTORY,
    UNKNOWN,
} file_type_t;

file_type_t detect_file_type(char *path);
int copy_file(int dst_fd, int src_fd);

#endif //FS_H
