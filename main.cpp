#include <chrono>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>

class Field {
public:
  Field(unsigned height, unsigned width, char c = ' ')
      : m_width(width), m_height(height) {
    m_str = new char[width * height + 1]();
    std::fill(m_str, m_str + width * height + 1, c);
    m_str[width * height] = '\0';
  }

  class Proxy {
  public:
    Proxy(char *str, unsigned width, unsigned row)
        : _m_str(str), _m_width(width), _m_row(row) {}

    char &operator[](unsigned col) {
      if (col < 0 || col >= _m_width)
        throw std::out_of_range("Column out of range");
      return _m_str[_m_row * _m_width + col];
    }

  private:
    char *_m_str;
    unsigned _m_width;
    unsigned _m_row;
  };

  Proxy operator[](unsigned row) {
    if (row < 0 || row >= m_height)
      throw std::out_of_range("Row out of range");
    return Proxy(m_str, m_width, row);
  }

  const char* data() const { return m_str; }

private:
  unsigned m_width, m_height;
  char* m_str;
};

class Point {
public:
  Point(double x, double y) : _x(x), _y(y) {}

  void normalize(double max_x, double max_y) {
    _x = _x / max_x * 2 - 1;
    _y = _y / max_y * 2 - 1;
  }

  void shift_x(double shift) { _x += shift; }
  void shift_y(double shift) { _y += shift; }

  double dist(const Point &p, double aspect = 1) {
    return sqrt((_x - p._x) * (_x - p._x) +
                (_y - p._y * aspect) * (_y - p._y * aspect));
  }

  double dist(double x, double y, double aspect = 1) {
    return sqrt((_x - x) * (_x - x) + (_y - y * aspect) * (_y - y * aspect));
  }

  void print() { printf("[%f, %f]\n", _x, _y); }

private:
  double _x;
  double _y;
};

void set_cursor_position(int x, int y) { printf("\033[%d;%dH", x + 1, y + 1); }

void clean_terminal() { printf("\033[2J"); }

double sin_oscilate(double coeff, int shifts, double border = 0)
{
    double shift = sin(2 * M_PI * shifts * coeff);
    shift = shift > 0 ? shift - border : shift + border;
    return shift;
}

int main() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  unsigned height = w.ws_row, width = w.ws_col;

  double radius = 0.4;
  unsigned iters = 1000;
  unsigned shifts = 10;

  double terminal_aspect = (double)width / height;
  double symbol_aspect = 11.0 / 24;
  double aspect = terminal_aspect * symbol_aspect;

  Field field(height, width);
  char c;

  clean_terminal();

  for (int iter = 0; iter < iters; iter++) {
    for (unsigned i = 0; i < height; i++) {
      for (unsigned j = 0; j < width; j++) {
        Point p(i, j);
        p.normalize(height, width);

        /* Point center(0, sin_oscilate((double)iter / iters, shifts)); */
        Point center(0, 0);

        c = (center.dist(p, aspect) <= radius) ? '@' : ' ';

        /* if (c != field[i][j]) { */
        /*   set_cursor_position(i, j); */
        /*   putchar(c); */
          field[i][j] = c;
        /* } */
      }
    }

    printf("%s", field.data());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  return 0;
}

