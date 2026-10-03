#include "aob/dsp.hpp"
#include <array>
#include <cassert>
#include <cmath>

static bool near(float a, float b, float eps = 1e-4f) {
    return std::fabs(a - b) <= eps;
}

int main() {
    using namespace aob;

    DspEngine dsp;

    // Disabled DSP must leave samples unchanged.
    DspSettings disabled{};
    disabled.enabled = false;
    dsp.configure(disabled);
    std::array<float, 4> samples{0.1f, -0.1f, 0.2f, -0.2f};
    dsp.process(samples.data(), 2, 2);
    assert(near(samples[0], 0.1f));
    assert(near(samples[3], -0.2f));

    // Output gain must be applied when DSP is enabled.
    DspSettings gain{};
    gain.enabled = true;
    gain.output_db = 6.0f;
    gain.limiter_db = 1.0f;
    dsp.configure(gain);
    samples = {0.1f, -0.1f, 0.2f, -0.2f};
    dsp.process(samples.data(), 2, 2);
    assert(samples[0] > 0.1f);
    assert(samples[1] < -0.1f);

    // Limiter must constrain positive and negative peaks.
    DspSettings limited{};
    limited.enabled = true;
    limited.output_db = 20.0f;
    limited.limiter_db = -1.0f;
    dsp.configure(limited);
    samples = {1.0f, -1.0f, 0.5f, -0.5f};
    dsp.process(samples.data(), 2, 2);
    const float limit = std::pow(10.0f, -1.0f / 20.0f);
    assert(std::fabs(samples[0]) <= limit + 1e-4f);
    assert(std::fabs(samples[1]) <= limit + 1e-4f);

    // Compressor must reduce a peak without changing its sign.
    DspSettings compressed{};
    compressed.enabled = true;
    compressed.compressor = 0.5f;
    compressed.limiter_db = 1.0f;
    dsp.configure(compressed);
    samples = {1.0f, -1.0f, 0.2f, -0.2f};
    dsp.process(samples.data(), 2, 2);
    assert(samples[0] > 0.7f && samples[0] < 1.0f);
    assert(samples[1] < -0.7f && samples[1] > -1.0f);

    // Presets must configure the documented settings.
    dsp.set_preset(Preset::Music);
    assert(near(dsp.settings().bass_db, 2.0f));
    assert(near(dsp.settings().treble_db, 1.0f));

    dsp.set_preset(Preset::BassBoost);
    assert(near(dsp.settings().bass_db, 6.0f));

    dsp.set_preset(Preset::AobMax);
    assert(near(dsp.settings().bass_db, 4.0f));
    assert(near(dsp.settings().treble_db, 2.0f));
    assert(dsp.settings().compressor > 0.0f);

    // Null input and zero channels must be safe no-ops.
    dsp.process(nullptr, 4, 2);
    samples = {0.1f, 0.2f, 0.3f, 0.4f};
    dsp.process(samples.data(), 2, 0);
    assert(near(samples[0], 0.1f));

    return 0;
}
