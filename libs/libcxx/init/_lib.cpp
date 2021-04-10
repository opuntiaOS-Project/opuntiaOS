namespace std {
extern int _ios_init();
extern int _ios_deinit();
} // namespace std

void _libcpp_init()
{
    std::_ios_init();
}

void _libcpp_deinit()
{
    std::_ios_deinit();
}
