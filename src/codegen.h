//===----------------------------------------------------------------------===//
// Code Generation
// このファイルでは、parser.hによって出来たASTからLLVM IRを生成しています。
// といっても、難しいことをしているわけではなく、IRBuilder(https://llvm.org/doxygen/IRBuilder_8h_source.html)
// のインターフェースを利用して、parser.hで定義した「ソースコードの意味」をIRに落としています。
// 各ファイルの中で一番LLVMの機能を多用しているファイルです。
//===----------------------------------------------------------------------===//

// https://llvm.org/doxygen/LLVMContext_8h_source.html
static LLVMContext Context;
// https://llvm.org/doxygen/classllvm_1_1IRBuilder.html
// LLVM IRを生成するためのインターフェース
static IRBuilder<> Builder(Context);
// https://llvm.org/doxygen/classllvm_1_1Module.html
// このModuleはC++ Moduleとは何の関係もなく、LLVM IRを格納するトップレベルオブジェクトです。
static std::unique_ptr<Module> myModule;

// https://llvm.org/doxygen/classllvm_1_1Value.html
// llvm::Valueという、LLVM IRのオブジェクトでありFunctionやModuleなどを構成するクラスを使います
Value *NumberAST::codegen() {
  // 64bit整数型のValueを返す
  return ConstantInt::get(Context, APInt(64, Val, true));
}

Value *LogErrorV(const char *str) {
  LogError(str);
  return nullptr;
}

Value *BinaryAST::codegen() {
  // 二項演算子の両方の引数をllvm::Valueにする。
  Value *L = LHS->codegen();
  Value *R = RHS->codegen();
  if (!L || !R)
    return nullptr;

  switch (Op) {
  case '+':
    // LLVM IR Builerを使い、この二項演算のIRを作る
    return Builder.CreateAdd(L, R, "addtmp");
  // TODO 1.7: '-'と'*'に対してIRを作ってみよう
  // 上の行とhttps://llvm.org/doxygen/classllvm_1_1IRBuilder.htmlを参考のこと
  // case '-': ...

  default:
    return LogErrorV("invalid binary operator");
  }
}

Function *PrototypeAST::codegen() {
  // MC言語では変数の型も関数の返り値もintの為、関数の返り値をInt64にする。
  std::vector<Type *> prototype(Args.size(), Type::getInt64Ty(Context));
  FunctionType *FT =
      FunctionType::get(Type::getInt64Ty(Context), prototype, false);
  // https://llvm.org/doxygen/classllvm_1_1Function.html
  // llvm::Functionは関数のIRを表現するクラス
  Function *F =
      Function::Create(FT, Function::ExternalLinkage, Name, myModule.get());

  // 引数の名前を付ける
  unsigned i = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[i++]);

  return F;
}

Function *FunctionAST::codegen() {
  // この関数が既にModuleに登録されているか確認
  Function *function = myModule->getFunction(Proto->getName());
  // 関数名が見つからなかったら、新しくこの関数のIRクラスを作る。
  if (!function)
    function = Proto->codegen();
  if (!function)
    return nullptr;

  // エントリーポイントを作る
  BasicBlock *BB = BasicBlock::Create(Context, "entry", function);
  Builder.SetInsertPoint(BB);

  // 関数のbody(ExprASTから継承されたNumberASTかBinaryAST)をcodegenする
  if (Value *RetVal = Body->codegen()) {
    // returnのIRを作る
    Builder.CreateRet(RetVal);

    // https://llvm.org/doxygen/Verifier_8h.html
    // 関数の検証
    verifyFunction(*function);

    return function;
  }

  // もし関数のbodyがnullptrなら、この関数をModuleから消す。
  function->eraseFromParent();
  return nullptr;
}

//===----------------------------------------------------------------------===//
// MC コンパイラエントリーポイント
// mc.cppでMainLoop()が呼ばれます。MainLoopは各top level expressionに対して
// HandleTopLevelExpressionを呼び、その中でASTを作り再帰的にcodegenをしています。
//===----------------------------------------------------------------------===//

static std::string streamstr;
static llvm::raw_string_ostream stream(streamstr);
// その名の通りtop level expressionをcodegenします。例えば、「2+1;3+3;」というファイルが
// 入力だった場合、この関数は最初の「2+1」をcodegenして返ります。(そしてMainLoopからまだ呼び出されます)
static void HandleTopLevelExpression() {
  // ここでテキストファイルを全てASTにします。
  if (auto FnAST = ParseTopLevelExpr()) {
    // できたASTをcodegenします。
    if (auto *FnIR = FnAST->codegen()) {
      streamstr = "";
      FnIR->print(stream);
    }
  } else {
    // エラー
    getNextToken();
  }
}

static void MainLoop() {
myModule = llvm::make_unique<Module>("my cool jit", Context);
  while (true) {
    switch (CurTok) {
    case tok_eof:
      // ここで最終的なLLVM IRをプリントしています。
      fprintf(stderr, "%s", stream.str().c_str());
      return;
    case ';': // ';'で始まった場合、無視します
      getNextToken();
      break;
    default:
      HandleTopLevelExpression();
      break;
    }
  }
}
