#
# Multi-stage image for guacenc (Guacamole recording → video).
# Style mirrors desktop-trajectory-capture process_audio image conventions:
# pinned Alpine, builder + production-image, IMAGE_ID, non-root srv, ENTRYPOINT.
#
# Build from repository root:
#   docker build -f guacenc/Dockerfile -t guacenc .
#

# All images in this Dockerfile need to be in sync, so use this var.
ARG ALPINE_VERSION=3.22

FROM alpine:${ALPINE_VERSION} AS builder

RUN apk add --no-cache \
        autoconf \
        automake \
        build-base \
        cairo-dev \
        cunit-dev \
        ffmpeg-dev \
        libjpeg-turbo-dev \
        libpng-dev \
        libtool \
        libwebp-dev \
        pkgconf \
        util-linux-dev

WORKDIR /src

COPY . .

RUN autoreconf -fi \
    && ./configure \
        --prefix=/usr/local \
        --disable-guacd \
        --disable-guaclog \
    && make -j"$(nproc)" \
    && make -C src/guacenc check \
    && make install \
    && test -x /usr/local/bin/guacenc


FROM alpine:${ALPINE_VERSION} AS production-image

RUN apk add --no-cache \
        cairo \
        ffmpeg-libavcodec \
        ffmpeg-libavformat \
        ffmpeg-libavutil \
        ffmpeg-libswscale \
        libjpeg-turbo \
        libpng \
        libwebp \
        libuuid \
    && addgroup -g 1000 srv \
    && adduser -D -u 1000 -G srv srv

COPY --from=builder /usr/local/bin/guacenc /usr/local/bin/guacenc
COPY --from=builder /usr/local/lib/libguac.so* /usr/local/lib/

ARG IMAGE_ID
ENV IMAGE_ID="${IMAGE_ID}" \
    LD_LIBRARY_PATH=/usr/local/lib \
    LANG=C.UTF-8

USER srv

ENTRYPOINT ["/usr/local/bin/guacenc"]
