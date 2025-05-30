//
// Created by đỗ quyên on 18/5/25.
//

#ifndef MIR_H
#define MIR_H
#include <string>
#include <bao/types.h>
#include <sys/types.h>
#include <vector>

namespace bao::mir {
    enum class ValueKind {
        Constant,
        Variable,
        Temporary,
    };

    struct Value {
        ValueKind kind;
        std::string name;
        std::unique_ptr<Type> type;

        Value(const Value& copy) {
            this->kind = copy.kind;
            this->name = copy.name;
            this->type = copy.type->clone();
        }
        Value(Value&& rval) = default;
        Value(): kind(ValueKind::Constant), name(""), type(std::move(std::make_unique<UnknownType>())) {}
        explicit Value(ValueKind&& kind, std::string&& name, std::unique_ptr<Type>&& type)
            : kind(std::move(kind)), name(std::move(name)), type(std::move(type)) {}
    };

    enum class BinaryOp {
        Add_s, // signed checked
        Add_u, // unsigned
        Add_f,
        Sub_s,
        Sub_u,
        Sub_f,
        Mul_s,
        Mul_u,
        Mul_f,
        Div_s,
        Div_u,
        Div_f,
        Rem_s,
        Rem_u,
        Lt_s,
        Lt_u,
    };

    /**
     * Base class for all instructions
     */
    struct Instruction {
        virtual ~Instruction() = default;
    };

    /*
    * Instruction to allocate on the stack
    *
    * dst (type)
    */
    struct AllocInst final : Instruction {
        Value dst;

        explicit AllocInst(Value& dst) : dst(dst) {}
    };

    /*
    * Instruction to store data into an alloc
    * 
    * src -> dst
    */
    struct StoreInst final : Instruction {
        Value src;
        Value dst;

        explicit StoreInst(Value&& src, Value& dst) : src(std::move(src)), dst(dst) {}
    };

    /*
    * Instruction to load data from an alloc
    *
    * dst <- src
    */
    struct LoadInst final : Instruction {
        Value dst;
        Value src;

        explicit LoadInst(Value& dst, Value&& src) : dst(dst), src(std::move(src)) {}
    };

    struct CallInst final : Instruction {
        Value res;
        std::string function_name;
        std::vector<Value> arguments;

        explicit CallInst(Value&& res, std::string&& function_name, std::vector<Value>&& arguments)
            : res(std::move(res)), function_name(std::move(function_name)), arguments(std::move(arguments)) {
        }
    };

    struct BinInst final : Instruction {
        Value dst;
        Value left;
        BinaryOp op;
        Value right;

        explicit BinInst(
            Value& dst, 
            Value&& left, 
            BinaryOp&& op,
            Value&& right
        ) : dst(dst),
            left(std::move(left)),
            op(std::move(op)),
            right(std::move(right)) {}
    };

    struct ReturnInst final : Instruction {
        Value ret_val;

        /**
         *
         * @param ret_val The return value
         */
        explicit ReturnInst(Value&& ret_val) : ret_val(std::move(ret_val)) {
        }
    };

    struct BasicBlock {
        std::string label;
        std::vector<std::unique_ptr<Instruction>> instructions;

        explicit BasicBlock(std::string&& label) : label(std::move(label)) {}
    };

    struct Function {
        std::string name;
        std::unique_ptr<Type> return_type;
        std::vector<Value> parameters;
        std::vector<BasicBlock> blocks;
        int temp_var_count = 0;
        int line;
        int column;
    };

    struct Module {
        std::string name;
        std::string path;
        std::vector<Function> functions;
    };
}
#endif //MIR_H
