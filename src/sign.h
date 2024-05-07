#include <string>
#include <tuple>

namespace sign
{
    void InitSignCall();
    std::tuple<std::string, std::string, std::string> CallSign(const std::string_view cmd, const std::string_view src, int seq);
}