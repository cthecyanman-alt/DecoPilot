#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include "StylePlan.hpp"
#include <functional>

namespace decopilot {

class OllamaClient {
    geode::async::TaskHolder<geode::utils::web::WebResponse> m_request;
public:
    using Callback = std::function<void(geode::Result<StylePlan>)>;

    void plan(
        std::string const& model,
        std::string const& userPrompt,
        std::string const& levelSummary,
        Callback callback
    );

    bool isPending() const { return m_request.isPending(); }
    void cancel() { m_request.cancel(); }
};

} // namespace decopilot
