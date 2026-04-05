#include <algorithm>
#include <asm-generic/ioctls.h>
#include <cstdio>
#include <print>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <utility>
#include <vector>

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

int terminalWidth() {
  winsize ws{};
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  return ws.ws_col > 0 ? ws.ws_col : 80;
}

struct Line {
  int start; // index into prompt
  int len;   // number of chars (excl. '\n'if present)
};

// Splits the prompt into lines, respecting both '\n' and terminal wrap
std::vector<Line> buildLineMap(const std::string &s, int width) {
  std::vector<Line> lines;
  int i = 0, n = s.size();
  while (i <= n) {
    Line ln{i, 0};
    while (i < n && s[i] != '\n' && ln.len < width) {
      ++ln.len;
      ++i;
    }
    lines.push_back(ln);
    if (i < n && s[i] == '\n') {
      ++i; // consume newline
    } else if (i == n)
      break;
  }
  if (lines.empty())
    lines.push_back({0, 0});
  return lines;
}

// Given a cursor (index into prompt), return {lineIndex, col}
std::pair<int, int> cursorToLineCol(const std::vector<Line> &lines,
                                    int cursor) {
  for (int i = 0; i < (int)lines.size(); ++i) {
    int end = lines[i].start + lines[i].len;
    if (cursor <= end)
      return {i, cursor - lines[i].start};
  }
  auto &last = lines.back();
  return {(int)lines.size() - 1, last.len};
}

// Given {lineIndex, col}, clamp col to line len and return string index
int lineColToCursor(const std::vector<Line> &lines, int li, int col) {
  li = std::clamp(li, 0, (int)lines.size() - 1);
  col = std::clamp(col, 0, lines[li].len);
  return lines[li].start + col;
}

const std::string PROMPT_PREFIX = "(alt+enter to send)> ";
// Redraw prompt and reposition cursor
void redraw(const std::string &prompt, int cursor, int width) {
  auto lines = buildLineMap(prompt, width - (int)PROMPT_PREFIX.size());

  // move to start of prompt
  int totalLines = lines.size();
  if (totalLines > 1)
    std::print("\x1b[{}A", totalLines - 1);
  std::print("\r");

  // clear below & reprint
  std::print("\x1b[J"); // erase from cursor to screen bottom
  std::print("{}", PROMPT_PREFIX);
  for (int i = 0; i < (int)lines.size(); ++i) {
    std::print("{}", prompt.substr(lines[i].start, lines[i].len));
    if (i + 1 < (int)lines.size())
      std::print("\r\n");
  }

  // reposition terminal cursor
  auto [li, col] = cursorToLineCol(lines, cursor);
  int fromBottom = (int)lines.size() - 1 - li;
  if (fromBottom > 0)
    std::print("\x1b[{}A", fromBottom); // move up
  int targetCol = (li == 0 ? (int)PROMPT_PREFIX.size() : 0) + col;
  std::print("\r\x1b[{}C", targetCol); // move right to col
}

// Returns 'A'-'D' for arrow keys, 0 for plain ESC, -1 on read err
int readEscapeSeq() {
  char seq[2];
  // Set a short timeout: if nothing follows ESC,
  // it's a bare ESC keypress
  termios t;
  tcgetattr(STDIN_FILENO, &t);
  t.c_cc[VTIME] = 1; // 100ms timeout
  t.c_cc[VMIN] = 0;  // return immediately if nothing arrives
  tcsetattr(STDIN_FILENO, TCSANOW, &t);

  int n = read(STDIN_FILENO, seq, 1);

  // restore blocking reads
  t.c_cc[VTIME] = 0;
  t.c_cc[VMIN] = 1;
  tcsetattr(STDIN_FILENO, TCSANOW, &t);

  if (n <= 0)
    return 0; // bare ESC
  if (seq[0] != '[')
    return 0; // unknown seq

  if (read(STDIN_FILENO, seq + 1, 1) != 1)
    return -1;
  if (seq[1] >= 'A' && seq[1] <= 'D')
    return seq[1]; // arrow key
  return 0;
}

int main() {
  RawMode term;
  std::setvbuf(stdout, nullptr, _IONBF, 0);
  std::print("{}", PROMPT_PREFIX);

  std::string prompt;
  int cursor = 0;
  int prefferedCol = -1; // "sticky" col

  char c;
  while (read(STDIN_FILENO, &c, 1) == 1) {
    int width = terminalWidth();
    auto lines = buildLineMap(prompt, width - (int)PROMPT_PREFIX.size());
    auto [li, col] = cursorToLineCol(lines, cursor);

    if (c == '\x1b') {
      int arrow = readEscapeSeq();
      if (arrow == 0)
        break;            // bare ESC = Alt+Enter -> done
      if (arrow == 'C') { // →
        if (cursor < (int)prompt.size())
          ++cursor;
        prefferedCol = -1;
      } else if (arrow == 'D') { // ←
        if (cursor > 0)
          --cursor;
        prefferedCol = -1;
      } else if (arrow == 'A' || arrow == 'B') { // ↑ or ↓
        if (prefferedCol < 0)
          prefferedCol = col;
        int targetLine = li + (arrow == 'A' ? -1 : 1);
        if (targetLine >= 0 && targetLine < (int)lines.size()) {
          cursor = lineColToCursor(lines, targetLine, prefferedCol);
        }
      }
    } else if (c == '\n' || c == '\r') { // Enter
      prompt.insert(cursor, 1, '\n');
      ++cursor;
      prefferedCol = -1;
    } else if (c == 127 || c == '\b') { // Backspace
      if (cursor > 0) {
        prompt.erase(cursor - 1, 1);
        --cursor;
        prefferedCol = -1;
      }
    } else {
      prompt.insert(cursor, 1, c);
      ++cursor;
      prefferedCol = -1;
    }

    redraw(prompt, cursor, width);
  }
}
