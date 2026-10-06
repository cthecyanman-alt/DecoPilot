#pragma once
#include <Geode/Geode.hpp>
#include <map>
#include <string>

namespace decopilot {

class LevelAnalyzer {
public:
    static std::string summarize(LevelEditorLayer* editor);
};

} // namespace decopilot
