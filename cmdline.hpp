#ifndef CMDLINE_HPP
#define CMDLINE_HPP

#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include <string>
#include <functional>

struct cmdline {
    using action_type = std::function<void (std::map<std::string, std::string>)>;

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
                it = std::find(args.begin(), args.end(), std::string{"-"} + short_name_);
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
        char short_name_ = '\0';
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

        mandatory& short_name(char c) {
            short_name_ = c;
            return *this;
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

        one_of& short_name(char c) {
            short_name_ = c;
            return *this;
        }
        std::vector<std::string> candidates;
    };

    struct subcommand : public arg_base {
        explicit subcommand(std::string const& name) :
            arg_base{name}
        {}

        template<typename T>
        subcommand& operator<<(T t) {
            commands.push_back(std::make_shared<T>(t));
            return *this;
        }
        template<typename F>
        subcommand& operator<=(F const& f) {
            action = f;
            return *this;
        }

        void parse(
                std::vector<std::string>& args,
                std::map<std::string, std::string>& params) const override {

            if (args[0] != name)
                throw std::logic_error{"invalid parsing"};
            args.erase(args.begin());
            for (auto const& command : commands) {
                bool is_subcommand = std::dynamic_pointer_cast<subcommand>(command) != nullptr;
                auto is_top_command_name = command->name == args[0];
                if (is_subcommand && is_top_command_name) {
                    command->parse(args, params);
                    break;
                } else if (is_subcommand && !is_top_command_name) {
                    continue;
                } else {
                    command->parse(args, params);
                }
            }

            if (action)
                action(params);
        }
        subcommand& short_name(char c) {
            short_name_ = c;
            return *this;
        }

        std::vector<std::shared_ptr<arg_base>> commands;
        action_type action;
    };

    struct flag : public arg_base {
        explicit flag(std::string const& name) :
            arg_base{name}
        {}

        void parse(
                std::vector<std::string>& args,
                std::map<std::string, std::string>& params) const override {
            auto p = position(args);
            if (p != args.end()) {
                params[name] = "true";
                args.erase(p);
            } else {
                params[name] = "false";
            }
        }

        flag& short_name(char c) {
            short_name_ = c;
            return *this;
        }
    };

    explicit cmdline() = default;

    template<typename T>
    cmdline& operator<<(T t) {
        commands.push_back(std::make_shared<T>(t));
        return *this;
    }
    template<typename F>
    cmdline& operator<=(F const& f) {
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
        for (auto const& command : commands) {
            bool is_subcommand = std::dynamic_pointer_cast<subcommand>(command) != nullptr;
            auto is_top_command_name = command->name == args[0];
            if (is_subcommand && is_top_command_name) {
                command->parse(args, params);
                break;
            } else if (is_subcommand && !is_top_command_name) {
                continue;
            } else {
                command->parse(args, params);
            }
        }

        if (action)
            action(params);
    }

    std::string program_name;

    std::vector<std::shared_ptr<arg_base>> commands;
    action_type action;
};

auto operator "" _mandatory(char const* str, size_t) {
    return cmdline::mandatory{str};
}

auto operator "" _one_of(char const* str, size_t) {
    return [=](auto ... args) {
        std::vector<std::string> candidates{args ...};
        return cmdline::one_of{str, candidates};
    };
}

auto operator "" _subcommand(char const* str, size_t) {
    return cmdline::subcommand{str};
}

auto operator ""_flag(char const* str, size_t) {
    return cmdline::flag{str};
}

#endif
