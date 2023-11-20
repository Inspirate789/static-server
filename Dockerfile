# Build
FROM gcc:13.2.0-bookworm AS build
WORKDIR /build

# Build the binary
RUN --mount=target=. \
    gcc -DLOG_USE_COLOR \
        -std=gnu99 -Wall -Wpedantic -Wextra -Wfloat-equal -Wfloat-conversion -Wvla  \
        -static \
        -Iinc/app -Iinc/http -Ilib/fs -Ilib/log \
        -O2 -o /app  \
        src/main.c src/app/* src/http/* lib/fs/fs.c lib/log/log.c

## Deploy
FROM scratch

# Copy our static executable
COPY --from=build /app /app

# Expose application port
EXPOSE 8080

# Run the binary
ENTRYPOINT ["/app"]
