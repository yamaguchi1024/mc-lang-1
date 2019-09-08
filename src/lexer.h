//===----------------------------------------------------------------------===//
// Lexer
// このセクションでは、Lexerというテキストから「数字」や「演算子」をまとまり毎に
// 分割するクラスを実装している。
//
// 全体的な流れとしては、gettokをParserから呼ぶことにより「次のトークン」を読み、
// それが数値リテラルだった場合はnumValという変数にセットする。
//===----------------------------------------------------------------------===//

// このLexerでは、EOFと数値以外は[0-255]を返す('+'や'-'を含む)。
enum Token {
    tok_eof = -1,
    tok_number = -5,
};

class Lexer {
    public:
        // gettok - トークンが数値だった場合はnumValにその数値をセットした上でtok_number
        // を返し、'+'や他のunknown tokenだった場合はそのascii codeを返します。
        int gettok() {
            static int lastChar = getNextChar(iFile);

            // スペースをスキップ
            while (isspace(lastChar))
                lastChar = getNextChar(iFile);

            // TODO 1.3: 数字のパーシングを実装してみよう
            // 今読んでいる文字(lastChar)が数字だった場合(isdigit(lastChar) == true)は、
            // 数字が終わるまで読み、その数値をnumValにセットする。
            // 1. lastCharが数字かチェックする e.g. if (isdigit(lastChar)) {..
            // 2. 数字を格納するための変数を作る e.g. std::string str = "";
            // 3. 入力が'456'という数字だったとして、今の文字は'4'なので、
            //    次の文字が数字ではなくなるまで読み込み、2で作った変数に格納する
            //    e.g. str += lastChar;
            // 4. 次の文字が数字でなくなるまで読み込む。ファイルから次の文字を読み込むのは、
            //    getNextChar(iFile)で出来る。 e.g. while(isdigit(lastChar = getNextChar(iFile))) {..
            // 5. 2と同様に、4が数字であれば2で作った変数に格納する
            // 6. 次の文字が数字でなくなったら、setnumValを読んでnumValに数値を格納する。その際、
            //   strtodというcharからintにする関数が利用できる。
            // 7. このトークンは数値だったので、tok_numberをreturnする。
            //
            // ここに実装して下さい

            // TODO 1.4: コメントアウトを実装してみよう
            // '#'を読んだら、その行の末尾まで無視をするコメントアウトを実装する。
            // 1. 今の文字(LastChar)が'#'かどうかをチェック
            // 2. lastCharに次のトークンを読み込む(getNextChar(iFile)を使う)
            // 3. lastCharが'\n'か、EOFになるまで読み込む。e.g. while(lastChar != EOF && lastChar != '\n')
            // 4. lastCharがEOFでない場合、行末まで読み込んだので次のトークンに進むためにgettok()をreturnする。
            //
            // ここに実装して下さい

            // EOFならtok_eofを返す
            if (iFile.eof())
                return tok_eof;

            // tok_numberでもtok_eofでもなければそのcharのasciiを返す
            int thisChar = lastChar;
            lastChar = getNextChar(iFile);
            return thisChar;
        }

        // 数字を格納するnumValのgetter, setter
        uint64_t getNumVal() { return numVal; }
        void setnumVal(uint64_t numval) { numVal = numval; }

        void initStream(std::string fileName) { iFile.open(fileName); }

    private:
        std::ifstream iFile;
        uint64_t numVal;
        static char getNextChar(std::ifstream &is) {
            char c;
            if (is.good())
                is.get(c);

            return c;
        }
};
