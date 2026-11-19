// Copyright 2026 Chris Vincent (@project802)
// SPDX-License-Identifier: GPL-3.0

#pragma once

// 75 ms dots at 200 BPM: C (-.-.) followed by Q (--.-).
#define CW_STARTUP_TEMPO 200
#define STARTUP_SONG SONG( \
    HD_NOTE(_F5), Q__NOTE(_REST), Q__NOTE(_F5), Q__NOTE(_REST), \
    HD_NOTE(_F5), Q__NOTE(_REST), Q__NOTE(_F5), HD_NOTE(_REST), \
    HD_NOTE(_F5), Q__NOTE(_REST), HD_NOTE(_F5), Q__NOTE(_REST), \
    Q__NOTE(_F5), Q__NOTE(_REST), HD_NOTE(_F5) \
)
