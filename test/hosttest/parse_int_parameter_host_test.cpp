#include "parse_int_parameter.h"
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>

static void Expect(bool cond, const char *msg)
{
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", msg);
        std::exit(1);
    }
}

static bool LeftoverStoiAborts(const std::string &s)
{
    try {
        (void)std::stoi(s);
        return false;
    } catch (const std::invalid_argument &) {
        return true;
    } catch (const std::out_of_range &) {
        return true;
    }
}

int main()
{
    int v = -1;
    Expect(ParseIntParameter("0", v) && v == 0, "zero");
    Expect(ParseIntParameter("1", v) && v == 1, "gnss_enable");
    Expect(ParseIntParameter("7275", v) && v == 7275, "agnss_port");
    Expect(ParseIntParameter("65535", v) && v == 65535, "maxport");
    Expect(ParseIntParameter("2147483647", v) && v == 2147483647, "intmax");
    Expect(!ParseIntParameter("", v), "empty");
    Expect(!ParseIntParameter("abc", v), "abc");
    Expect(!ParseIntParameter("12a", v), "12a");
    Expect(!ParseIntParameter("9999999999999999999", v), "huge");
    Expect(!ParseIntParameter("2147483648", v), "overflow");
    Expect(!ParseIntParameter(" 1", v), "space");
    Expect(LeftoverStoiAborts("9999999999999999999"), "leftover huge throws");
    Expect(LeftoverStoiAborts("2147483648"), "leftover overflow throws");
    std::puts("ok");
    return 0;
}
