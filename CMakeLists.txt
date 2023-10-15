cmake_minimum_required(VERSION 3.26)
project(static-server C)

set(CMAKE_C_STANDARD 99)

add_executable(static-server src/main.c
        inc/app/server.h
        src/app/server.c
        inc/app/thread_pool.h
        src/app/thread_pool.c
        src/app/thread_pool.c
        lib/log/log.c
        lib/log/log.h
        inc/http/header.h
        inc/http/headers.h
        inc/http/request.h
        src/http/request.c
        inc/http/response.h
        inc/http/event.h
        src/http/response.c
        src/http/header.c
        src/http/headers.c
        lib/fs/fs.c
        lib/fs/fs.h
)
