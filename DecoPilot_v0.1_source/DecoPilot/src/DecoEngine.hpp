#pragma once
#include <Geode/Geode.hpp>
#include "StylePlan.hpp"

namespace decopilot {

struct GenerationStats {
    int anchors = 0;
    int placed = 0;
    int skipped = 0;
};

class DecoEngine {
public:
    static geode::Result<GenerationStats> generate(LevelEditorLayer* editor, StylePlan const& plan);
};

} // namespace decopilot
