APP := pacman
SRC_DIR := Allegro_pacman/Allegro_pacman/Src
BUILD_DIR := build
ASSET_DIR := Allegro_pacman/Allegro_pacman/Assets

CC ?= gcc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra
PKGS := allegro-5 allegro_main-5 allegro_image-5 allegro_font-5 allegro_ttf-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5 allegro_dialog-5
LDLIBS := $(shell pkg-config --libs $(PKGS)) -lm
CPPFLAGS := -I$(SRC_DIR) $(shell pkg-config --cflags $(PKGS))
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

.PHONY: all clean run

all: $(BUILD_DIR)/$(APP)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(APP): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDLIBS)

run: all
	cd Allegro_pacman/Allegro_pacman && ../../$(BUILD_DIR)/$(APP)

clean:
	rm -rf $(BUILD_DIR)
