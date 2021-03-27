#include <stdio.h>
#include <math.h>

#if PICO_ON_DEVICE

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"

#endif

#include "pico/stdlib.h"

#include "pico/audio_i2s.h"
#include "macro_oscillator.h"



#define SINE_WAVE_TABLE_LEN 2048
#define SAMPLES_PER_BUFFER 256

using namespace braids;

static int16_t sine_wave_table[SINE_WAVE_TABLE_LEN];
static uint8_t sync_buffer[SAMPLES_PER_BUFFER];

struct audio_buffer_pool *init_audio() {

    static audio_format_t audio_format = {
            .sample_freq = 24000,
            .format = AUDIO_BUFFER_FORMAT_PCM_S16,
            .channel_count = 1
    };

    static struct audio_buffer_format producer_format = {
            .format = &audio_format,
            .sample_stride = 2
    };

    struct audio_buffer_pool *producer_pool = audio_new_producer_pool(&producer_format, 3,
                                                                      SAMPLES_PER_BUFFER); // todo correct size
    bool __unused ok;
    const struct audio_format *output_format;
    struct audio_i2s_config config = {
            .data_pin = PICO_AUDIO_I2S_DATA_PIN,
            .clock_pin_base = PICO_AUDIO_I2S_CLOCK_PIN_BASE,
            .dma_channel = 0,
            .pio_sm = 0,
    };

    output_format = audio_i2s_setup(&audio_format, &config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }

    ok = audio_i2s_connect(producer_pool);
    assert(ok);
    audio_i2s_set_enabled(true);
    return producer_pool;
}

MacroOscillator osc;

int main() {

    stdio_init_all();
    int counter = 0;
    struct audio_buffer_pool *ap = init_audio();
    osc.Init();
    osc.set_shape(MACRO_OSC_SHAPE_SAW_SWARM);

    while (true) {
        counter = counter+10;
        struct audio_buffer *buffer = take_audio_buffer(ap, true);  
        //struct audio_buffer *mixBuffer = take_audio_buffer(ap, true);  
        memset(sync_buffer, 0, SAMPLES_PER_BUFFER);
        int i=counter;
        uint16_t tri = (i * 3);
        uint16_t tri2 = (i * 11);
        uint16_t ramp = i * 150;
        tri = tri > 32767 ? 65535 - tri : tri;
        tri2 = tri2 > 32767 ? 65535 - tri2 : tri2;
        osc.set_parameters(tri, 0);
        osc.set_pitch((48 << 7));
        osc.Render(sync_buffer, (int16_t *) buffer->buffer->bytes, buffer->max_sample_count);
        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(ap, buffer);
    }
    return 0;
}