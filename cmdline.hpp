#ifndef CMDLINE_HPP
#define CMDLINE_HPP

#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include <string>

#include <iostream>

struct cmdline {
    struct arg_base {
        explicit arg_base(std::string const& str) :
            name{str}
        {}
        virtual ~arg_base() = default;
        virtual void parse(
                std::vector<std::string>& input,
                std::map<std::string, std::string>&) const = 0;

        std::vector<std::string>::const_iterator position(std::vector<std::string> const& args) const {
            auto it = std::find(args.begin(), args.end(), "--" + name);
            if (it == args.end())
                throw std::logic_error{name + " is required"};
            return it;
        }
        std::string value(std::vector<std::string>& args) const {
            auto it = position(args);
            if (it+1 == args.end())
                throw std::logic_error{"not found: " + name + "'s value"};
            std::string value = *(it+1);
            args.erase(it+1);
            args.erase(it);
            return value;
        }
        std::string name;
    };

    struct mandatory : public arg_base {
        explicit mandatory(std::string const& str) :
            arg_base{str}
        {}
        void parse(
                std::vector<std::string>& args,
                std::map<std::string, std::string>& params) const override {
            params[name] = value(args);
        }
    };

    struct one_of : public arg_base {
        explicit one_of(std::string const& name, std::vector<std::string> const& candidates) :
            arg_base{name},
            candidates{candidates}
        {}
        void parse(
                std::vector<std::string>& args,
                std::map<std::string, std::string>& params) const override {
            auto val = value(args);
            auto found = std::find(candidates.begin(), candidates.end(), val);
            if (found == candidates.end())
                throw std::logic_error{val + " is not in range"};
            params[name] = val;
        }
        std::vector<std::string> candidates;
    };

    template<typename ... Args>
    explicit cmdline(Args const& ... args) :
        commands{args ...}
    {}

    cmdline& operator<=(std::function<void (std::map<std::string, std::string>)> const& f) {
        action = f;
        return *this;
    }

    void parse(int argc, char const* argv[]) {
        std::vector<std::string> args;
        program_name = argv[0];
        for (int i=1; i<argc; ++i) {
            std::string arg{argv[i]};
            auto eq_pos = std::find(arg.begin(), arg.end(), '=');
            if (eq_pos == arg.end()) {
                args.push_back(arg);
            } else {
                args.emplace_back(arg.begin(), eq_pos);
                args.emplace_back(eq_pos+1, arg.end());
            }
        }
        std::map<std::string, std::string> params;
        for (auto const& command : commands)
            command->parse(args, params);

        if (action)
            action(params);
    }
private:
    std::string program_name;

    std::vector<std::shared_ptr<arg_base>> commands;
    std::function<void (std::map<std::string, std::string> const&)> action;
};

auto operator "" _mandatory(char const* str, size_t) {
    return std::make_shared<cmdline::mandatory>(str);
}

auto operator "" _one_of(char const* str, size_t) {
    return [=](auto ... args) {
        std::vector<std::string> candidates{args ...};
        return std::make_shared<cmdline::one_of>(str, candidates);
    };
}

#endif
