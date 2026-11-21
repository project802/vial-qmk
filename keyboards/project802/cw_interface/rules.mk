CUSTOM_MATRIX = lite

AUDIO_ENABLE = yes
AUDIO_DRIVER = pwm_software

SRC += matrix.c

DEBOUNCE_TYPE = asym_eager_defer_pk
DEBOUNCE = 30
