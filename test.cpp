#include "cmdline.hpp"
#include <iostream>

int main(int argc, char const* argv[]) {
    auto cmd = cmdline()
        << ("new"_subcommand
            << "name"_mandatory
            << "type"_one_of("bin", "lib")
            <= [](std::map<std::string, std::string> params) {
                std::cout << "new project: " << params["name"] << ", " << params["type"] << std::endl;
            })
        << ("build"_subcommand
            << "type"_one_of("debug", "release")
            <= [](std::map<std::string, std::string> params) {
                std::cout << "building: " << params["type"] << std::endl;
            })
        << ("config"_subcommand
            << ("user"_subcommand
                << "name"_mandatory
                <= [](std::map<std::string, std::string> params) {
                    std::cout << "new user name is " << params["name"] << std::endl;
                })
            << ("env"_subcommand
                << "vcs"_one_of("git", "subversion", "other")
                <= [](std::map<std::string, std::string> params) {
                    std::cout << "new vcs is " << params["vcs"] << std::endl;
                }))
        << "help"_flag.short_name('h')
        <= [](std::map<std::string, std::string> params) {
            if (params["help"] == "true")
                std::cout << "Usage: ..." << std::endl;
        };
    cmd.parse(argc, argv);
}


