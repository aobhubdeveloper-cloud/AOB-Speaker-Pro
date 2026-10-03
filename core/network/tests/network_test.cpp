#include "aob/jitter.hpp"
#include "aob/sync.hpp"
#include "aob/telemetry.hpp"
#include <array>
#include <cassert>
#include <cmath>

int main() {
    using aob::JitterBuffer;

    JitterBuffer<4> jb;
    std::array<float, 1920> a{};
    a[0] = 0.25f;
    a[1919] = -0.25f;

    assert(jb.push(0, a.data(), a.size()));
    auto f = jb.pop();
    assert(f.has_value());
    assert(std::abs((*f)[0] - 0.25f) < 1e-6f);
    assert(std::abs((*f)[1919] + 0.25f) < 1e-6f);

    // Out-of-order packets wait for the expected sequence.
    assert(jb.push(2, a.data(), a.size()));
    assert(!jb.pop().has_value());
    assert(jb.push(1, a.data(), a.size()));
    assert(jb.pop().has_value());
    assert(jb.pop().has_value());

    // Late packets are rejected.
    assert(!jb.push(1, a.data(), a.size()));

    // Oversized frames cannot corrupt the ring.
    assert(!jb.push(3, a.data(), a.size() + 1));

    aob::Telemetry t{};
    t.received = 90;
    t.lost = 10;
    assert(std::abs(aob::loss_percent(t) - 10.0f) < 1e-6f);

    aob::SyncState s{};
    s.rate = 1.000125;
    assert(std::abs(aob::correction_ppm(s) - 125.0) < 1e-9);

    return 0;
}
