#include <string>

class Error {
    private:
    std::string desc;
    int32_t code;

    public:
    // the main constructor
    Error(std::string desc, int32_t code = -1);
};