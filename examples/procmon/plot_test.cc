#include "plot.h"
#include <vector>
#include <math.h>
#include <stdio.h>

int main()
{
  std::vector<double> cpu_usage;
  for (int i = 0; i < 300; ++i)
    cpu_usage.push_back(1.0 + sin(pow(i / 30.0, 2)));
  Plot plot(600, 100, 600, 2);
  muduo::string png = plot.plotCpu(cpu_usage);

  FILE* fp = fopen("test.png", "wb");
  fwrite(png.data(), 1, png.size(), fp);
  fclose(fp);
}
