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

        Value(): kind(ValueKind::Constant), name(""), type(std::move(std::make_unique<UnknownType>())) {}
        explicit Value(ValueKind&& kind, std::string&& name, std::unique_ptr<Type>&& type)
            : kind(std::move(kind)), name(std::move(name)), type(std::move(type)) {}
    };

    enum class InstructionKind {
        Assign,
        BinaryOp,
        Call,
        Jump,
        Branch,
        Return,
    };

    /**
     * Base class for all instructions
     */
    struct Instruction {
        InstructionKind kind;
        virtual ~Instruction() = default;
    };

    struct AssignInst final : Instruction {
        Value dst;
        Value src;

        AssignInst(Value&& dst, Value&& src) : dst(std::move(dst)), src(std::move(src)) {
            kind = InstructionKind::Assign;
        }
    };

    struct CallInst final : Instruction {
        Value res;
        std::string function_name;
        std::vector<Value> arguments;

        explicit CallInst(Value&& res, std::string&& function_name, std::vector<Value>&& arguments)
            : res(std::move(res)), function_name(std::move(function_name)), arguments(std::move(arguments)) {
            kind = InstructionKind::Call;
        }
    };

    struct ReturnInst final : Instruction {
        Value ret_val;

        /**
         *
         * @param ret_val The return value
         */
        explicit ReturnInst(Value&& ret_val) : ret_val(std::move(ret_val)) {
            kind = InstructionKind::Return;
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
