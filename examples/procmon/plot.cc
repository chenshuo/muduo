#include "plot.h"
#include <algorithm>

#include <math.h>

#include <gd.h>
#include <gdfonts.h>

struct Plot::MyGdFont : public gdFont {};

Plot::Plot(int width, int height, int totalSeconds, int samplingPeriod)
  : width_(width),
    height_(height),
    totalSeconds_(totalSeconds),
    samplingPeriod_(samplingPeriod),
    image_(gdImageCreate(width_, height_)),
    font_(static_cast<MyGdFont*>(gdFontGetSmall())),
    fontWidth_(font_->w),
    fontHeight_(font_->h),
    white_(gdImageColorAllocate(image_, 255, 255, 240)),
    black_(gdImageColorAllocate(image_, 0, 0, 0)),
    gray_(gdImageColorAllocate(image_, 200, 200, 200)),
    blue_(gdImageColorAllocate(image_, 128, 128, 255)),
    kRightMargin_(3 * fontWidth_ + 5),
    ratioX_(static_cast<double>(samplingPeriod_ * (width_ - kLeftMargin_ - kRightMargin_)) / totalSeconds_)
{
  // gdImageSetAntiAliased(image_, black_);
}

Plot::~Plot()
{
  gdImageDestroy(image_);
}

muduo::string Plot::plotCpu(const std::vector<double> data)
{
  gdImageFilledRectangle(image_, 0, 0, width_, height_, white_);
  if (data.size() > 1)
  {
    gdImageSetThickness(image_, 2);
    double max = *std::max_element(data.begin(), data.end());
    if (max >= 10.0)
    {
      max = ceil(max);
    }
    else
    {
      max = std::max(0.1, ceil(max*10.0) / 10.0);
    }
    label(max);

    for (size_t i = 0; i < data.size()-1; ++i)
    {
      gdImageLine(image_,
                  getX(i, data.size()),
                  getY(data[i] / max),
                  getX(i+1, data.size()),
                  getY(data[i+1]/max),
                  black_);
    }
  }

  int total = totalSeconds_/samplingPeriod_;
  gdImageSetThickness(image_, 1);
  gdImageLine(image_, getX(0, total), getY(0)+2, getX(total, total), getY(0)+2, gray_);
  gdImageLine(image_, getX(total, total), getY(0)+2, getX(total, total), getY(1)+2, gray_);
  return toPng();
}

void Plot::label(double maxValue)
{
    char buf[64];
    if (maxValue >= 10.0)
      snprintf(buf, sizeof buf, "%.0f", maxValue);
    else
      snprintf(buf, sizeof buf, "%.1f", maxValue);

    gdImageString(image_,
                  font_,
                  width_ - kRightMargin_ + 3,
                  kMarginY_ - 3,
                  reinterpret_cast<unsigned char*>(buf),
                  black_);

    snprintf(buf, sizeof buf, "0");
    gdImageString(image_,
                  font_,
                  width_ - kRightMargin_ + 3,
                  height_ - kMarginY_ - 3 - fontHeight_ / 2,
                  reinterpret_cast<unsigned char*>(buf),
                  gray_);

    snprintf(buf, sizeof buf, "-%ds", totalSeconds_);
    gdImageString(image_,
                  font_,
                  kLeftMargin_,
                  height_ - kMarginY_ - fontHeight_,
                  reinterpret_cast<unsigned char*>(buf),
                  blue_);
}

int Plot::getX(ssize_t i, ssize_t total) const
{
  double x = (width_ - kLeftMargin_ - kRightMargin_) + static_cast<double>(i - total) * ratioX_;
  return static_cast<int>(x + 0.5) + kLeftMargin_;
}

int Plot::getY(double value) const
{
  return static_cast<int>((1.0 - value) * (height_-2*kMarginY_) + 0.5) + kMarginY_;
}

muduo::string Plot::toPng()
{
  int size = 0;
  void* png = gdImagePngPtr(image_, &size);
  muduo::string result(static_cast<char*>(png), size);
  gdFree(png);
  return result;
}
