#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <print>
#include <replxx.hxx>
#include <string>

using json = nlohmann::json;

void send_api_request(std::string input) {
  json req_body = {
      {"model", "openrouter/elephant-alpha"},
      {"messages",
       json::array({{{"role", "user"}, {"content", std::move(input)}}})}
  };
  auto res = cpr::Post(
      cpr::Url{"https://openrouter.ai/api/v1/chat/completions"},
      cpr::Body{req_body.dump()},
      cpr::Header{
          {"Content-Type", "application/json"},
          {"Authorization", "Bearer " + std::string(OPENROUTER_API_KEY)}
      }
  );

  if (res.status_code == 200) {
    try {
      auto res_json = json::parse(res.text);
      auto message = res_json["choices"][0]["message"]["content"];
      std::print("> {}\n\n", message.dump());
    } catch (const json::parse_error &err) {
      std::print("Failed to parse API Resp: {}\n\n", err.what());
    }
  } else {
    std::print(
        "API Req Failed!\nStatus code: {}\nError: {}\n\n",
        res.status_code,
        res.error.message
    );
  }
}

int main() {
  replxx::Replxx rx;
  rx.install_window_change_handler(); // Automatically handles terminal resizing
  rx.set_max_history_size(100);

  while (const char *c_input = rx.input("λ ")) {
    std::string input{c_input};

    if (input.empty()) continue;
    if (input == "exit") break;

    rx.history_add(input);
    send_api_request(std::move(input));
  }

  return 0;
}
