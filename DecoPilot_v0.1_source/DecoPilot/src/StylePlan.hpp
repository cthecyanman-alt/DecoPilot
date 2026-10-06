#pragma once
#include <Geode/Geode.hpp>
#include <algorithm>
#include <cmath>

namespace decopilot {

struct RGB {
    int r = 255;
    int g = 80;
    int b = 190;
};

struct StylePlan {
    RGB primary {255, 47, 179};
    RGB secondary {37, 196, 255};
    RGB accent {139, 92, 255};
    RGB background {19, 21, 43};

    float outlineStrength = 0.80f;
    float glowStrength = 0.45f;
    float panelDensity = 0.70f;
    float airDecoDensity = 0.18f;
    float backgroundDensity = 0.25f;
    float variation = 0.45f;
    float readability = 0.95f;
    std::string style = "modern-neon";
    std::string notes;
};

inline float clamp01(float v) {
    return std::clamp(v, 0.0f, 1.0f);
}

inline RGB parseHex(std::string hex, RGB fallback) {
    if (!hex.empty() && hex[0] == '#') hex.erase(hex.begin());
    if (hex.size() != 6) return fallback;
    try {
        auto v = static_cast<unsigned int>(std::stoul(hex, nullptr, 16));
        return RGB {
            static_cast<int>((v >> 16) & 0xFF),
            static_cast<int>((v >> 8) & 0xFF),
            static_cast<int>(v & 0xFF)
        };
    } catch (...) {
        return fallback;
    }
}

inline StylePlan parseStylePlan(matjson::Value const& root) {
    StylePlan plan;

    if (root.contains("style")) plan.style = root["style"].asString().unwrapOr(plan.style);
    if (root.contains("notes")) plan.notes = root["notes"].asString().unwrapOr("");

    auto readColor = [&](char const* key, RGB fallback) {
        if (!root.contains(key)) return fallback;
        auto value = root[key];
        if (value.isString()) return parseHex(value.asString().unwrapOr(""), fallback);
        if (value.isObject()) {
            RGB out = fallback;
            out.r = static_cast<int>(value["r"].asInt().unwrapOr(out.r));
            out.g = static_cast<int>(value["g"].asInt().unwrapOr(out.g));
            out.b = static_cast<int>(value["b"].asInt().unwrapOr(out.b));
            out.r = std::clamp(out.r, 0, 255);
            out.g = std::clamp(out.g, 0, 255);
            out.b = std::clamp(out.b, 0, 255);
            return out;
        }
        return fallback;
    };

    plan.primary = readColor("primary", plan.primary);
    plan.secondary = readColor("secondary", plan.secondary);
    plan.accent = readColor("accent", plan.accent);
    plan.background = readColor("background", plan.background);

    auto read01 = [&](char const* key, float fallback) {
        if (!root.contains(key)) return fallback;
        return clamp01(static_cast<float>(root[key].asDouble().unwrapOr(fallback)));
    };

    plan.outlineStrength = read01("outlineStrength", plan.outlineStrength);
    plan.glowStrength = read01("glowStrength", plan.glowStrength);
    plan.panelDensity = read01("panelDensity", plan.panelDensity);
    plan.airDecoDensity = read01("airDecoDensity", plan.airDecoDensity);
    plan.backgroundDensity = read01("backgroundDensity", plan.backgroundDensity);
    plan.variation = read01("variation", plan.variation);
    plan.readability = read01("readability", plan.readability);

    return plan;
}

} // namespace decopilot
