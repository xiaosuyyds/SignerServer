#include <string>
#include <tuple>

class Sign
{
public:
    Sign();

private:
    typedef int (*SignFunctionType)(const char *cmd, const unsigned char *src, size_t src_len, int seq, unsigned char *result);
    SignFunctionType SignFunction = nullptr;
    void Init();

public:
    std::tuple<std::string, std::string, std::string> Call(const std::string_view cmd, const std::string_view src, int seq);
};