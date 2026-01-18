// SPDX-FileCopyrightText: 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#include <lui/ranged.hpp>

namespace lui {

/** A type of dial. Like a Knob on a synth.
    @ingroup widgets
    @headerfile lui/slider.hpp
*/
class LUI_API Dial : public Ranged {
public:
    Dial();
    virtual ~Dial();

private:
    class Impl;
    std::unique_ptr<Impl> impl;

    /** @private */
    bool obstructed (int x, int y) override { return true; }
    /** @private */
    void paint (Graphics& g) override;
    /** @private */
    void resized() override;
    /** @private */
    void pressed (const Event&) override;
    /** @private */
    void drag (const Event&) override;
};

} // namespace lui
