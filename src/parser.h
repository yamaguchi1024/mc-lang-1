//===----------------------------------------------------------------------===//
// AST
// ここのセクションでは、AST（構文解析木）の葉を定義している。
// MCコンパイラの根幹であるクラスで、ParserだけではなくCodeGenでも使われているので
// 非常に重要。イメージとしては、Lexerによって次のトークンを取ってきて、それが例えば
// 数値リテラルであったらNumberASTに値を格納し、そのポインタを親ノードが保持する。
// 全てのコードを無事にASTとして表現できたら、後述するcodegenを再帰的に呼び出す事に
// よりオブジェクトファイルを生成する。
//===----------------------------------------------------------------------===//

namespace {
    // ExprAST - `5+2`や`2*10-2`等のexpressionを表すクラス
    class ExprAST {
        public:
            virtual ~ExprAST() = default;
            virtual Value *codegen() = 0;
    };

    // NumberAST - `5`や`2`等の数値リテラルを表すクラス
    class NumberAST : public ExprAST {
        // 実際に数値の値を保持する変数
        uint64_t Val;

        public:
            NumberAST(uint64_t Val) : Val(Val) {}
            Value *codegen() override;
    };

    // BinaryAST - `+`や`*`等の二項演算子を表すクラス
    class BinaryAST : public ExprAST {
        char Op;
        std::unique_ptr<ExprAST> LHS, RHS;

        public:
            BinaryAST(char Op, std::unique_ptr<ExprAST> LHS,
                    std::unique_ptr<ExprAST> RHS)
                : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

            Value *codegen() override;
    };

    // 第一回の課題では関数の定義・呼び出しは扱いませんが、後々のつながりを良くする為と
    // オブジェクトファイルのエントリーポイントを作り"それらしく"するために関数と関数シグネチャー
    // のクラスを作ります。
    //
    // PrototypeAST - 関数シグネチャーのクラスで、関数の名前と引数の名前を表すクラスです。
    class PrototypeAST {
        std::string Name;
        std::vector<std::string> Args;

        public:
            PrototypeAST(const std::string &Name, std::vector<std::string> Args)
                : Name(Name), Args(std::move(Args)) {}

            Function *codegen();
            const std::string &getName() const { return Name; }
    };

    // FunctionAST - 関数シグネチャー(PrototypeAST)に加えて関数のbody(C++で言うint foo) {...}の中身)を
    // 表すクラスです。
    class FunctionAST {
        std::unique_ptr<PrototypeAST> Proto;
        std::unique_ptr<ExprAST> Body;

        public:
            FunctionAST(std::unique_ptr<PrototypeAST> Proto,
                    std::unique_ptr<ExprAST> Body)
                : Proto(std::move(Proto)), Body(std::move(Body)) {}

            Function *codegen();
    };
} // end anonymous namespace

//===----------------------------------------------------------------------===//
// Parser
// Parserでは、ParseTopLevelExprから始まり、お互いを再帰的に呼び出すことでAST Tree(構文解析木)
// を構成していく。
//===----------------------------------------------------------------------===//

// CurTokは現在のトークン(tok_number, tok_eof, または')'や'+'などの場合そのascii)が
// 格納されている。
// getNextTokenにより次のトークンを読み、Curtokを更新する。
static int CurTok;
static int getNextToken() { return CurTok = lexer.gettok(); }

// 二項演算子の結合子をmc.cppで定義している。
static std::map<char, int> BinopPrecedence;

// GetTokPrecedence - 二項演算子の結合度を取得
// もし現在のトークンが二項演算子ならその結合度を返し、そうでないなら-1を返す。
static int GetTokPrecedence() {
    if (!isascii(CurTok))
        return -1;

    int tokprec = BinopPrecedence[CurTok];
    if (tokprec <= 0)
        return -1;
    return tokprec;
}

// LogError - エラーを表示しnullptrを返してくれるエラーハンドリング関数
std::unique_ptr<ExprAST> LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

// Forward declaration
static std::unique_ptr<ExprAST> ParseExpression();

// 数値リテラルをパースする関数。
static std::unique_ptr<ExprAST> ParseNumberExpr() {
    // NumberASTのValにlexerからnumValを読んできて、セットする。
    auto Result = llvm::make_unique<NumberAST>(lexer.getNumVal());
    getNextToken(); // トークンを一個進めて、returnする。
    return std::move(Result);
}

// TODO 1.5: 括弧を実装してみよう
// 括弧は`'(' ExprAST ')'`の形で表されます。最初の'('を読んだ後、次のトークンは
// ExprAST(NumberAST or BinaryAST)のはずなのでそれをパースし、最後に')'で有ることを
// 確認します。
static std::unique_ptr<ExprAST> ParseParenExpr() {
    // 1. ParseParenExprが呼ばれた時、CurTokは'('のはずなので、括弧の中身を得るために
    //    トークンを進めます。e.g. getNextToken()
    // 2. 現在のトークンはExprASTのはずなので、ParseExpression()を呼んでパースします。
    //    もし返り値がnullptrならエラーなのでParseParenExprでもnullptrを返します。
    // 3. 2で呼んだParseExpressionではトークンが一つ進められて帰ってきているはずなので、
    // 　 CurTokが')'かどうかチェックします。もし')'でなければ、LogErrorを用いてエラーを出して下さい。
    // 4. getNextToken()を呼んでトークンを一つ進め、2で呼んだParseExpressionの返り値を返します。
    //
    // 課題を解く時はこの行を消してここに実装して下さい。
    return nullptr;
}

// ParsePrimary - NumberASTか括弧をパースする関数
static std::unique_ptr<ExprAST> ParsePrimary() {
    switch (CurTok) {
        default:
            return LogError("unknown token when expecting an expression");
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
    }
}

// TODO 1.6: 二項演算のパーシングを実装してみよう
// このパーサーの中で一番重要と言っても良い、二項演算子のパーシングを実装します。
// LHSに二項演算子の左側が入った状態で呼び出され、LHSとRHSと二項演算子がペアになった
// 状態で返ります。
static std::unique_ptr<ExprAST> ParseBinOpRHS(int CallerPrec,
        std::unique_ptr<ExprAST> LHS) {
    // 課題を解く時はこの行を消して下さい。
    return LHS;
    while (true) {
        // 1. 現在の二項演算子の結合度を取得する。 e.g. int tokprec = GetTokPrecedence();

        // 2. もし呼び出し元の演算子(CallerPrec)よりも結合度が低ければ、ここでは演算をせずにLHSを返す。
        // 例えば、「4*2+3」ではここで'2'が返るはずで、「4+2*3」ではここでは返らずに処理が進み、
        // '2*3'がパースされた後に返る。

        // 3. 二項演算子をセットする。e.g. int BinOp = CurTok;

        // 4. 次のトークン(二項演算子の右のexpression)に進む。

        // 5. 二項演算子の右のexpressionをパースする。 e.g. auto RHS = ParsePrimary();

        // GetTokPrecedence()を呼んで、もし次のトークンも二項演算子だった場合を考える。
        // もし次の二項演算子の結合度が今の演算子の結合度よりも強かった場合、ParseBinOpRHSを再帰的に
        // 呼んで先に次の二項演算子をパースする。
        //int NextPrec = GetTokPrecedence();
        //if (tokprec < NextPrec) {
        //    RHS = ParseBinOpRHS(tokprec + 1, std::move(RHS));
        //    if (!RHS)
        //        return nullptr;
        //}

        // LHS, RHSをBinaryASTにしてLHSに代入する。
        //LHS = llvm::make_unique<BinaryAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

// ExprASTは1. 数値リテラル 2. '('から始まる演算 3. 二項演算子の三通りが考えられる為、
// 最初に1,2を判定して、そうでなければ二項演算子だと思う。
static std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
}

// パーサーのトップレベル関数。まだ関数定義は実装しないので、今のmc言語では
// __anon_exprという関数がトップレベルに作られ、その中に全てのASTが入る。
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    if (auto E = ParseExpression()) {
        auto Proto = llvm::make_unique<PrototypeAST>("__anon_expr",
                std::vector<std::string>());
        return llvm::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}
