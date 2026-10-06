#include "OllamaClient.hpp"

using namespace geode::prelude;

namespace decopilot {

void OllamaClient::plan(
    std::string const& model,
    std::string const& userPrompt,
    std::string const& levelSummary,
    Callback callback
) {
    matjson::Value body = matjson::Value::object();
    body["model"] = model;
    body["stream"] = false;
    body["format"] = "json";

    matjson::Value options = matjson::Value::object();
    options["temperature"] = 0.20;
    body["options"] = options;

    matjson::Value messages = matjson::Value::array();

    matjson::Value system = matjson::Value::object();
    system["role"] = "system";
    system["content"] = R"PROMPT(
You are the STYLE PLANNER for DecoPilot, a Geometry Dash decoration-only tool.
You NEVER output Geometry Dash object IDs, coordinates, commands, code, prose outside JSON, or gameplay changes.
The deterministic engine will place safe decoration later. Your only job is to translate the user's artistic request and level summary into a coherent style plan.

Return ONE JSON object with exactly these fields:
{
  "style": "short style name",
  "primary": "#RRGGBB",
  "secondary": "#RRGGBB",
  "accent": "#RRGGBB",
  "background": "#RRGGBB",
  "outlineStrength": 0.0-1.0,
  "glowStrength": 0.0-1.0,
  "panelDensity": 0.0-1.0,
  "airDecoDensity": 0.0-1.0,
  "backgroundDensity": 0.0-1.0,
  "variation": 0.0-1.0,
  "readability": 0.0-1.0,
  "notes": "one short sentence"
}

Prioritize readability and consistent visual language. The original gameplay is immutable and is not yours to redesign.
)PROMPT";
    messages.push(system);

    matjson::Value user = matjson::Value::object();
    user["role"] = "user";
    user["content"] = fmt::format(
        "USER ART DIRECTION:\n{}\n\n{}\n\nChoose a palette and densities that fit the request. JSON only.",
        userPrompt, levelSummary
    );
    messages.push(user);
    body["messages"] = messages;

    web::WebRequest req;
    req.header("Content-Type", "application/json");
    req.timeout(std::chrono::seconds(180));
    req.bodyJSON(body);

    m_request.spawn(
        "DecoPilot Ollama style planning",
        req.post("http://127.0.0.1:11434/api/chat"),
        [callback = std::move(callback)](web::WebResponse response) mutable {
            if (!response.ok()) {
                callback(Err(fmt::format(
                    "Ollama returned HTTP {}. Is Ollama running and is the model installed?",
                    response.code()
                )));
                return;
            }

            auto outerRes = response.json();
            if (outerRes.isErr()) {
                callback(Err(fmt::format("Ollama returned invalid JSON: {}", outerRes.unwrapErr())));
                return;
            }
            auto outer = outerRes.unwrap();
            auto content = outer["message"]["content"].asString().unwrapOr("");
            if (content.empty()) {
                callback(Err("Ollama returned an empty style plan."));
                return;
            }

            auto planRes = matjson::parse(content);
            if (planRes.isErr()) {
                callback(Err(fmt::format("Model did not return valid JSON: {}", planRes.unwrapErr())));
                return;
            }

            callback(Ok(parseStylePlan(planRes.unwrap())));
        }
    );
}

} // namespace decopilot
