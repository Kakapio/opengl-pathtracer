#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char** argv) {
  if (argc != 2) {
    cerr << "expected one argument\n";
    exit(EXIT_FAILURE);
  }

  ifstream ppmIn(argv[1]);
  if (!ppmIn.is_open()) {
    cerr << "could not open file '" << argv[1] << "'\n";
    exit(EXIT_FAILURE);
  }
  ofstream ppmOut(string(argv[1]) + ".flipped");
  ostream &out = ppmOut;

  string str;
  ppmIn >> str;
  out << str << endl;

  int width, height, maxVal;
  ppmIn >> width >> height >> maxVal;
  out << width << ' ' << height << endl << maxVal << endl;

  int* pixelData = new int[width * height * 3];

  int r,g,b;
  int col = 0;
  /*
  while (ppmIn >> r) {
    ppmIn >> g >> b;
    if (col == 0) rows.push_back(new int[width * 3]);
    rows.back()[col] = r;
    rows.back()[col+1] = g;
    rows.back()[col+2] = b;

    col += 3;
    if (col >= width * 3) col = 0;
  }
  */
  for (int jj = 0; jj < height; ++jj) {
    for (int ii = 0; ii < width; ++ii) {
      ppmIn >> r >> g >> b;

      int index = (jj * width + ii) * 3;
      pixelData[index] = r;
      pixelData[index+1] = g;
      pixelData[index+2] = b;
    }
  }

  const int maxPixelsInRow = 5;
  int pixelsInRow = 0;
  for (int jj = height - 1; jj >= 0; --jj) {
    for (int ii = 0; ii < width; ++ii, pixelsInRow++) {
      if (pixelsInRow >= maxPixelsInRow) {
        out << endl;
        pixelsInRow = 0;
      }
      int *pixel = pixelData + ((jj * width + ii) * 3);
      out << pixel[0] << ' ' << pixel[1] << ' ' << pixel[2] << ' ';
    }
  }

  delete[] pixelData;

  return 0;
}
