#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace llvm;
using namespace llvm::sys;

#include "lexer.h"

Lexer lexer;

#include "parser.h"

#include "codegen.h"

#include "helper/helper.h"

//===----------------------------------------------------------------------===//
// Main driver code.
// コンパイラのインターフェースをドライバーと言ったりしますが、このメイン関数がまさにそれです。
//===----------------------------------------------------------------------===//

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "./mc file.mc" << std::endl;
        return -1;
    }

    // mc言語のテキストファイルの読み込み
    std::string fileName = argv[1];
    lexer.initStream(fileName);

    // 二項演算子の定義
    // 数字が低いほど結合度が低い
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40;

    getNextToken();

    MainLoop();
    
    write_output();

    return 0;
}
