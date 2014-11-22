#include <muduo/base/Types.h>
#include <vector>
#include <boost/noncopyable.hpp>

typedef struct gdImageStruct* gdImagePtr;

class Plot : boost::noncopyable
{
 public:
  Plot(int width, int height, int totalSeconds, int samplingPeriod);
  ~Plot();
  muduo::string plotCpu(const std::vector<double> data);

 private:
  muduo::string toPng();
  // pair<shared_ptr<void*>, int> toPng();
  int getX(long x, long total) const;
  int getY(double value) const;
  void label(double maxValue);

  struct MyGdFont;
  typedef struct MyGdFont* MyGdFontPtr;

  const int width_;
  const int height_;
  const int totalSeconds_;
  const int samplingPeriod_;
  const gdImagePtr image_;
  const MyGdFontPtr font_;  // no way to forward declaration gdFontPtr
  const int fontWidth_;
  const int fontHeight_;
  const int white_;
  const int black_;
  const int gray_;
  const int blue_;

  const int kRightMargin_;
  static const int kLeftMargin_ = 5;
  static const int kMarginY_ = 5;

  const double ratioX_;
};

