#include <stdio.h>
#include <math.h>

#if PICO_ON_DEVICE

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"

#endif

#if USE_AUDIO_I2S
#include "pico/audio_i2s.h"
#elif USE_AUDIO_PWM
#include "pico/audio_pwm.h"
#elif USE_AUDIO_SPDIF
#include "pico/audio_spdif.h"
#endif

#include "macro_oscillator.h"



#define SINE_WAVE_TABLE_LEN 2048
#define SAMPLES_PER_BUFFER 256

using namespace braids;

static int16_t sine_wave_table[SINE_WAVE_TABLE_LEN];
static uint8_t sync_buffer[SAMPLES_PER_BUFFER];

struct audio_buffer_pool *init_audio() {

    static audio_format_t audio_format = {
#if USE_AUDIO_SPDIF
            .sample_freq = 44100,
#else
            .sample_freq = 44100,
#endif
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

#if USE_AUDIO_I2S
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
#elif USE_AUDIO_PWM
    output_format = audio_pwm_setup(&audio_format, -1, &default_mono_channel_config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }
    ok = audio_pwm_default_connect(producer_pool, false);
    assert(ok);
    audio_pwm_set_enabled(true);
#elif USE_AUDIO_SPDIF
    output_format = audio_spdif_setup(&audio_format, &audio_spdif_default_config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }
    //ok = audio_spdif_connect(producer_pool);
    ok = audio_spdif_connect(producer_pool);
    assert(ok);
    audio_spdif_set_enabled(true);
#endif
    return producer_pool;
}

MacroOscillator osc;

int main() {
#if PICO_ON_DEVICE
#if USE_AUDIO_PWM
    set_sys_clock_48mhz();
#endif
#endif


    stdio_init_all();
    int counter = 0;
    struct audio_buffer_pool *ap = init_audio();
    osc.Init();
    osc.set_shape(MACRO_OSC_SHAPE_WAVETABLES);
    uint vol = 128;
    uint32_t step = 0x200000;

    while (true) {
        #if USE_AUDIO_PWM
        enum audio_correction_mode m = audio_pwm_get_correction_mode();
#endif
        int c = getchar_timeout_us(0);
        if (c >= 0) {
            if (c == '-' && vol) vol -= 4;
            if ((c == '=' || c == '+') && vol < 255) vol += 4;
            if (c == '[' && step > 0x10000) step -= 0x10000;
            if (c == ']' && step < (SINE_WAVE_TABLE_LEN / 16) * 0x20000) step += 0x10000;
            if (c == 'q') break;
#if USE_AUDIO_PWM
            if (c == 'c') {
                bool done = false;
                while (!done) {
                    if (m == none) m = fixed_dither;
                    else if (m == fixed_dither) m = dither;
                    else if (m == dither) m = noise_shaped_dither;
                    else if (m == noise_shaped_dither) m = none;
                    done = audio_pwm_set_correction_mode(m);
                }
            }
            printf("vol = %d, step = %d mode = %d      \r", vol, step >>16, m);
#else
            printf("vol = %d, step = %d      \r", vol, step >> 16);
#endif
        }

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