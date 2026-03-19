#ifndef CYSH_RUN_H
#define CYSH_RUN_H

#include <stdint.h>

#include <expected>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

#include "error.h"

struct cyProcResult {
    int exitCode;
    std::string stdout;
    std::string stderr;
};

struct cyVal {
    enum type { NIL, NUMBER, STRING, PROC_RESULT };

    using variant =
        std::variant<std::monostate, double, std::string, cyProcResult>;

    type dt;
    variant value;

    cyVal(void) : dt(NIL), value(std::monostate{}) {}
    cyVal(double val) : dt(NUMBER), value(val) {}
    cyVal(std::string val) : dt(STRING), value(val) {}
    cyVal(cyProcResult val) : dt(PROC_RESULT), value(val) {}
};

struct cyVar {
    enum flags : uint8_t {
        NONE = 0,
        ENV = 1 << 0,
        CONST = 1 << 1,
    };

    cyVal val;
    flags valFlags;

    cyVar(cyVal v, flags f) : val(v), valFlags(f) {}
};

struct cyScope {
   public:
    using sptr = std::shared_ptr<cyScope>;

    cyScope() : parent(nullptr) {}
    cyScope(sptr parent) : parent(parent) {}

    static sptr createGlobal();

    std::expected<void, cyErr> declare(
        const std::string &name, cyVal val, int line,
        cyVar::flags flags = cyVar::flags::NONE) {
        if (symbolTable.contains(name))
            return std::unexpected(mkerr(cyErr::SYNTAX_ERR, line,
                                         "redeclaration of %s", name.c_str()));

        symbolTable.try_emplace(name, val, flags);
        return {};
    }

    std::expected<std::reference_wrapper<cyVar>, cyErr> find(
        const std::string &name, int line) {
        if (symbolTable.contains(name))
            return std::ref(symbolTable.at(name));
        else if (parent != nullptr)
            return parent->find(name, line);

        return std::unexpected(
            mkerr(cyErr::SYNTAX_ERR, line, "%s is unbound", name.c_str()));
    }

    std::expected<void, cyErr> set(const std::string &name, cyVal newVal,
                                   int line) {
        auto val = find(name, line);
        if (!val) return std::unexpected(val.error());

        (*val).get().val = newVal;

        return {};
    }

   private:
    std::unordered_map<std::string, cyVar> symbolTable;
    sptr parent;
};

#endif  // CYSH_RUN_H
