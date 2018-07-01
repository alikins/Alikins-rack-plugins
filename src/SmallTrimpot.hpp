#include "rack.hpp"

struct SmallPurpleTrimpot : Trimpot {
    SmallPurpleTrimpot();
};

SmallPurpleTrimpot::SmallPurpleTrimpot() : Trimpot() {
    setSVG(SVG::load(assetPlugin(plugin, "res/SmallPurpleTrimpot.svg")));
    shadow->blurRadius = 0.0;
    shadow->opacity = 0.10;
    shadow->box.pos = Vec(0.0, box.size.y * 0.1);
}
