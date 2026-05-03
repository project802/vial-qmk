CUSTOM_MATRIX = yes

F_CPU = 8000000

BOOTLOADER_SIZE = 6144

# project specific files
SRC ?=	matrix.c \
        led.c \
        light_ws2812.c \
        rgblight.c
