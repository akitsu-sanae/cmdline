# cmdline

cmdline is a header-only command line parser for C++.
All you must do is `#include "cmdline.hpp"`

# example

```
#include "cmdline.hpp"
#include <iostream>

int main(int argc, char const* argv[]) {
    auto cmd = cmdline()
        << ("hoge"_subcommand)
            << "foo"_mandatory
            << "bar"_one_of("cat", "dog")
            <= [](std::map<std::string, std::string> params) {
                std::cout << params["foo"] << params["bar"] << std::endl;
            })
        << ("fuga"_subcommand
            << "poyo"_one_of("neko", "inu")
            << "help"_flag
            <= [](std::map<std::string, std::string> params) {
                if (params["help"] == "true")
                    std::cout << "here is fuga's help" << std::endl;
                else
                    std::cout << param["poyo"] << std::endl;
            })
        << "help"_flag.short_name('h')
        <= [](std::map<std::string, std::string> params) {
            if (params["help"] == "true")
                std::cout << "Usage: ..." << std::endl;
        };
    cmd.parse(argc, argv);
}
```
compile this C++ program, and output as `"a.out"`,
then
```
$ ./a.out hoge --foo=nyan --bar=cat    # => output: nyancat
$ ./a.out hoge --foo=nyan --bar=buncho # => fail
$ ./a.out fuga --poyo neko             # => output: neko
$ ./a.out fuga --help                  # => output: here is fuga's help
$ ./a.out --help                       # => output: Usage: ...
$ ./a.out -h                           # => output: Usage: ...
```

# Copyright
Copyright (C) 2017 akitsu sanae.  
Distributed under the Boost Software License, Version 1.0. 
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost/org/LICENSE_1_0.txt)  


