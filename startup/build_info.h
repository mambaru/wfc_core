extern const char* startup_build_info_string;
struct startup_build_info{ const char* operator()() const { return startup_build_info_string;} };
