
// this is not a standalone header file

class Percentile
{
 public:
  Percentile(std::vector<int>& latencies, int infly)
  {
    stat << "recv " << muduo::Fmt("%6zd", latencies.size()) << " in-fly " << infly;

    if (!latencies.empty())
    {
      std::sort(latencies.begin(), latencies.end());
      int min = latencies.front();
      int max = latencies.back();
      int sum = std::accumulate(latencies.begin(), latencies.end(), 0);
      int mean = sum / static_cast<int>(latencies.size());
      int median = getPercentile(latencies, 50);
      int p90 = getPercentile(latencies, 90);
      int p99 = getPercentile(latencies, 99);
      stat << " min " << min
           << " max " << max
           << " avg " << mean
           << " median " << median
           << " p90 " << p90
           << " p99 " << p99;
    }
  }

  const muduo::LogStream::Buffer& report() const
  {
    return stat.buffer();
  }

  void save(const std::vector<int>& latencies, muduo::StringArg name) const
  {
    muduo::FileUtil::AppendFile f(name);
    for (size_t i = 0; i < latencies.size(); ++i)
    {
      char buf[64];
      int n = snprintf(buf, sizeof buf, "%d\n", latencies[i]);
      f.append(buf, n);
    }
  }

 private:

  static int getPercentile(const std::vector<int>& latencies, int percent)
  {
    // The Nearest Rank method
    assert(latencies.size() > 0);
    size_t idx = 0;
    if (percent > 0)
    {
      idx = (latencies.size() * percent + 99) / 100 - 1;
      assert(idx < latencies.size());
    }
    return latencies[idx];
  }

  muduo::LogStream stat;
};
