#include <iostream>
#include <print>
#include <string>

int main() {
  std::print("(alt+enter to send)> ");
  std::string prompt, line;
  while (std::getline(std::cin, line)) {
    if (line.ends_with('\x1b')) { // ESC aka Alt
      prompt += line.substr(0, line.size() -1);
      break;
    }
    prompt += line + '\n';
  }
}
