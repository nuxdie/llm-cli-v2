#include <iostream>
#include <replxx.hxx>
#include <string>

int main() {
  replxx::Replxx rx;
  rx.install_window_change_handler(); // Automatically handles terminal resizing
  rx.set_max_history_size(100);

  std::string prompt = "λ ";

  while (true) {
    const char *c_input = rx.input(prompt);

    if (c_input == nullptr) {
      // User pressed Ctrl+D (EOF) or Ctrl+C
      break;
    }

    std::string input{c_input};

    if (input.empty())
      continue;
    if (input == "exit")
      break;

    rx.history_add(input);

    // Process the user's input
    std::cout << "You sent: \n" << input << "\n\n";
  }

  return 0;
}
