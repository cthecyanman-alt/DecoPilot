#include "LevelAnalyzer.hpp"
#include <Geode/utils/cocos.hpp>
#include <limits>

using namespace geode::prelude;

namespace decopilot {

std::string LevelAnalyzer::summarize(LevelEditorLayer* editor) {
    if (!editor || !editor->m_objects) return "No level objects available.";

    std::map<int, int> histogram;
    int solids = 0;
    int slopes = 0;
    int hazards = 0;
    int decorations = 0;
    int total = 0;

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();

    for (auto* obj : editor->m_objects->asExt<GameObject>()) {
        if (!obj) continue;
        ++total;
        histogram[obj->m_objectID]++;
        auto p = obj->getPosition();
        minX = std::min(minX, p.x);
        minY = std::min(minY, p.y);
        maxX = std::max(maxX, p.x);
        maxY = std::max(maxY, p.y);

        switch (obj->m_objectType) {
            case GameObjectType::Solid:
            case GameObjectType::Breakable:
                ++solids; break;
            case GameObjectType::Slope:
                ++slopes; break;
            case GameObjectType::Hazard:
            case GameObjectType::AnimatedHazard:
                ++hazards; break;
            case GameObjectType::Decoration:
                ++decorations; break;
            default:
                break;
        }
    }

    if (total == 0) return "Empty level.";

    std::vector<std::pair<int, int>> common(histogram.begin(), histogram.end());
    std::sort(common.begin(), common.end(), [](auto const& a, auto const& b) {
        return a.second > b.second;
    });
    if (common.size() > 24) common.resize(24);

    std::string ids;
    for (auto const& [id, count] : common) {
        if (!ids.empty()) ids += ", ";
        ids += fmt::format("{}:{}", id, count);
    }

    return fmt::format(
        "LEVEL SUMMARY\n"
        "objects={} solids={} slopes={} hazards={} existing_deco={}\n"
        "bounds=({:.0f},{:.0f})..({:.0f},{:.0f}) size=({:.0f}x{:.0f})\n"
        "most_common_object_ids={}\n"
        "Hard rule: original objects are immutable; decoration is additive only.",
        total, solids, slopes, hazards, decorations,
        minX, minY, maxX, maxY, maxX - minX, maxY - minY, ids
    );
}

} // namespace decopilot
