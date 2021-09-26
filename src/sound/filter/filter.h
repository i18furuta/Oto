#pragma once

#include <stdio.h>
#include <stdint.h>

#include "../track.h"
#include "../sound.h"

typedef enum {
    NONE_FILTER = 0,
    FADE_IN,
    FADE_OUT,
    AMP,
    TREMOLO,
    ADSR,
    LOW_PASS,
    HIGH_PASS,
    VIBRATO,
    WAH,
    DELAY,
    REVERB,
} filter_num_t;

struct filter {
    filter_num_t filter_num;
    uint64_t param;
};

typedef struct filter *FILTER;

void init_filter();

void filtering(TRACK t, SOUND s);

void fade_in(TRACK t, double time);
void fade_out(TRACK t, double time);

void amp(TRACK t, double gain);
void tremolo(TRACK t, double speed, double range);