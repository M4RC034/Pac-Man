FROM debian:bookworm-slim AS build

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        liballegro-acodec5-dev \
        liballegro-audio5-dev \
        liballegro-dialog5-dev \
        liballegro-image5-dev \
        liballegro-ttf5-dev \
        liballegro5-dev \
        pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY Makefile ./
COPY Allegro_pacman ./Allegro_pacman
RUN make

FROM debian:bookworm-slim

ENV DISPLAY=:1 \
    ALSOFT_DRIVERS=null \
    PORT=7860 \
    VNC_RESOLUTION=900x900 \
    VNC_DEPTH=24

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        fluxbox \
        liballegro-acodec5.2 \
        liballegro-audio5.2 \
        liballegro-dialog5.2 \
        liballegro-image5.2 \
        liballegro-ttf5.2 \
        liballegro5.2 \
        novnc \
        websockify \
        x11vnc \
        xvfb \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /app/build/pacman ./pacman
COPY Allegro_pacman/Allegro_pacman/Assets ./Assets
COPY scripts/start-novnc.sh ./scripts/start-novnc.sh

RUN chmod +x ./scripts/start-novnc.sh

EXPOSE 7860
CMD ["./scripts/start-novnc.sh"]
