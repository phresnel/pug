//
// This file is part of pug-vm.
// pug-vm is free software: you can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// pug-vm is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;  without
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with pug-vm. If not,
// see <http://www.gnu.org/licenses/>
//

#ifndef PUGVM_HH_INCLUDED_20130913
#define PUGVM_HH_INCLUDED_20130913

// TODO: In safe mode, enable type safety by adding RTTI to operands.

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <vector>
#include <limits>

namespace PugVM {

    enum Operation {
        PushFloat,
        PushInt,
        PushBool,

        Pop,
        PopReduce,

        Dup,

        StoreFloat,
        StoreInt,
        StoreBool,
        StoreStAbs,
        StoreStRel,
        StoreLocal,
        StoreArg,

        LoadFloat,
        LoadInt,
        LoadBool,
        LoadStAbs,
        LoadStRel,
        LoadLocal,
        LoadArg,

        EqualsFF,
        EqualsII,
        EqualsBB,
        NotEqualsFF,
        NotEqualsII,
        NotEqualsBB,

        Jump,
        JumpIfTrue,
        JumpRel,
        JumpRelIfTrue,

        Call,
        Return,
        ReturnTos,

        AddFF,
        AddII,

        SubFF,
        SubII,

        MulFF,
        MulII,

        DecrementI,
        IncrementI,

        Dump,

        Exit
    };

    inline
    std::ostream& operator<< (std::ostream &os, Operation op) {
        switch (op) {
        case PushFloat: return os << "PushFloat";
        case PushInt: return os << "PushInt";
        case PushBool: return os << "PushBool";
        case Pop: return os << "Pop";
        case PopReduce: return os << "PopReduce";

        case Dup: return os << "Dup";

        case StoreFloat: return os << "StoreFloat";
        case StoreInt: return os << "StoreInt";
        case StoreBool: return os << "StoreBool";
        case StoreStAbs: return os << "StoreStAbs";
        case StoreStRel: return os << "StoreStRel";
        case StoreLocal: return os << "StoreLocal";
        case StoreArg: return os << "StoreArg";

        case LoadFloat: return os << "LoadFloat";
        case LoadInt: return os << "LoadInt";
        case LoadBool: return os << "LoadBool";
        case LoadStAbs: return os << "LoadStAbs";
        case LoadStRel: return os << "LoadStRel";
        case LoadLocal: return os << "LoadLocal";
        case LoadArg: return os << "LoadArg";

        case EqualsFF: return os << "EqualsFF";
        case EqualsII: return os << "EqualsII";
        case EqualsBB: return os << "EqualsBB";
        case NotEqualsFF: return os << "NotEqualsFF";
        case NotEqualsII: return os << "NotEqualsII";
        case NotEqualsBB: return os << "NotEqualsBB";

        case Jump: return os << "Jump";
        case JumpIfTrue: return os << "JumpIfTrue";
        case JumpRel: return os << "JumpRel";
        case JumpRelIfTrue: return os << "JumpRelIfTrue";

        case Call: return os << "Call";
        case Return: return os << "Return";
        case ReturnTos: return os << "ReturnTos";

        case AddFF: return os << "AddFF";
        case AddII: return os << "AddII";

        case SubFF: return os << "SubFF";
        case SubII: return os << "SubII";

        case MulFF: return os << "MulFF";
        case MulII: return os << "MulII";

        case IncrementI: return os << "IncrementI";
        case DecrementI: return os << "DecrementI";

        case Dump: return os << "Dump";

        case Exit: return os << "Exit";
        }
        throw std::logic_error("unimplemented");
    }

    struct Operand {
        union {
            int int_;
            float float_;
            bool bool_;
            const char *cstr_;

            int *pint_;
            float *pfloat_;
            bool *pbool_;
        };
        Operand(int i) : int_(i) {}
        Operand(float i) : float_(i) {}
        Operand(bool i) : bool_(i) {}
        Operand(const char *c) : cstr_(c) {}
        Operand(int *i) : pint_(i) {}
        Operand(float *i) : pfloat_(i) {}
        Operand(bool *i) : pbool_(i) {}
    };

    struct Instruction {
        Operation operation;
        Operand operand;

        Instruction(Operation operation, Operand operand) : operation(operation), operand(operand) {}
        Instruction(Operation operation) : operation(operation), operand(0) {}
    };


    template <bool debug>
    class StackMachine {
    public:
        StackMachine (std::vector<Instruction> program) : program(program)
        {
            if (program.size() >= static_cast<unsigned>(std::numeric_limits<int>::max()))
                throw std::runtime_error("program is too big, lad.");
        }

        void tick() {
            if (pc < 0 || pc >= (int)program.size()) {
                halt = true;
                return;
            }

            if (halt)
                throw std::logic_error("used StackMachine::step() on halted machine");

            if (debug) statedump();
            auto const &p = program[pc++];

            switch (p.operation) {
            case PushFloat: push(p.operand.float_); break;
            case PushInt:   push(p.operand.int_);   break;
            case PushBool:  push(p.operand.bool_);  break;
            case Pop:       pop();                  break;
            case PopReduce: { auto top = take(); pop(); push(top); break; }

            case Dup: push(stack.back()); break;

            case LoadFloat:  push(*p.operand.pfloat_);    break;
            case LoadInt:    push(*p.operand.pint_);      break;
            case LoadBool:   push(*p.operand.pbool_);     break;
            case LoadStAbs:  push(stack[p.operand.int_]); break;
            case LoadStRel:  push(stack[(stack.size()-1)-p.operand.int_]); break;
            case LoadLocal:  push(stack[stack_base+p.operand.int_]);       break;
            case LoadArg:    push(stack[(stack_base-2)-p.operand.int_]);   break;

            case StoreFloat: *p.operand.pfloat_    = take_float(); break;
            case StoreInt:   *p.operand.pint_      = take_int();   break;
            case StoreBool:  *p.operand.pbool_     = take_bool();  break;
            case StoreStAbs: stack[p.operand.int_] = take();       break;
            case StoreStRel: stack[(stack.size()-1)-p.operand.int_] = take(); break;
            case StoreLocal: stack[stack_base+p.operand.int_] = take();       break;
            case StoreArg:   stack[(stack_base-2)-p.operand.int_] = take();   break;

            case EqualsFF:    push(bool(take_float() == take_float())); break;
            case EqualsII:    push(bool(take_int()   == take_int()));   break;
            case EqualsBB:    push(bool(take_bool()  == take_bool()));  break;
            case NotEqualsFF: push(bool(take_float() != take_float())); break;
            case NotEqualsII: push(bool(take_int()   != take_int()));   break;
            case NotEqualsBB: push(bool(take_bool()  != take_bool()));  break;

            case Jump:          pc = p.operand.int_;                           break;
            case JumpRel:       pc = (pc-1) + p.operand.int_;                  break;
            case JumpIfTrue:    if (take_bool()) pc = p.operand.int_;          break;
            case JumpRelIfTrue: if (take_bool()) pc = (pc-1) + p.operand.int_; break;

            case Call:      push(pc);
                            push(stack_base);
                            pc = p.operand.int_;
                            stack_base = stack.size()-1;
                            break;

            case Return:    stack_base = take_int();
                            pc = take_int();
                            break;
            case ReturnTos: { auto tos = take();
                              stack_base = take_int();
                              pc = take_int();
                              push(tos);
                              break; }

            case AddFF: push(take_float() + take_float()); break;
            case AddII: push(take_int()   + take_int());   break;
            case SubFF: push(take_float() - take_float()); break;
            case SubII: push(take_int()   - take_int());   break;
            case MulFF: push(take_float() * take_float()); break;
            case MulII: push(take_int()   * take_int());   break;
            case IncrementI: push(take_int()+1); break;
            case DecrementI: push(take_int()-1); break;

            case Dump: std::cout << p.operand.cstr_; break;

            case Exit: halt = true; break;
            }
        }

        bool halted() const {
            return halt;
        }

        void statedump() {
            if (halted()) {
                std::cerr << "           exit state : ";
            } else {
                auto const &p = program[pc];
                std::cerr << '[' << std::setw(3) << (pc) << ']' << std::setw(16) << p.operation << " : ";
            }
            for (auto &x : stack) { std::cerr << x.int_ << " | "; }
            std::cerr << std::endl;
        }

    private:
        int pc = 0;
        int stack_base = 0;
        bool halt = false;
        std::vector<Instruction> program;

        std::vector<Operand> stack;

    private:
        void push(Operand const &operand) { stack.push_back(operand); }
        void push(float val) { stack.emplace_back(val); }
        void push(int   val) { stack.emplace_back(val); }
        void push(bool  val) { stack.emplace_back(val); }
        void pop() { stack.pop_back(); }

        Operand take () {
            auto ret = stack.back();
            stack.pop_back();
            return ret;
        }

        float take_float () { return take().float_; }
        int   take_int ()   { return take().int_; }
        bool  take_bool()   { return take().bool_; }
    };
}


#endif // PUGVM_HH_INCLUDED_20130913
