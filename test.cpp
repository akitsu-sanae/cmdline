#include "cmdline.hpp"
#include <iostream>

int main(int argc, char const* argv[]) {
    auto cmd = cmdline{
        "name"_mandatory,
        "type"_one_of("bin", "lib"),
    } <= [](std::map<std::string, std::string> params) {
            std::cout << params["name"] << ", " << params["type"] << std::endl;
    };
    /*
    auto cmd = cmdline{
        "new"_sub(
            "name"_mandatory,
            "type"_one_of("bin", "lib").default_("lib"),
            [&](cmd::args const& args) {
                std::cout << "create project : " << args["name"]
                          << " in type : " << args["type"] << std::endl;
            }
        ),
        "build"_sub(
            "type"_one_of("debug", "release").default_("debug"),
            "opt_level"_range(0, 99).default_(0),
            [&](cmd::args const& args) {
                std::cout << "building in type : " << args["type"]
                          << " opt : " << args["opt_level"] << std::endl;
            }
        ),
        "clean"_sub(
            [&](cmd::args const&) {
                std::cout << "cleaning..." << std::endl;
            }
        ),
        "help"_sub.short_name('h')(
            [&](cmd::args const&) {
                std::cout << "usage : ..." << std::endl;
            }
        )
    };
    */
    cmd.parse(argc, argv);
}


