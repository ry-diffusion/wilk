setup:
  meson setup build

build:
  meson compile -C build


run: build
  ./build/wilk
