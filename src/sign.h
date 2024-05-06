#include <string>
#include <tuple>

namespace sign
{
    void InitSignCall();
    std::tuple<std::string, std::string, std::string> CallSign(const std::string cmd, const std::string src, const int seq);
}