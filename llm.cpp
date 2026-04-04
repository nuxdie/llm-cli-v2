#include <cstdio>
#include <print>
#include <string>
#include <termios.h>
#include <unistd.h>

struct RawMode {
  termios orig;
  RawMode() {
    tcgetattr(STDIN_FILENO, &orig);
    termios raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON); // no echo, byte-by-byte
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
  }
  ~RawMode() { tcsetattr(STDIN_FILENO, TCSANOW, &orig); }
};

int main() {
  RawMode term;
  std::setvbuf(stdout, nullptr, _IONBF, 0);
  std::print("(alt+enter to send)> ");

  std::string prompt;
  char c;
  while (read(STDIN_FILENO, &c, 1) == 1) {
    if (c == '\x1b')
      break;                     // Alt+Enter sends ESC
    if (c == 127 || c == '\b') { // Backspace
      if (!prompt.empty()) {
        prompt.pop_back();
        std::print("\b \b"); // erase on screen
      }
    } else {
      prompt += c;
      std::print("{}", c); // echo
    }
  }
}
