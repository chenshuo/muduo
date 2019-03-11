
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
    if (latencies.empty())
      return;
    muduo::FileUtil::AppendFile f(name);
    f.append("# ", 2);
    f.append(stat.buffer().data(), stat.buffer().length());
    f.append("\n", 1);

    const int kInterval = 5; // 5 us per bucket
    int low = latencies.front() / kInterval * kInterval;
    int count = 0;
    int sum = 0;
    const double total = static_cast<double>(latencies.size());
    char buf[64];
#ifndef NDEBUG
    for (size_t i = 0; i < latencies.size(); ++i)
    {
      int n = snprintf(buf, sizeof buf, "# %d\n", latencies[i]);
      f.append(buf, n);
    }
#endif
    // FIXME: improve this O(N) algorithm, maybe use lower_bound().
    for (size_t i = 0; i < latencies.size(); ++i)
    {
      if (latencies[i] < low + kInterval)
        ++count;
      else
      {
        sum += count;
        int n = snprintf(buf, sizeof buf, "%4d %5d %5.2f\n", low, count, 100 * sum / total);
        f.append(buf, n);
        low = latencies[i] / kInterval * kInterval;
        assert(latencies[i] < low + kInterval);
        count = 1;
      }
    }
    sum += count;
    assert(sum == total);
    int n = snprintf(buf, sizeof buf, "%4d %5d %5.1f\n", low, count, 100 * sum / total);
    f.append(buf, n);
  }

 private:

  static int getPercentile(const std::vector<int>& latencies, int percent)
  {
    // The Nearest Rank method
    assert(!latencies.empty());
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
