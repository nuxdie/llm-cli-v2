#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <print>
#include <replxx.hxx>
#include <string>

using json = nlohmann::json;

void send_api_request(std::string input) {
  json req_body = {{"message", std::move(input)}};
  auto res = cpr::Post(
      cpr::Url{"https://httpbin.org/post"},
      cpr::Body{req_body.dump()},
      cpr::Header{{"Content-Type", "application/json"}}
  );

  if (res.status_code == 200) {
    try {
      auto res_json = json::parse(res.text);
      std::print("API Resp:\n{}\n\n", res_json["json"].dump(2));
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

  const std::string prompt = "λ ";

  while (const char *c_input = rx.input(prompt)) {
    std::string input{c_input};

    if (input.empty()) continue;
    if (input == "exit") break;

    rx.history_add(input);
    send_api_request(std::move(input));
  }

  return 0;
}
