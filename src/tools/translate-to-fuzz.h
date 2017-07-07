/*
 * Copyright 2017 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// Translate a binary stream of bytes into a valid wasm module, *somehow*.
// This is helpful for fuzzing.
//

namespace wasm {

class TranslateToFuzzReader {
public:
  TranslateToFuzzReader(Module& wasm) : wasm(wasm), builder(wasm) {}

  void read(std::string& filename) {
    auto input(read_file<std::vector<char>>(filename, Flags::Binary, Flags::Release));
    bytes.swap(input);
    pos = 0;
    finishedInput = false;
    // ensure *some* input to be read
    if (bytes.size() == 0) {
      bytes.push_back(0);
    }
    build();
  }

private:
  Module& wasm;
  Builder builder;
  std::vector<char> bytes; // the input bytes
  size_t pos; // the position in the input
  bool finishedInput; // whether we already cycled through all the input (if so, we should try to finish things off)

  // some things require luck, try them a few times
  static int TRIES = 10;

  int8_t get() {
    if (pos == bytes.size()) {
      // we ran out of input, go to the start for more stuff
      finishedInput = true;
      pos = 0;
    }
    return bytes[pos++];
  }

  int16_t get16() {
    return (int16_t(get()) << 8) | int16_t(get());
  }

  int32_t get32() {
    return (int32_t(get16()) << 16) | int32_t(get16());
  }

  int64_t get64() {
    return (int64_t(get32()) << 32) | int64_t(get32());
  }

  float getFloat() {
    return Literal(get32()).reinterpretf32();
  }

  float getDouble() {
    return Literal(get64()).reinterpretf64();
  }

  void build() {
    // keep adding functions until we run out of input
    while (!finishedInput) {
      addFunction();
    }
  }

  // function generation state

  Function* func;
  std::vector<Expression*> breakableStack;
  Index labelIndex;

  void addFunction() {
    Index num = wasm.functions.size();
    func = new Function;
    func->name = std::string("func_") + std::to_string(num);
    func->result = getReachableType();
    Index numParams = logify(get16());
    for (Index i = 0; i < numParams; i++) {
      func->params.push_back(getConcreteType());
    }
    Index numVars = logify(get16());
    for (Index i = 0; i < numVars; i++) {
      func->vars.push_back(getConcreteType());
    }
    labelIndex = 0;
    assert(breakableStack.empty());
    // with small chance, make the body unreachable
    if (oneIn(10)) {
      func->body = make(unreachable);
    } else {
      func->body = make(func->result);
    }
    assert(breakableStack.empty());
    wasm.addFunction(func);
  }

  Name makeLabel() {
    return std::string("label$") + std::to_string(labelIndex++);
  }

  // TODO: avoid recursion in core make and makeTYPE, if finishedInput, return simple to quit quickly

  Expression* make(WasmType type) {
    switch (type) {
      case i32: return makei32();
      case i64: return makei64();
      case f32: return makef32();
      case f64: return makef64();
      case none: return makenone();
      case unreachable: return makeunreachable();
    }
  }

  Expression* makei32() {
    switch (get() % 13) {
      case 0: return makeBlock(i32);
      case 1: return makeIf(i32);
      case 2: return makeLoop(i32);
      case 3: return makeBreak(i32);
      case 4: return makeCall(i32);
      case 5: return makeCallIndirect(i32);
      case 6: return makeGetLocal(i32);
      case 7: return makeSetLocal(i32);
      case 8: return makeLoad(i32);
      case 9: return makeConst(i32);
      case 10: return makeUnary(i32);
      case 11: return makeBinary(i32);
      case 12: return makeSelect(i32);
    }
  }

  Expression* makei64() {
    switch (get() % 13) {
      case 0: return makeBlock(i64);
      case 1: return makeIf(i64);
      case 2: return makeLoop(i64);
      case 3: return makeBreak(i64);
      case 4: return makeCall(i64);
      case 5: return makeCallIndirect(i64);
      case 6: return makeGetLocal(i64);
      case 7: return makeSetLocal(i64);
      case 8: return makeLoad(i64);
      case 9: return makeConst(i64);
      case 10: return makeUnary(i64);
      case 11: return makeBinary(i64);
      case 12: return makeSelect(i64);
    }
  }

  Expression* makef32() {
    switch (get() % 13) {
      case 0: return makeBlock(f32);
      case 1: return makeIf(f32);
      case 2: return makeLoop(f32);
      case 3: return makeBreak(f32);
      case 4: return makeCall(f32);
      case 5: return makeCallIndirect(f32);
      case 6: return makeGetLocal(f32);
      case 7: return makeSetLocal(f32);
      case 8: return makeLoad(f32);
      case 9: return makeConst(f32);
      case 10: return makeUnary(f32);
      case 11: return makeBinary(f32);
      case 12: return makeSelect(f32);
    }
  }

  Expression* makef64() {
    switch (get() % 13) {
      case 0: return makeBlock(f64);
      case 1: return makeIf(f64);
      case 2: return makeLoop(f64);
      case 3: return makeBreak(f64);
      case 4: return makeCall(f64);
      case 5: return makeCallIndirect(f64);
      case 6: return makeGetLocal(f64);
      case 7: return makeSetLocal(f64);
      case 8: return makeLoad(f64);
      case 9: return makeConst(f64);
      case 10: return makeUnary(f64);
      case 11: return makeBinary(f64);
      case 12: return makeSelect(f64);
    }
  }

  Expression* makenone() {
    switch (get() % 12) {
      case 0: return makeBlock(none);
      case 1: return makeIf(none);
      case 2: return makeLoop(none);
      case 3: return makeBreak(none);
      case 4: return makeCall(none);
      case 5: return makeCallIndirect(none);
      case 6: return makeSetLocal(none);
      case 8: return makeStore(none);
      case 9: return makeDrop(none);
      case 11: return makeNop(none);
    }
  }

  Expression* makeunreachable() {
    switch (get() % 16) {
      case 0: return makeBlock(unreachable);
      case 1: return makeIf(unreachable);
      case 2: return makeLoop(unreachable);
      case 3: return makeBreak(unreachable);
      case 4: return makeCall(unreachable);
      case 5: return makeCallIndirect(unreachable);
      case 6: return makeSetLocal(unreachable);
      case 7: return makeStore(unreachable);
      case 8: return makeUnary(unreachable);
      case 9: return makeBinary(unreachable);
      case 10: return makeSelect(unreachable);
      case 11: return makeSwitch(unreachable);
      case 12: return makeStore(unreachable);
      case 13: return makeDrop(unreachable);
      case 14: return makeReturn(unreachable);
      case 15: return makeUnreachable(unreachable);
    }
  }

  // specific expression creators

  Expression* makeBlock(WasmType type) {
    auto* ret = builder.makeBlock();
    ret->type = type; // so we have it during child creation
    ret->name = makeLabel();
    breakableStack.push_back(ret);
    Index num = logify(get());
    while (num > 0) {
      ret->block.push_back(makenone());
    }
    ret->block.push_back(make(type));
    breakableStack.pop_back();
    ret->finalize(type);
    return ret;
  }

  Expression* makeLoop(WasmType type) {
    auto* ret = builder.makeLoop();
    ret->type = type; // so we have it during child creation
    ret->name = makeLabel();
    breakableStack.push_back(ret);
    Index num = logify(get());
    ret->body = make(type);
    breakableStack.pop_back();
    ret->finalize(type);
    return ret;
  }

  Expression* makeIf(WasmType type) {
    return builder.makeIf(makei32(), make(type), make(type));
  }

  Expression* makeBreak(WasmType type) {
    if (breakableStack.empty()) return make(type);
    Expression* condition = nullptr;
    if (type != unreachable) {
      condition = makei32();
    }
    // we need to find a proper target to break to; try a few times 
    int tries = TRIES;
    while (tries-- > 0) {
      auto* target = choice(breakableStack);
      auto name = getTargetName(target);
      auto valueType = target->type;
      if (isConcreteWasmType(type)) {
        // we are flowing out a value
        if (targetType != type) {
          // we need to break to a proper place
          continue;
        }
        return builder.makeBreak(name, make(type), condition);
      } else if (type == none) {
        if (targetType != none) {
          // we need to break to a proper place
          continue;
        }
        return builder.makeBreak(name, nullptr, condition);
      } else {
        assert(type == unreachable);
        if (targetType != none) {
          // we need to break to a proper place
          continue;
        }
        return builder.makeBreak(name);
      }
    }
    // we failed to find something
    return make(type);
  }

  Expression* makeCall(WasmType type) {
    int tries = TRIES;
    while (tries-- > 0) {
      Function* target = func;
      if (!wasm.functions.empty() && !oneIn(wasm.functions.size())) {
        target = choice(wasm.functions).get();
      }
      if (target->result != type) continue;
      // we found one!
      std::vector<Expression*> args;
      for (auto argType : target->params) {
        args.push_back(make(argType));
      }
      return builder.makeCall(target->name, args, type);
    }
    // we failed to find something
    return make(type);
  }

  Expression* makeCallIndirect(WasmType type) {
    return make(type); // TODO
  }

  Expression* makeGetLocal(WasmType type) {
    auto total = func->getNumLocals();
    if (total == 0) return make(type);
    int tries = TRIES;
    while (tries-- > 0) {
      auto index = upTo(total);
      if (func->getLocalType(index) != type) continue;
      // we found one
      return builder.makeGetLocal(index, type);
    }
    // we failed to find something
    return make(type);
  }

  Expression* makeSetLocal(WasmType type) {
    auto total = func->getNumLocals();
    if (total == 0) return make(type);
    int tries = TRIES;
    while (tries-- > 0) {
      auto index = upTo(total);
      if (func->getLocalType(index) != type) continue;
      // we found one
      if (type == none) {
        return builder.makeSetLocal(index, make(type));
      } else {
        return builder.makeTeeLocal(index, make(type));
      }
    }
    // we failed to find something
    return make(type);
  }

  Expression* makeLoad(WasmType type) {
    auto offset = logify(get());
    auto ptr = make(i32); // TODO: mask it, think about memory properly
    switch (type) {
      case i32: {
        bool signed_ = get() & 1;
        switch (upTo(3)) {
          case 0: return builder.makeLoad(1, signed_, offset, 1, ptr, type);
          case 1: return builder.makeLoad(2, signed_, offset, choice({ 1, 2 }), ptr, type);
          case 2: return builder.makeLoad(4, signed_, offset, choice({ 1, 2, 4 }), ptr, type);
        }
        WASM_UNREACHABLE();
      }
      case i64: {
        bool signed_ = get() & 1;
        switch (upTo(4)) {
          case 0: return builder.makeLoad(1, signed_, offset, 1, ptr, type);
          case 1: return builder.makeLoad(2, signed_, offset, choice({ 1, 2 }), ptr, type);
          case 2: return builder.makeLoad(4, signed_, offset, choice({ 1, 2, 4 }), ptr, type);
          case 3: return builder.makeLoad(8, signed_, offset, choice({ 1, 2, 4, 8 }), ptr, type);
        }
        WASM_UNREACHABLE();
      }
      case f32: {
        return builder.makeLoad(4, false, offset, choice({ 1, 2, 4 }), ptr, type);
      }
      case f64: {
        return builder.makeLoad(8, false, offset, choice({ 1, 2, 4, 8 }), ptr, type);
      }
      default: WASM_UNREACHABLE();
    }
  }

  Expression* makeStore(WasmType type) {
    auto offset = logify(get());
    auto ptr = make(i32); // TODO: mask it, think about memory properly
    switch (type) {
      case i32: {
        switch (upTo(3)) {
          case 0: return builder.makeStore(1, offset, 1, ptr, make(type), type);
          case 1: return builder.makeStore(2, offset, choice({ 1, 2 }), ptrmake(type), type);
          case 2: return builder.makeStore(4, offset, choice({ 1, 2, 4 }), ptrmake(type), type);
        }
        WASM_UNREACHABLE();
      }
      case i64: {
        switch (upTo(4)) {
          case 0: return builder.makeStore(1, offset, 1, ptrmake(type), type);
          case 1: return builder.makeStore(2, offset, choice({ 1, 2 }), ptrmake(type), type);
          case 2: return builder.makeStore(4, offset, choice({ 1, 2, 4 }), ptrmake(type), type);
          case 3: return builder.makeStore(8, offset, choice({ 1, 2, 4, 8 }), ptrmake(type), type);
        }
        WASM_UNREACHABLE();
      }
      case f32: {
        return builder.makeStore(4, offset, choice({ 1, 2, 4 }), ptrmake(type), type);
      }
      case f64: {
        return builder.makeStore(8, offset, choice({ 1, 2, 4, 8 }), ptrmake(type), type);
      }
      default: WASM_UNREACHABLE();
    }
  }

  Expression* makeConst(WasmType type) {
    Literal value;
    // TODO: favor special numbers like 0, -1, i8 size, etc.?
    switch (type) {
      case i32: value = Literal(get32()); break;
      case i64: value = Literal(get64()); break;
      case f32: value = Literal(getFloat()); break;
      case f64: value = Literal(getDouble()); break;
      default: WASM_UNREACHABLE();
    }
    auto* ret = wasm.allocator.alloc<Const>();
    ret->value = value;
    ret->type = value.type;
    return ret;
  }

  Expression* makeUnary(WasmType type) {
    if (type == unreachable) {
      return builder.makeUnary(makeUnary(getConcreteType())->cast<Unary>()->op, make(unreachable));
    }
    switch (type) {
      case i32: {
        switch (upTo(4)) {
          case 0: return builder.makeUnary(choice({ EqZInt32, ClzInt32, CtzInt32, PopcntInt32 }), make(i32));
          case 1: return builder.makeUnary(choice({ WrapInt64 }), make(i64));
          case 2: return builder.makeUnary(choice({ TruncSFloat32ToInt32, TruncUFloat32ToInt32, ReinterpretFloat32 }), make(f32));
          case 3: return builder.makeUnary(choice({ TruncSFloat64ToInt32, TruncUFloat64ToInt32 }), make(f64));
        }
        WASM_UNREACHABLE();
      }
      case i64: {
        switch (upTo(4)) {
          case 0: return builder.makeUnary(choice({ EqZInt64, ClzInt64, CtzInt64, PopcntInt64 }), make(i64));
          case 1: return builder.makeUnary(choice({ ExtendSInt32, ExtendUInt32 }), make(i32));
          case 2: return builder.makeUnary(choice({ TruncSFloat32ToInt64, TruncUFloat32ToInt64 }), make(f32));
          case 3: return builder.makeUnary(choice({ TruncSFloat64ToInt64, TruncUFloat64ToInt64, ReinterpretFloat64 }), make(f64));
        }
        WASM_UNREACHABLE();
      }
      case f32: {
        switch (upTo(4)) {
          case 0: return builder.makeUnary(choice({ NegFloat32, AbsFloat32, CeilFloat32, FloorFloat32, TruncFloat32, NearestFloat32, SqrtFloat32 }), make(f32));
          case 1: return builder.makeUnary(choice({ ConvertUInt32ToFloat32, ConvertSInt32ToFloat32, ReinterpretInt32 }), make(i32));
          case 2: return builder.makeUnary(choice({ ConvertUInt64ToFloat32, ConvertSInt64ToFloat32 }), make(i64));
          case 3: return builder.makeUnary(DemoteFloat64, make(f64));
        }
        WASM_UNREACHABLE();
      }
      case f64: {
        switch (upTo(4)) {
          case 0: return builder.makeUnary(choice({ NegFloat64, AbsFloat64, CeilFloat64, FloorFloat64, TruncFloat64, NearestFloat64, SqrtFloat64 }), make(f64));
          case 1: return builder.makeUnary(choice({ ConvertUInt32ToFloat64, ConvertSInt32ToFloat64 }), make(i32));
          case 2: return builder.makeUnary(choice({ ConvertUInt64ToFloat64, ConvertSInt64ToFloat64, ReinterpretInt64 }), make(i64));
          case 3: return builder.makeUnary(PromoteFloat32, make(f32));
        }
        WASM_UNREACHABLE();
      }
    }
    WASM_UNREACHABLE();
  }

  Expression* makeBinary(WasmType type) {
    if (type == unreachable) {
      return builder.makeBinary(makeBinary(getConcreteType())->cast<Binary>()->op, make(unreachable), make(unreachable));
    }
    switch (type) {
      case i32: {
        switch (upTo(4)) {
          case 0: return builder.makeBinary(choice({ AddInt32, SubInt32, MulInt32, DivSInt32, DivUInt32, RemSInt32, RemUInt32, AndInt32, OrInt32, XorInt32, ShlInt32, ShrUInt32, ShrSInt32, RotLInt32, RotRInt32, EqInt32, NeInt32, LtSInt32, LtUInt32, LeSInt32, LeUInt32, GtSInt32, GtUInt32, GeSInt32, GeUInt32 }), make(i32), make(i32));
          case 1: return builder.makeBinary(choice({ EqInt64, NeInt64, LtSInt64, LtUInt64, LeSInt64, LeUInt64, GtSInt64, GtUInt64, GeSInt64, GeUInt64 }), make(i64), make(i64));
          case 2: return builder.makeBinary(choice({ EqFloat32, NeFloat32, LtFloat32, LeFloat32, GtFloat32, GeFloat32 }), make(f32), make(f32));
          case 3: return builder.makeBinary(choice({ EqFloat64, NeFloat64, LtFloat64, LeFloat64, GtFloat64, GeFloat64 }), make(f64), make(f64));
        }
        WASM_UNREACHABLE();
      }
      case i64: {
        return builder.makeBinary(choice({ AddInt64, SubInt64, MulInt64, DivSInt64, DivUInt64, RemSInt64, RemUInt64, AndInt64, OrInt64, XorInt64, ShlInt64, ShrUInt64, ShrSInt64, RotLInt64, RotRInt64 }), make(i64), make(i64));
      }
      case f32: {
        return builder.makeBinary(choice({ AddFloat32, SubFloat32, MulFloat32, DivFloat32, CopySignFloat32, MinFloat32, MaxFloat32 }), make(f32), make(f32));
      }
      case f64: {
        return builder.makeBinary(choice({ AddFloat64, SubFloat64, MulFloat64, DivFloat64, CopySignFloat64, MinFloat64, MaxFloat64 }), make(f64), make(f64));
      }
    }
    WASM_UNREACHABLE();
  }

  Expression* makeSelect(WasmType type) {
    return builder.makeSelect(make(i32), make(type), make(type));
  }

  Expression* makeSwitch(WasmType type) {
    assert(type == unreachable);
    if (breakableStack.empty()) return make(type);
    // we need to find proper targets to break to; try a bunch
    int tries = 2 * TRIES;
    std::vector<Name> names;
    WasmType valueType;
    while (tries-- > 0) {
      auto* target = choice(breakableStack);
      auto name = getTargetName(target);
      if (names.empty()) {
        valueType == target->type;
      } else {
        if (valueType != target->type) {
          continue; // all values must be the same
        }
      }
      names.push_back(name);
    }
    if (names.size() < 2) {
      // we failed to find enough
      return make(type);
    }
    auto default_ = names.back();
    names.pop_back();
    return builder.makeSwitch(names, default_, nullptr, isConcreteValueType(valueType) ? make(valueType) : nullptr);
  }

  Expression* makeDrop(WasmType type) {
    return builder.makeDrop(make(type));
  }

  Expression* makeReturn(WasmType type) {
    return builder.makeReturn(isConcreteValueType(func->result) ? make(func->result) : nullptr);
  }

  Expression* makeNop(WasmType type) {
    assert(type == none);
    return builder.makeNop();
  }

  // special getters

  WasmType getType() {
    switch (get() % 6) {
      case 0: return i32;
      case 1: return i64;
      case 2: return f32;
      case 3: return f64;
      case 4: return none;
      case 5: return unreachable;
    }
  }

  WasmType getReachableType() {
    switch (get() % 5) {
      case 0: return i32;
      case 1: return i64;
      case 2: return f32;
      case 3: return f64;
      case 4: return none;
    }
  }

  WasmType getConcreteType() {
    switch (get() % 4) {
      case 0: return i32;
      case 1: return i64;
      case 2: return f32;
      case 3: return f64;
    }
  }

  // statistical distributions

  // 0 to the limit, logarithmic scale
  Index logify(Index x) {
    return std::floor(std::log(1 + x));
  }

  bool oneIn(Index x) {
    return (get32() % x) == 0;
  }

  bool upTo(Index x) {
    return get32() % x;
  }

  template<typename T>
  T choice(std::vector<T>& vec) {
    // TODO: get32?
    assert(!vec.empty());
    auto index = get() % vec.size();
    return vec[index];
  }

  // utilities

  Name getTargetName(Expression* target) {
    if (auto* block = target->dynCast<Block>()) {
      return block->name;
    } else if (auto* loop = target->dynCast<Loop>()) {
      return loop->name;
    } else {
      WASM_UNREACHABLE();
    }
  }
};

} // namespace wasm

// XXX Switch class has a condition?! is it real? should the node type be the value type if it exists?!