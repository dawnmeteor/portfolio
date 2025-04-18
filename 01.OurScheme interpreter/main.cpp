# include <iostream>
# include <cstdio>
# include <cstdlib>
# include <string>
# include <vector>
# include <iomanip>
# include <map>

using namespace std ;

enum Token_type { LEFT_PAREN, RIGHT_PAREN, INT, STRING, DOT, FLOAT, NIL, T, QUOTE, SYMBOL, ERROR };

struct Token_detail {
  int line ;
  int column ; // the place of first char
  Token_type type ;
  string name_str ;
  int name_int ;
  float name_float ;
  char name_char ;
};

struct Tree_Node {
  Token_detail token ;
  bool full ;
  bool eval ;
  Tree_Node * left ;
  Tree_Node * right ;
};

typedef Tree_Node * Tree_Node_Ptr ;

class Token {
private:
  int m_line ;
  int m_column ;
  int m_tokenPlace ;

  bool m_errorToken ;
  bool m_eof ;

  vector<Token_detail> m_tokenList ;
  vector<Token_detail> m_tokenList_ST ;

  map<string, Tree_Node_Ptr> m_binding ;
  vector<string> m_function ;

  string m_error_message ;
  string m_no_return ;

public:
  Token() {
    m_line = 1 ;
    m_column = 1 ;
    m_tokenPlace = 0 ;
    m_errorToken = false ;
    m_eof = false ;
    SetFunction() ;
    m_error_message = "ERROR" ;
    m_no_return = "m_no_return" ;
  } // Token()

  // ================== GetToken =============================

  char GetNextChar() {
    char temp ;
    scanf( "%c", &temp ) ;
    if ( temp != '\n' )
      m_column++ ;
    else {
      m_line++ ;
      m_column = 1 ;
    } // else

    return temp ;
  } // GetNextChar()

  void SkipWhite_space_char() {
    char temp ;
    temp = cin.peek() ;
    while ( temp == ' ' || temp == '\t' || temp == '\n' ) {
      GetNextChar() ;
      temp = cin.peek() ;
    } // while
  } // SkipWhite_space_char()

  int SkipWhite_space_char_token() {
    int num = 0 ;
    char temp ;
    temp = cin.peek() ;
    while ( temp == ' ' || temp == '\t' ) {
      GetNextChar() ;
      num++ ;
      temp = cin.peek() ;
    } // while

    if ( temp == ';' ) {
      while ( temp != '\n' ) {
        GetNextChar() ;
        temp = cin.peek() ;
      } // while
    } // if

    return num ;
  } // SkipWhite_space_char_token()

  void SkipComment() {
    char temp ;
    temp = cin.peek() ;
    while ( temp != '\n' && temp != EOF ) {
      GetNextChar() ;
      temp = cin.peek() ;
    } // while

    GetNextChar() ;
  } // SkipComment()

  bool GetString( string &nextToken ) {
    char temp, item ;
    bool finish = false ;
    nextToken = GetNextChar() ; // get '"'
    temp = cin.peek() ;
    while ( !finish ) {
      if ( item == '"' && nextToken.length() > 1 && nextToken[nextToken.length()-2] != '\\' )
        finish = true ;
      else if ( temp == '\n' )
        return false ;
      else {
        item = GetNextChar() ;
        nextToken = nextToken + item ;
        temp = cin.peek() ;
      } // else
    } // while

    return true;
  } // GetString()

  void GetPAREN( string &nextToken ) {
    char temp ;
    nextToken = GetNextChar() ;
    temp = cin.peek() ;
    if ( nextToken == "(" ) {
      while ( temp == ' ' || temp == '\t' || temp == '\n' ) {
        SkipWhite_space_char() ;
        temp = cin.peek() ;
        if ( temp == ';' )
          SkipComment() ;
        temp = cin.peek() ;
      } // while

      if ( temp == ')' )
        nextToken = nextToken + GetNextChar() ;
    } // if

  } // GetPAREN()

  void GetQUOTE( string &nextToken ) {
    nextToken = GetNextChar() ;
  } // GetQUOTE()

  void GetOther( string &nextToken ) {
    char temp ;
    nextToken = GetNextChar() ;
    temp = cin.peek() ;
    while ( temp != ' ' && temp != '\t' && temp != '\n' && temp != '('
            && temp != ')' && temp != ';' && temp != '\"' && temp != '\'' ) {
      nextToken = nextToken + GetNextChar() ;
      temp = cin.peek() ;
    } // while

  } // GetOther()

  bool IsDigit( char temp ) {
    if ( temp == '0' )
      return true ;
    else if ( temp == '1' )
      return true ;
    else if ( temp == '2' )
      return true ;
    else if ( temp == '3' )
      return true ;
    else if ( temp == '4' )
      return true ;
    else if ( temp == '5' )
      return true ;
    else if ( temp == '6' )
      return true ;
    else if ( temp == '7' )
      return true ;
    else if ( temp == '8' )
      return true ;
    else if ( temp == '9' )
      return true ;
    else
      return false ;
  } // IsDigit()

  bool IsINT( string temp_str ) {
    if ( temp_str.length() == 1 && ( temp_str[0] == '+' || temp_str[0] == '-' ) )
      return false ;
    if ( temp_str[0] == '+' || temp_str[0] == '-' || IsDigit( temp_str[0] ) ) {
      for ( int i = 1 ; i < temp_str.length() ; i++ ) {
        if ( !IsDigit( temp_str[i] ) )
          return false ;
      } // for

      return true ;
    } // if

    return false ;
  } // IsINT()

  bool IsFLOAT( string temp_str ) {
    bool dot = false, digit = false ;
    if ( temp_str.length() == 1 && ( temp_str[0] == '+' || temp_str[0] == '-' ) )
      return false ;
    if ( temp_str[0] == '.' || temp_str[0] == '+' || temp_str[0] == '-' || IsDigit( temp_str[0] ) ) {
      if ( temp_str[0] == '.' )
        dot = true ;
      if ( IsDigit( temp_str[0] ) ) // digit appeared
        digit = true ;
      for ( int i = 1 ; i < temp_str.length() ; i++ ) {
        if ( temp_str[i] == '.' ) {
          if ( dot ) // dot has appeared
            return false ;
          else // dot first appear
            dot = true ;
        } // if
        else if ( !IsDigit( temp_str[i] ) )
          return false ;
        else // digit appeared
          digit = true ;
      } // for

      if ( !digit ) // digit not appeared
        return false ;

      return true ;
    } // if

    return false ;
  } // IsFLOAT()

  float Stof( string token ) {
    float result = 0 ;
    string temp ;

    if ( token[0] == '+' || token[0] == '-' ) {
      temp = token[0] ;
      token.erase( token.begin() ) ;
    } // if

    if ( token[0] == '.' )
      token = "0" + token ;

    result = atof( token.c_str() ) ;

    if ( temp == "-" )
      result = result * -1 ;

    return result ;
  } // Stof()

  Token_detail GetToken() {
    Token_detail nextToken ;
    char nextChar ;

    SkipWhite_space_char() ;

    nextChar = cin.peek() ;

    while ( nextChar == ';' ) { // comment
      SkipComment() ;
      SkipWhite_space_char() ;
      nextChar = cin.peek() ;
    } // while

    nextToken.line = m_line ;
    nextToken.column = m_column ;

    if ( nextChar == '"' ) {
      if ( GetString( nextToken.name_str ) )
        nextToken.type = STRING ;
      else {
        cout << "ERROR (no closing quote) : END-OF-LINE encountered at Line "
             << nextToken.line << " Column " << nextToken.column + nextToken.name_str.length() << endl ;
        nextToken.type = ERROR ;
        m_errorToken = true ;
      } // else

    } // if
    else if ( nextChar == '(' ) {
      GetPAREN( nextToken.name_str ) ;
      if ( nextToken.name_str == "()" )
        nextToken.type = NIL ;
      else
        nextToken.type = LEFT_PAREN ;
    } // else if
    else if ( nextChar == ')' ) {
      GetPAREN( nextToken.name_str ) ;
      nextToken.type = RIGHT_PAREN ;
    } // else if
    else if ( nextChar == '\'' ) {
      GetQUOTE( nextToken.name_str ) ;
      nextToken.type = QUOTE ;
    } // else if
    else {
      GetOther( nextToken.name_str ) ;
      if ( nextToken.name_str == "nil" || nextToken.name_str == "#f" )
        nextToken.type = NIL ;
      else if ( nextToken.name_str == "t" || nextToken.name_str == "#t" )
        nextToken.type = T ;
      else if ( nextToken.name_str == "." )
        nextToken.type = DOT ;
      else if ( IsINT( nextToken.name_str ) ) {
        nextToken.type = INT ;
        if ( nextToken.name_str[0] == '+' ) {
          nextToken.name_str.erase( nextToken.name_str.begin() ) ;
          nextToken.name_int = atoi( nextToken.name_str.c_str() ) ;
        } // if
        else if ( nextToken.name_str[0] == '-' ) {
          nextToken.name_str.erase( nextToken.name_str.begin() ) ;
          nextToken.name_int = atoi( nextToken.name_str.c_str() ) ;
          nextToken.name_int = nextToken.name_int * -1 ;
        } // else if
        else
          nextToken.name_int = atoi( nextToken.name_str.c_str() ) ;
      } // else if
      else if ( IsFLOAT( nextToken.name_str ) ) {
        nextToken.type = FLOAT ;
        nextToken.name_float = Stof( nextToken.name_str ) ;
      } // else if
      else
        nextToken.type = SYMBOL ;
    } // else

    m_tokenList.push_back( nextToken ) ;
    return nextToken ;
  } // GetToken()

  // =========================================================

  // ====================== 文法 ==============================

  bool ATOM( Token_detail token ) {
    if ( token.type == DOT )
      return false ;
    else if ( token.type == LEFT_PAREN )
      return false ;
    else if ( token.type == RIGHT_PAREN )
      return false ;
    else if ( token.type == QUOTE )
      return false ;
    else if ( token.type == ERROR )
      return false ;
    else
      return true ;
  } // ATOM()

  bool S_exp() {
    bool dot = false ;
    Token_detail nextToken ;

    SkipWhite_space_char() ;
    while ( cin.peek() == ';' ) { // comment
      SkipComment() ;
      SkipWhite_space_char() ;
    } // while

    if ( cin.peek() == EOF ) {
      m_eof = true ;
      return false ;
    } // if

    nextToken = GetToken() ;
    if ( ATOM( nextToken ) )
      return true ;
    else if ( nextToken.type == QUOTE ) {
      if ( S_exp() )
        return true ;
      else
        return false ;
    } // else if
    else if ( nextToken.type == LEFT_PAREN ) {
      bool s_exp = S_exp() ;
      if ( s_exp ) {
        while ( s_exp ) {
          s_exp = S_exp() ;
        } // while

        if ( m_errorToken )
          return false ;

        if ( m_tokenList.at( m_tokenList.size() - 1 ).type == DOT
             && m_tokenList.at( m_tokenList.size() - 2 ).type != LEFT_PAREN ) {
          if ( !S_exp() )
            return false ;
          nextToken = GetToken() ;
          dot = true ;
        } // if

        if ( !dot && m_tokenList.at( m_tokenList.size() - 1 ).type == RIGHT_PAREN )
          return true ;
        else if ( dot && nextToken.type == RIGHT_PAREN )
          return true ;
        else if ( m_tokenList.at( m_tokenList.size() - 1 ).type == ERROR )
          return false ;
        else {
          cout << "ERROR (unexpected token) : ')' expected when token at Line "
               << nextToken.line << " Column " << nextToken.column << " is >>"
               << nextToken.name_str << "<<" << endl ;
          m_errorToken = true ;
          m_tokenList.at( m_tokenList.size() - 1 ).type = ERROR ;
          return false ;
        } // else


      } // if
      else {
        m_errorToken = true ;
        return false ;
      } // else

    } // else if
    else {
      // m_errorToken = nextToken ;
      return false ;
    } // else


  } // S_exp()

  bool ReadSExp() {
    if ( S_exp() )
      return true ;
    else if ( m_tokenList.at( m_tokenList.size()-1 ).type == ERROR || m_eof )
      return false ;
    else {
      cout << "ERROR (unexpected token) : atom or '(' expected when token at Line "
           << m_tokenList.at( m_tokenList.size()-1 ).line
           << " Column " << m_tokenList.at( m_tokenList.size()-1 ).column << " is >>"
           << m_tokenList.at( m_tokenList.size()-1 ).name_str << "<<" << endl ;
      return false ;
    } // else
  } // ReadSExp()

  bool Get_m_eof() {
    return m_eof ;
  } // Get_m_eof()

  // =========================================================

  // ====================== 建樹 ==============================

  int Get_m_tokenList_size() {
    return m_tokenList.size() ;
  } // Get_m_tokenList_size()

  Token_detail GetNextToken() {

    Token_detail next ;
    next = m_tokenList.at( m_tokenPlace ) ;
    m_tokenPlace++ ;
    return next ;

  } // GetNextToken()

  Tree_Node_Ptr S_exp( Token_detail nextToken, Tree_Node_Ptr walk ) {

    Tree_Node_Ptr temp = NULL ;
    Tree_Node_Ptr temp_nil = NULL ;
    Tree_Node_Ptr temp_quote = NULL ;

    if ( walk == NULL ) {
      walk = new Tree_Node ;
      walk->token.type = LEFT_PAREN ;
      walk->token.name_str = "(" ;
      walk->eval = false ;
      walk->left = NULL ;
      walk->right = NULL ;
    } // if

    if ( ATOM( nextToken ) ) {
      walk->token = nextToken ;
      return walk ;
    } // if
    else if ( nextToken.type == QUOTE ) {
      temp_quote = walk ;
      temp_quote->left = new Tree_Node ;
      temp_quote->left->token.type = QUOTE ;
      temp_quote->left->token.name_str = "quote" ;
      temp_quote->left->eval = false ;
      temp_quote->left->left = NULL ;
      temp_quote->left->right = NULL ;
      temp_quote->right = new Tree_Node ;
      temp_quote = temp_quote->right ;
      temp_quote->token.type = LEFT_PAREN ;
      temp_quote->token.name_str = "(" ;
      temp_quote->eval = false ;
      temp_quote->left = NULL ;
      temp_quote->right = NULL ;
      nextToken = GetNextToken() ;
      temp_quote->left = S_exp( nextToken, temp_quote->left ) ;
      temp_quote->right = new Tree_Node ;
      temp_quote = temp_quote->right ;
      temp_quote->token.type = NIL ;
      temp_quote->token.name_str = "nil" ;
      temp_quote->eval = false ;
      temp_quote->right = NULL ;
      temp_quote->left = NULL ;
      return walk ;
    } // else if
    else if ( nextToken.type == LEFT_PAREN ) {
      nextToken = GetNextToken() ;
      temp = S_exp(  nextToken, walk->left ) ;
      walk->left = temp ;

      nextToken = GetNextToken() ;
      Tree_Node_Ptr right = walk ;
      while ( nextToken.type != DOT && nextToken.type != RIGHT_PAREN ) {
        right->right = new Tree_Node ;
        right = right->right ;
        right->token.type = LEFT_PAREN ;
        right->token.name_str = "(" ;
        right->eval = false ;
        right->right = NULL ;
        right->left = NULL ;
        temp = S_exp( nextToken, right->left ) ;
        right->left = temp ;
        nextToken = GetNextToken() ;
      } // while

      if ( nextToken.type == DOT ) {
        nextToken = GetNextToken() ;
        temp = S_exp( nextToken, right->right ) ;
        right->right = temp ;
        nextToken = GetNextToken() ;
      } // if
      else {
        temp_nil = new Tree_Node ;
        temp_nil->token.type = NIL ;
        temp_nil->token.name_str = "nil" ;
        temp_nil->eval = false ;
        temp_nil->right = NULL ;
        temp_nil->left = NULL ;
        right->right = temp_nil ;
      } // else

      if ( nextToken.type == RIGHT_PAREN ) {
        return walk ;
      } // if

    } // else if

    return NULL ;
  } // S_exp()

  Tree_Node_Ptr BuildTree() {

    Token_detail nextToken = GetNextToken() ;
    return S_exp( nextToken, NULL ) ;

  } // BuildTree()

  int FindTreeHigh( Tree_Node_Ptr tree ) {
    int left = 0, right = 0 ;

    if ( tree->left != NULL )
      left = FindTreeHigh( tree->left ) ;
    if ( tree->right != NULL )
      right = FindTreeHigh( tree->right ) ;

    if ( left >= right )
      return left+1 ;
    else
      return right+1 ;
  } // FindTreeHigh()

  bool FindExit( Tree_Node_Ptr tree ) {
    if ( FindTreeHigh( tree ) != 2 )
      return false ;
    else if ( tree->left->token.name_str == "exit" && tree->right->token.type == NIL )
      return true ;
    else
      return false ;
  } // FindExit()

  // =========================================================

  // ====================== eval =============================

  bool Clean_Environment( Tree_Node_Ptr tree ) {
    if ( FindTreeHigh( tree ) != 2 )
      return false ;
    else if ( tree->left->token.name_str == "clean-environment" && tree->right->token.type == NIL ) {
      m_binding.clear() ;
      return true ;
    } // else if
    else
      return false ;
  } // Clean_Environment()

  bool IsPureList( Tree_Node_Ptr tree ) {

    Tree_Node_Ptr walk ;
    walk = tree ;
    while ( walk->right != NULL ) {
      walk = walk->right ;
    } // while

    if ( walk->token.type == NIL ) // if (...) is a pure list
      return true ;
    else // // if (...) is not a pure list
      return false ;

  } // IsPureList()

  void SetFunction() {
    m_function.push_back( "cons" ) ;
    m_function.push_back( "list" ) ;
    m_function.push_back( "define" ) ;
    m_function.push_back( "car" ) ;
    m_function.push_back( "cdr" ) ;
    m_function.push_back( "atom?" ) ;
    m_function.push_back( "pair?" ) ;
    m_function.push_back( "list?" ) ;
    m_function.push_back( "null?" ) ;
    m_function.push_back( "integer?" ) ;
    m_function.push_back( "real?" ) ;
    m_function.push_back( "number?" ) ;
    m_function.push_back( "string?" ) ;
    m_function.push_back( "boolean?" ) ;
    m_function.push_back( "symbol?" ) ;
    m_function.push_back( "+" ) ;
    m_function.push_back( "-" ) ;
    m_function.push_back( "*" ) ;
    m_function.push_back( "/" ) ;
    m_function.push_back( "not" ) ;
    m_function.push_back( "and" ) ;
    m_function.push_back( "or" ) ;
    m_function.push_back( ">" ) ;
    m_function.push_back( ">=" ) ;
    m_function.push_back( "<" ) ;
    m_function.push_back( "<=" ) ;
    m_function.push_back( "=" ) ;
    m_function.push_back( "string-append" ) ;
    m_function.push_back( "string>?" ) ;
    m_function.push_back( "string<?" ) ;
    m_function.push_back( "string=?" ) ;
    m_function.push_back( "eqv?" ) ;
    m_function.push_back( "equal?" ) ;
    m_function.push_back( "begin" ) ;
    m_function.push_back( "if" ) ;
    m_function.push_back( "cond" ) ;
    m_function.push_back( "quote" ) ;
    m_function.push_back( "clean-environment" ) ;
    m_function.push_back( "exit" ) ;
  } // SetFunction()

  bool IsFunction( string token ) {

    for ( int i = 0 ; i < m_function.size() ; i++ ) {
      if ( token == m_function[i] )
        return true ;
    } // for

    return false ;

  } // IsFunction()

  int NumOfArg( Tree_Node_Ptr tree ) {
    int order = 0 ;
    Tree_Node_Ptr walk = tree ;

    while ( walk->right != NULL && walk->right->token.type != NIL ) {
      order++ ;
      walk = walk->right ;
    } // while

    return order ;

  } // NumOfArg()

  bool CheckTree( Tree_Node_Ptr one, Tree_Node_Ptr two ) {
    bool equal = false ;

    if ( one->token.type == INT && two->token.type == INT ) {
      if ( one->token.name_int == two->token.name_int )
        equal = true ;
      else
        equal = false ;
    } // if
    else if ( one->token.type == FLOAT && two->token.type == FLOAT ) {
      if ( one->token.name_float == two->token.name_float )
        equal = true ;
      else
        equal = false ;
    } // else if
    else if ( one->token.type == SYMBOL && two->token.type == SYMBOL ) {
      if ( one->token.name_str == two->token.name_str )
        equal = true ;
      else
        equal = false ;
    } // else if
    else if ( one->token.type == STRING && two->token.type == STRING ) {
      if ( one->token.name_str == two->token.name_str )
        equal = true ;
      else
        equal = false ;
    } // else if
    else if ( one->token.type == T && two->token.type == T ) {
      equal = true ;
    } // else if
    else if ( one->token.type == NIL && two->token.type == NIL ) {
      equal = true ;
    } // else if
    else if ( one->token.type == LEFT_PAREN && two->token.type == LEFT_PAREN ) {
      equal = true ;
    } // else if
    else if ( one->token.type == QUOTE && two->token.type == QUOTE ) {
      equal = true ;
    } // else if

    if ( equal ) {
      // 先看左邊 aaa
      if ( one->left != NULL && two->left != NULL )
        equal = CheckTree( one->left, two->left ) ;
      else if ( one->left == NULL && two->left == NULL )
        return true ;
      else
        return false ;

      // 再看右邊 aaa
      if ( equal && one->right != NULL && two->right != NULL )
        equal = CheckTree( one->right, two->right ) ;
      else if ( one->left == NULL && two->left == NULL )
        return true ;
      else
        return false ;

    } // if
    else
      return false ;

    return equal ; // 右邊回來的結果 aaa

  } // CheckTree()

  Tree_Node_Ptr Function_Work( Tree_Node_Ptr tree, int level, string function ) {
    map<string, Tree_Node_Ptr>:: iterator it ;
    Tree_Node_Ptr temp = new Tree_Node ;

    temp->left = NULL ;
    temp->right = NULL ;

    if ( function == "cons" ) {
      if ( NumOfArg( tree ) != 2 ) {
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        temp->token.type = LEFT_PAREN ;
        temp->token.name_str = "(" ;
        temp->eval = true ;
        temp->left = EVAL( tree->right->left, level+1 ) ;
        temp->right = EVAL( tree->right->right->left, level+1 ) ;
        return temp ;
      } // else
    } // if
    else if ( function == "list" ) {
      if ( NumOfArg( tree ) > 0 ) {
        Tree_Node_Ptr walk = tree ;
        Tree_Node_Ptr result = new Tree_Node ;
        temp = result ;
        temp->token.type = LEFT_PAREN ;
        temp->token.name_str = "(" ;
        temp->eval = true ;
        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          result->left = EVAL( walk->left, level+1 ) ;
          result->right = new Tree_Node ;
          result->right->token.type = LEFT_PAREN ;
          result->right->token.name_str = "(" ;
          result = result->right ;
        } // while

        result->token.type = NIL ;
        result->token.name_str = "nil" ;
        result->right = NULL ;
        result->left = NULL ;
        result = NULL ;

        return temp ;
      } // if
      else { // list 後面沒東西 aaa
        temp->token.type = NIL ;
        temp->token.name_str = "nil" ;
        temp->left = NULL ;
        temp->right = NULL ;
        temp->eval = true ;
        return temp ;
      } // else
    } // else if
    else if ( function == "'" || function == "quote" ) {
      if ( NumOfArg( tree ) != 1 ) {
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        temp = tree->right->left ;
        return temp ;
      } // else
    } // else if
    else if ( function == "define" ) {
      if ( NumOfArg( tree ) != 2 ) { // error
        cout << "ERROR (DEFINE format) : " ;
        Print_Tree( tree, 0, false ) ;
        throw m_error_message ;
      } // if
      else if ( tree->right->left->token.type != SYMBOL ) { // 第一個arg不是symbol
        cout << "ERROR (DEFINE format) : " ;
        Print_Tree( tree, 0, false ) ;
        throw m_error_message ;
      } // else if
      else if ( IsFunction( tree->right->left->token.name_str ) ) { // 第一個arg是function
        cout << "ERROR (DEFINE format) : " ;
        Print_Tree( tree, 0, false ) ;
        throw m_error_message ;
      } // else if
      else {
        temp = EVAL( tree->right->right->left, level+1 ) ;
        if ( !m_binding.empty() ) {
          it = m_binding.find( temp->token.name_str ) ;
          if ( it != m_binding.end() )
            m_binding.erase( it ) ;
        } // if

        m_binding[tree->right->left->token.name_str] = temp ;
        cout << tree->right->left->token.name_str << " defined" << endl ;
        throw m_no_return ;
      } // else
    } // else if
    else if ( function == "car" ) {
      if ( NumOfArg( tree ) != 1 ) {
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        temp = EVAL( tree->right->left, level+1 ) ;
        if ( temp->left == NULL && temp->right == NULL ) { // error
          cout << "ERROR (car with incorrect argument type) : " ;
          Print_Tree( temp, 0, false ) ;
          throw m_error_message ;
        } // if
        else {
          temp = temp->left ;
          temp->eval = true ;
          return temp ;
        } // else
      } // else

    } // else if
    else if ( function == "cdr" ) {
      if ( NumOfArg( tree ) != 1 ) {
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        temp = EVAL( tree->right->left, level+1 ) ;
        if ( temp->left == NULL && temp->right == NULL ) { // error
          cout << "ERROR (cdr with incorrect argument type) : " ;
          Print_Tree( temp, 0, false ) ;
          throw m_error_message ;
        } // if
        else {
          temp = temp->right ;
          if ( temp->token.type == NIL ) {
            temp = new Tree_Node ;
            temp->token.type = NIL ;
            temp->token.name_str = "nil" ;
            temp->eval = true ;
            temp->right = NULL ;
            temp->left = NULL ;
          } // if

          return temp ;
        } // else
      } // else

    } // else if
    else if ( function == "atom?" ) {
      if ( NumOfArg( tree ) != 1 ) {
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        temp = EVAL( tree->right->left, level+1 ) ;
        if ( ATOM( temp->token ) ) {
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "pair?" ) {
      if ( NumOfArg( tree ) != 1 ) {
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        temp = EVAL( tree->right->left, level+1 ) ;
        if ( temp->left != NULL && temp->right != NULL ) { // 不是葉子 aaa
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "list?" ) {
      if ( NumOfArg( tree ) != 1 ) {
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        temp = EVAL( tree->right->left, level+1 ) ;
        Tree_Node_Ptr walk = temp ;
        while ( walk->right != NULL ) {
          walk = walk->right ;
        } // while

        if ( walk->token.type == NIL ) { // 右下角是NIL
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "null?" || function == "not" ) {
      if ( NumOfArg( tree ) != 1 ) {
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        temp = EVAL( tree->right->left, level+1 ) ;
        if ( temp->token.type == NIL ) { // 是NIL
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "integer?" ) {
      if ( NumOfArg( tree ) != 1 ) {
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        temp = EVAL( tree->right->left, level+1 ) ;
        if ( temp->token.type == INT ) { // 是INT
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "real?" || function == "number?" ) {
      if ( NumOfArg( tree ) != 1 ) {
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        temp = EVAL( tree->right->left, level+1 ) ;
        if ( temp->token.type == INT || temp->token.type == FLOAT ) { // 是數字 aaa
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "string?" ) {
      if ( NumOfArg( tree ) != 1 ) {
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        temp = EVAL( tree->right->left, level+1 ) ;
        if ( temp->token.type == STRING ) { // 是STRING
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "boolean?" ) {
      if ( NumOfArg( tree ) != 1 ) {
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        temp = EVAL( tree->right->left, level+1 ) ;
        if ( temp->token.type == NIL || temp->token.type == T ) { // 是boolean
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "symbol?" ) {
      if ( NumOfArg( tree ) != 1 ) {
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        temp = EVAL( tree->right->left, level+1 ) ;
        if ( temp->token.type == SYMBOL ) { // 是SYMBOL
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "+" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        float sum = 0 ;
        bool used_float = false ;
        Tree_Node_Ptr walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type == INT ) { // arg 是 INT
            sum = sum + temp->token.name_int ;
          } // if
          else if ( temp->token.type == FLOAT ) { // arg 是 FLOAT
            sum = sum + temp->token.name_float ;
            used_float = true ;
          } // else if
          else { // 不是數字  error
            cout << "ERROR (+ with incorrect argument type) : " ;
            Print_Tree( temp, 0, false ) ;
            throw m_error_message ;
          } // else
        } // while

        temp = new Tree_Node ;
        temp->right = NULL ;
        temp->left = NULL ;
        temp->token.name_str = "" ; // 用來作為輸出的判斷 aaa
        temp->eval = true ;
        if ( used_float ) { // 有用過小數 aaa
          temp->token.type = FLOAT ;
          temp->token.name_float = sum ;
        } // if
        else {
          temp->token.type = INT ;
          temp->token.name_int = ( int ) sum ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "-" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        float sum = 1 ;
        bool used_float = false, set = false ;
        Tree_Node_Ptr walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type == INT ) { // arg 是 INT
            if ( !set ) {
              sum = temp->token.name_int ;
              set = true ;
            } // if
            else
              sum = sum - temp->token.name_int ;
          } // if
          else if ( temp->token.type == FLOAT ) { // arg 是 FLOAT
            if ( !set ) {
              sum = temp->token.name_float ;
              set = true ;
              used_float = true ;
            } // if
            else {
              sum = sum - temp->token.name_float ;
              used_float = true ;
            } // else

          } // else if
          else { // 不是數字  error
            cout << "ERROR (- with incorrect argument type) : " ;
            Print_Tree( temp, 0, false ) ;
            throw m_error_message ;
          } // else
        } // while

        temp = new Tree_Node ;
        temp->right = NULL ;
        temp->left = NULL ;
        temp->token.name_str = "" ; // 用來作為輸出的判斷 aaa
        temp->eval = true ;
        if ( used_float ) { // 有用過小數 aaa
          temp->token.type = FLOAT ;
          temp->token.name_float = sum ;
        } // if
        else {
          temp->token.type = INT ;
          temp->token.name_int = ( int ) sum ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "*" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        float sum = 1 ;
        bool used_float = false ;
        Tree_Node_Ptr walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type == INT ) { // arg 是 INT
            sum = sum * temp->token.name_int ;
          } // if
          else if ( temp->token.type == FLOAT ) { // arg 是 FLOAT
            sum = sum * temp->token.name_float ;
            used_float = true ;
          } // else if
          else { // 不是數字  error
            cout << "ERROR (* with incorrect argument type) : " ;
            Print_Tree( temp, 0, false ) ;
            throw m_error_message ;
          } // else
        } // while

        temp = new Tree_Node ;
        temp->right = NULL ;
        temp->left = NULL ;
        temp->token.name_str = "" ; // 用來作為輸出的判斷 aaa
        temp->eval = true ;
        if ( used_float ) { // 有用過小數 aaa
          temp->token.type = FLOAT ;
          temp->token.name_float = sum ;
        } // if
        else {
          temp->token.type = INT ;
          temp->token.name_int = ( int ) sum ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "/" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        float sum = 1 ;
        bool used_float = false, set = false ;
        Tree_Node_Ptr walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type == INT ) { // arg 是 INT
            if ( !set ) {
              sum = temp->token.name_int ;
              set = true ;
            } // if
            else if ( temp->token.name_int == 0 ) {
              cout << "ERROR (division by zero) : /" << endl ;
              throw m_error_message ;
            } // else if
            else
              sum = sum / temp->token.name_int ;
          } // if
          else if ( temp->token.type == FLOAT ) { // arg 是 FLOAT
            if ( !set ) {
              sum = temp->token.name_float ;
              set = true ;
              used_float = true ;
            } // if
            else if ( temp->token.name_float == 0 ) {
              cout << "ERROR (division by zero) : /" << endl ;
              throw m_error_message ;
            } // else if
            else {
              sum = sum / temp->token.name_float ;
              used_float = true ;
            } // else

          } // else if
          else { // 不是數字  error
            cout << "ERROR (/ with incorrect argument type) : " ;
            Print_Tree( temp, 0, false ) ;
            throw m_error_message ;
          } // else
        } // while

        temp = new Tree_Node ;
        temp->right = NULL ;
        temp->left = NULL ;
        temp->token.name_str = "" ; // 用來作為輸出的判斷 aaa
        temp->eval = true ;
        if ( used_float ) { // 有用過小數 aaa
          temp->token.type = FLOAT ;
          temp->token.name_float = sum ;
        } // if
        else {
          temp->token.type = INT ;
          temp->token.name_int = ( int ) sum ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "and" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        Tree_Node_Ptr walk = tree ;
        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type == NIL ) { // arg 是 NIL
            temp = new Tree_Node ;
            temp->token.type = NIL ;
            temp->token.name_str = "nil" ;
            temp->eval = true ;
            temp->right = NULL ;
            temp->left = NULL ;

            return temp ;
          } // if
        } // while

        return temp ; // 都不是NIL所以回傳最後一個 aaa
      } // else

    } // else if
    else if ( function == "or" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        Tree_Node_Ptr walk = tree ;
        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type != NIL ) { // arg 不是 NIL
            return temp ;
          } // if
        } // while

        temp = new Tree_Node ;
        temp->token.type = NIL ;
        temp->token.name_str = "nil" ;
        temp->eval = true ;
        temp->right = NULL ;
        temp->left = NULL ;

        return temp ; // 都是NIL所以回傳NIL
      } // else

    } // else if
    else if ( function == ">" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        float test = 1 ;
        bool set = false, fail = false ;
        Tree_Node_Ptr walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type == INT ) { // arg 是 INT
            if ( !set ) {
              test = temp->token.name_int ;
              set = true ;
            } // if
            else if ( test > temp->token.name_int ) {
              test = temp->token.name_int ;
            } // else if
            else {
              test = temp->token.name_int ;
              fail = true ;
            } // else

          } // if
          else if ( temp->token.type == FLOAT ) { // arg 是 FLOAT
            if ( !set ) {
              test = temp->token.name_float ;
              set = true ;
            } // if
            else if ( test > temp->token.name_float ) {
              test = temp->token.name_float ;
            } // else if
            else {
              test = temp->token.name_float ;
              fail = true ;
            } // else


          } // else if
          else { // 不是數字  error
            cout << "ERROR (> with incorrect argument type) : " ;
            Print_Tree( temp, 0, false ) ;
            throw m_error_message ;
          } // else
        } // while

        if ( !fail ) { // 成功 aaa
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == ">=" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        float test = 1 ;
        bool set = false, fail = false ;
        Tree_Node_Ptr walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type == INT ) { // arg 是 INT
            if ( !set ) {
              test = temp->token.name_int ;
              set = true ;
            } // if
            else if ( test >= temp->token.name_int ) {
              test = temp->token.name_int ;
            } // else if
            else {
              test = temp->token.name_int ;
              fail = true ;
            } // else

          } // if
          else if ( temp->token.type == FLOAT ) { // arg 是 FLOAT
            if ( !set ) {
              test = temp->token.name_float ;
              set = true ;
            } // if
            else if ( test >= temp->token.name_float ) {
              test = temp->token.name_float ;
            } // else if
            else {
              test = temp->token.name_float ;
              fail = true ;
            } // else

          } // else if
          else { // 不是數字  error
            cout << "ERROR (>= with incorrect argument type) : " ;
            Print_Tree( temp, 0, false ) ;
            throw m_error_message ;
          } // else
        } // while

        if ( !fail ) { // 成功 aaa
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "<" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        float test = 1 ;
        bool set = false, fail = false ;
        Tree_Node_Ptr walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type == INT ) { // arg 是 INT
            if ( !set ) {
              test = temp->token.name_int ;
              set = true ;
            } // if
            else if ( test < temp->token.name_int ) {
              test = temp->token.name_int ;
            } // else if
            else {
              test = temp->token.name_int ;
              fail = true ;
            } // else

          } // if
          else if ( temp->token.type == FLOAT ) { // arg 是 FLOAT
            if ( !set ) {
              test = temp->token.name_float ;
              set = true ;
            } // if
            else if ( test < temp->token.name_float ) {
              test = temp->token.name_float ;
            } // else if
            else {
              test = temp->token.name_float ;
              fail = true ;
            } // else

          } // else if
          else { // 不是數字  error
            cout << "ERROR (< with incorrect argument type) : " ;
            Print_Tree( temp, 0, false ) ;
            throw m_error_message ;
          } // else
        } // while

        if ( !fail ) { // 成功 aaa
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "<=" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        float test = 1 ;
        bool set = false, fail = false ;
        Tree_Node_Ptr walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type == INT ) { // arg 是 INT
            if ( !set ) {
              test = temp->token.name_int ;
              set = true ;
            } // if
            else if ( test <= temp->token.name_int ) {
              test = temp->token.name_int ;
            } // else if
            else {
              test = temp->token.name_int ;
              fail = true ;
            } // else

          } // if
          else if ( temp->token.type == FLOAT ) { // arg 是 FLOAT
            if ( !set ) {
              test = temp->token.name_float ;
              set = true ;
            } // if
            else if ( test <= temp->token.name_float ) {
              test = temp->token.name_float ;
            } // else if
            else {
              test = temp->token.name_float ;
              fail = true ;
            } // else

          } // else if
          else { // 不是數字  error
            cout << "ERROR (<= with incorrect argument type) : " ;
            Print_Tree( temp, 0, false ) ;
            throw m_error_message ;
          } // else
        } // while

        if ( !fail ) { // 成功 aaa
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "=" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        float test = 1 ;
        bool set = false, fail = false ;
        Tree_Node_Ptr walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type == INT ) { // arg 是 INT
            if ( !set ) {
              test = temp->token.name_int ;
              set = true ;
            } // if
            else if ( test == temp->token.name_int ) {
              test = temp->token.name_int ;
            } // else if
            else {
              test = temp->token.name_int ;
              fail = true ;
            } // else

          } // if
          else if ( temp->token.type == FLOAT ) { // arg 是 FLOAT
            if ( !set ) {
              test = temp->token.name_float ;
              set = true ;
            } // if
            else if ( test == temp->token.name_float ) {
              test = temp->token.name_float ;
            } // else if
            else {
              test = temp->token.name_float ;
              fail = true ;
            } // else

          } // else if
          else { // 不是數字  error
            cout << "ERROR (= with incorrect argument type) : " ;
            Print_Tree( temp, 0, false ) ;
            throw m_error_message ;
          } // else
        } // while

        if ( !fail ) { // 成功 aaa
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "string-append" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        string test = "", temp_str ;
        Tree_Node_Ptr walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type == STRING ) { // arg 是 STRING
            temp_str = temp->token.name_str ;
            temp_str.erase( temp_str.begin() ) ;
            temp_str.erase( temp_str.end()-1 ) ;
            test = test + temp_str ;
          } // if
          else { // 不是數字  error
            cout << "ERROR (string-append with incorrect argument type) : " ;
            Print_Tree( temp, 0, false ) ;
            throw m_error_message ;
          } // else
        } // while

        test = "\"" + test + "\"" ;

        temp = new Tree_Node ;
        temp->right = NULL ;
        temp->left = NULL ;
        temp->token.type = STRING ;
        temp->token.name_str = test ;
        temp->eval = true ;

        return temp ;
      } // else

    } // else if
    else if ( function == "string>?" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        string test ;
        bool set = false, fail = false ;
        Tree_Node_Ptr walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type == STRING ) { // arg 是 STRING
            if ( !set ) {
              test = temp->token.name_str ;
              set = true ;
            } // if
            else if ( test.compare( temp->token.name_str ) > 0 ) {
              test = temp->token.name_str ;
            } // else if
            else {
              test = temp->token.name_str ;
              fail = true ;
            } // else

          } // if
          else { // 不是字串  error
            cout << "ERROR (string>? with incorrect argument type) : " ;
            Print_Tree( temp, 0, false ) ;
            throw m_error_message ;
          } // else
        } // while

        if ( !fail ) { // 成功 aaa
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "string<?" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        string test ;
        bool set = false, fail = false ;
        Tree_Node_Ptr walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type == STRING ) { // arg 是 STRING
            if ( !set ) {
              test = temp->token.name_str ;
              set = true ;
            } // if
            else if ( test.compare( temp->token.name_str ) < 0 ) {
              test = temp->token.name_str ;
            } // else if
            else {
              test = temp->token.name_str ;
              fail = true ;
            } // else

          } // if
          else { // 不是字串  error
            cout << "ERROR (string<? with incorrect argument type) : " ;
            Print_Tree( temp, 0, false ) ;
            throw m_error_message ;
          } // else
        } // while

        if ( !fail ) { // 成功 aaa
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "string=?" ) {
      if ( NumOfArg( tree ) < 2 ) { // 接受的數量 >= 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        string test ;
        bool set = false, fail = false ;
        Tree_Node_Ptr walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
          if ( temp->token.type == STRING ) { // arg 是 STRING
            if ( !set ) {
              test = temp->token.name_str ;
              set = true ;
            } // if
            else if ( test.compare( temp->token.name_str ) == 0 ) {
              test = temp->token.name_str ;
            } // else if
            else {
              test = temp->token.name_str ;
              fail = true ;
            } // else

          } // if
          else { // 不是字串  error
            cout << "ERROR (string=? with incorrect argument type) : " ;
            Print_Tree( temp, 0, false ) ;
            throw m_error_message ;
          } // else
        } // while

        if ( !fail ) { // 成功 aaa
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else

    } // else if
    else if ( function == "eqv?" ) {
      if ( NumOfArg( tree ) != 2 ) { // 接受的數量 == 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        bool equal = false ;
        Tree_Node_Ptr one, two ;

        one = EVAL( tree->right->left, level+1 ) ;
        two = EVAL( tree->right->right->left, level+1 ) ;

        if ( one == two ) { // 樹 位置 相同 aaa
          equal = true ;
        } // if
        else if ( one->token.type == INT && two->token.type == INT ) {
          if ( one->token.name_int == two->token.name_int )
            equal = true ;
          else
            equal = false ;
        } // else if
        else if ( one->token.type == FLOAT && two->token.type == FLOAT ) {
          if ( one->token.name_float == two->token.name_float )
            equal = true ;
          else
            equal = false ;
        } // else if
        else if ( one->token.type == SYMBOL && two->token.type == SYMBOL ) {
          if ( one->token.name_str == two->token.name_str )
            equal = true ;
          else
            equal = false ;
        } // else if
        else if ( one->token.type == T && two->token.type == T ) {
          equal = true ;
        } // else if
        else if ( one->token.type == NIL && two->token.type == NIL ) {
          equal = true ;
        } // else if
        else
          equal = false ;

        if ( equal ) { // 兩者相同 aaa
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else
    } // else if
    else if ( function == "equal?" ) {
      if ( NumOfArg( tree ) != 2 ) { // 接受的數量 == 2
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        bool equal = false ;
        Tree_Node_Ptr one, two ;

        one = EVAL( tree->right->left, level+1 ) ;
        two = EVAL( tree->right->right->left, level+1 ) ;

        if ( one == two ) { // 樹 位置 相同 aaa
          equal = true ;
        } // if
        else if ( CheckTree( one, two ) ) { // 內容相同 aaa
          equal = true ;
        } // else if
        else
          equal = false ;

        if ( equal ) { // 兩者相同 aaa
          temp = new Tree_Node ;
          temp->token.type = T ;
          temp->token.name_str = "#t" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // if
        else {
          temp = new Tree_Node ;
          temp->token.type = NIL ;
          temp->token.name_str = "nil" ;
          temp->eval = true ;
          temp->right = NULL ;
          temp->left = NULL ;
        } // else

        return temp ;
      } // else
    } // else if
    else if ( function == "begin" ) {
      if ( NumOfArg( tree ) < 1 ) { // 接受的數量 >= 1
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        Tree_Node_Ptr walk = tree ;
        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;
          temp = EVAL( walk->left, level+1 ) ;
        } // while

        return temp ; // 回傳最後一個arg

      } // else
    } // else if
    else if ( function == "if" ) {
      Tree_Node_Ptr test ;

      if ( NumOfArg( tree ) != 2 && NumOfArg( tree ) != 3 ) { // 接受的數量 == 2 || == 3
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        test = EVAL( tree->right->left, level+1 ) ;
        if ( test->token.type != NIL ) { // 拿 True return
          temp = EVAL( tree->right->right->left, level+1 ) ;
          return temp ;
        } // if
        else { // 拿 False return
          if ( NumOfArg( tree ) == 2 ) { // error 沒有 False return
            cout << "ERROR (no return value) : " ;
            Print_Tree( tree, 0, false ) ;
            throw m_error_message ;
          } // if
          else {
            temp = EVAL( tree->right->right->right->left, level+1 ) ;
            return temp ;
          } // else

        } // else

      } // else

    } // else if
    else if ( function == "cond" ) {
      Tree_Node_Ptr test, walk ;

      if ( NumOfArg( tree ) < 1 ) { // 接受的數量 >= 1
        cout << "ERROR (COND format) : " ;
        Print_Tree( tree, 0, false ) ;
        throw m_error_message ;
      } // if
      else {
        walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;

          if ( IsPureList( walk->left ) && NumOfArg( walk->left ) >= 1 ) {

            if ( walk->left->token.type != LEFT_PAREN ) {
              cout << "ERROR (COND format) : " ;
              Print_Tree( tree, 0, false ) ;
              throw m_error_message ;
            } // if

          } // if
          else { // error
            cout << "ERROR (COND format) : " ;
            Print_Tree( tree, 0, false ) ;
            throw m_error_message ;
          } // else

        } // while

        walk = tree ;

        while ( walk->right != NULL && walk->right->token.type != NIL ) {
          walk = walk->right ;

          Tree_Node_Ptr walk2 = walk->left ;

          if ( walk->right->token.type == NIL ) { // 現在是最後一個 aaa
            if ( walk2->left->token.name_str != "else" ) {
              test = EVAL( walk2->left, level+1 ) ; // 條件判斷 aaa
              if ( test->token.type != NIL ) {
                Tree_Node_Ptr walk3 = walk2 ;
                while ( walk3->right != NULL && walk3->right->token.type != NIL ) {
                  walk3 = walk3->right ;
                  temp = EVAL( walk3->left, level+1 ) ;
                } // while

                // temp = EVAL( walk3->left, level+1 ) ;
                return temp ;
              } // if
              else { // else
                cout << "ERROR (no return value) : " ;
                Print_Tree( tree, 0, false ) ;
                throw m_error_message ;
              } // else

            } // if
            else { // 是 else
              Tree_Node_Ptr walk3 = walk2 ;
              while ( walk3->right != NULL && walk3->right->token.type != NIL ) {
                walk3 = walk3->right ;
                temp = EVAL( walk3->left, level+1 ) ;
              } // while

              // temp = EVAL( walk3->left, level+1 ) ;
              return temp ;
            } // else

          } // if
          else { // 現在不是最後一個 aaa
            test = EVAL( walk2->left, level+1 ) ; // 條件判斷 aaa

            if ( test->token.type != NIL ) {
              Tree_Node_Ptr walk3 = walk2 ;
              while ( walk3->right != NULL && walk3->right->token.type != NIL ) {
                walk3 = walk3->right ;
                temp = EVAL( walk3->left, level+1 ) ;
              } // while

              // temp = EVAL( walk3->left, level+1 ) ;
              return temp ;
            } // if

          } // else


        } // while

      } // else

    } // else if
    else if ( function == "clean-environment" ) {
      if ( NumOfArg( tree ) != 0 ) { // 接受的數量 == 0
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        m_binding.clear() ;
        throw m_no_return ;
      } // else
    } // else if
    else if ( function == "exit" ) {
      if ( NumOfArg( tree ) != 0 ) { // 接受的數量 == 0
        cout << "ERROR (incorrect number of arguments) : " << function << endl ;
        throw m_error_message ;
      } // if
      else {
        string str_exit = "exit" ;
        throw str_exit ;
      } // else
    } // else if

    return NULL ;
  } // Function_Work()

  Tree_Node_Ptr EVAL( Tree_Node_Ptr tree, int level ) {
    map<string, Tree_Node_Ptr>:: iterator it ;
    Tree_Node_Ptr temp, result ;

    if ( ATOM( tree->token ) && tree->token.type != SYMBOL ) {
      temp = new Tree_Node ;
      temp->token = tree->token ;
      temp->eval = true ;
      temp->right = NULL ;
      temp->left = NULL ;
      return temp ;
    } // if
    else if ( tree->token.type == SYMBOL ) {
      // check whether it is bound to an S-expression or an internal function
      if ( !m_binding.empty() )
        it = m_binding.find( tree->token.name_str ) ;

      if ( IsFunction( tree->token.name_str ) ) {
        temp = new Tree_Node ;
        temp->token = tree->token ;
        temp->eval = true ;
        temp->right = NULL ;
        temp->left = NULL ;
        return temp ;
      } // if
      else if ( !m_binding.empty() && it != m_binding.end() ) {
        temp = it->second ;
        // return temp ;
        return it->second ;
      } // else if
      else { // error
        cout << "ERROR (unbound symbol) : " << tree->token.name_str << endl ;
        throw m_error_message ;
      } // else

    } // else if
    else { // 是一棵樹 aaa
      if ( !IsPureList( tree ) ) { // (...) is not a pure list 最右下角不是nil
        cout << "ERROR (non-list) : " ;
        Print_Tree( tree, 0, false ) ;
        throw m_error_message ;
      } // if
      else if ( ATOM( tree->left->token ) && tree->left->token.type != SYMBOL ) {
        // first argument of (...) is an atom , which is not a symbol
        if ( tree->left->token.type == INT )
          cout << "ERROR (attempt to apply non-function) : " << tree->left->token.name_int << endl ;
        else if ( tree->left->token.type == FLOAT )
          cout << "ERROR (attempt to apply non-function) : " << tree->left->token.name_float << endl ;
        else
          cout << "ERROR (attempt to apply non-function) : " << tree->left->token.name_str << endl ;
        throw m_error_message ;
      } // else if
      else if ( tree->left->token.type == SYMBOL || tree->left->token.type == QUOTE ) {
        // first argument of (...) is a symbol SYM
        if ( IsFunction( tree->left->token.name_str ) || tree->left->token.type == QUOTE ) {
          // SYM is the name of a known function
          if ( level != 0  && tree->left->token.name_str == "clean-environment" ) { // error
            cout << "ERROR (level of CLEAN-ENVIRONMENT)" << endl ;
            throw m_error_message ;
          } // if
          else if ( level != 0  && tree->left->token.name_str == "define" ) { // error
            cout << "ERROR (level of DEFINE)" << endl ;
            throw m_error_message ;
          } // else if
          else if ( level != 0  && tree->left->token.name_str == "exit" ) { // error
            cout << "ERROR (level of EXIT)" << endl ;
            throw m_error_message ;
          } // else if
          else {
            temp = Function_Work( tree, level, tree->left->token.name_str ) ;
            return temp ;
          } // else

        } // if
        else { // SYM is 'abc', which is not the name of a known function

          if ( !m_binding.empty() )
            it = m_binding.find( tree->left->token.name_str ) ;

          if ( m_binding.empty() || it == m_binding.end() ) {
            cout << "ERROR (unbound symbol) : " << tree->left->token.name_str << endl ;
            throw m_error_message ;
          } // if
          else if ( !m_binding.empty() && it != m_binding.end() ) {
            if ( IsFunction( it->second->token.name_str ) || it->second->token.type == QUOTE ) {
              // SYM is the name of a known function
              if ( level != 0  && it->second->token.name_str == "clean-environment" ) { // error
                cout << "ERROR (level of CLEAN-ENVIRONMENT)" << endl ;
                throw m_error_message ;
              } // if
              else if ( level != 0  && it->second->token.name_str == "define" ) { // error
                cout << "ERROR (level of DEFINE)" << endl ;
                throw m_error_message ;
              } // else if
              else if ( level != 0  && it->second->token.name_str == "exit" ) { // error
                cout << "ERROR (level of EXIT)" << endl ;
                throw m_error_message ;
              } // else if
              else {
                temp = Function_Work( tree, level, it->second->token.name_str ) ;
                return temp ;
              } // else

            } // if
            else {
              cout << "ERROR (attempt to apply non-function) : " ;
              Print_Tree( it->second, 0, false ) ;
              throw m_error_message ;
            } // else

          } // else if


        } // else

      } // else if
      else { // function的位置是一棵樹 aaa
        result = EVAL( tree->left, level+1 ) ;

        if ( ATOM( result->token ) && result->token.type != SYMBOL ) {
          // first argument of (...) is an atom , which is not a symbol
          if ( result->token.type == INT )
            cout << "ERROR (attempt to apply non-function) : " << result->token.name_int << endl ;
          else if ( result->token.type == FLOAT )
            cout << "ERROR (attempt to apply non-function) : " << result->token.name_float << endl ;
          else
            cout << "ERROR (attempt to apply non-function) : " << result->token.name_str << endl ;
          throw m_error_message ;
        } // else if
        else if ( result->token.type == SYMBOL || result->token.type == QUOTE ) {
          // first argument of (...) is a symbol SYM
          if ( IsFunction( result->token.name_str ) || result->token.type == QUOTE ) {
            // SYM is the name of a known function
            if ( level != 0  && result->token.name_str == "clean-environment" ) { // error
              cout << "ERROR (level of CLEAN-ENVIRONMENT)" << endl ;
              throw m_error_message ;
            } // if
            else if ( level != 0  && result->token.name_str == "define" ) { // error
              cout << "ERROR (level of DEFINE)" << endl ;
              throw m_error_message ;
            } // else if
            else if ( level != 0  && result->token.name_str == "exit" ) { // error
              cout << "ERROR (level of EXIT)" << endl ;
              throw m_error_message ;
            } // else if
            else {
              temp = Function_Work( tree, level, result->token.name_str ) ;
              return temp ;
            } // else

          } // if
          else { // SYM is 'abc', which is not the name of a known function
            cout << "ERROR (attempt to apply non-function) : " ;
            Print_Tree( result, 0, false ) ;
            throw m_error_message ;
          } // else

        } // else if
        else { // 如果回來還是一棵樹 aaa
          cout << "ERROR (attempt to apply non-function) : " ;
          Print_Tree( result, 0, false ) ;
          throw m_error_message ;
        } // else

      } // else

    } // else

    return NULL ;
  } // EVAL()

  bool Evaluate( Tree_Node_Ptr & tree ) {

    int level = 0 ;

    try {
      tree = EVAL( tree, level ) ;
      Print_Tree( tree, 0, false ) ;
      return false ;
    } // try
    catch ( string str ) {
      if ( str == "ERROR" )
        return false ;
      else if ( str == "m_no_return" )
        return false ;
      else if ( str == "exit" )
        return true ;
    } // catch

    return false ;
  } // Evaluate()

  // =========================================================

  // ====================== 輸出 ==============================

  void Stoi( string & token ) {
    if ( token[0] == '+' )
      token.erase( token.begin() ) ;
  } // Stoi()

  void CheckString( string token ) {
    for ( int i = 0 ; i < token.length() ; i++ ) {
      if ( token[i] != '\\' )
        cout << token[i] ;
      else if ( token[i+1] != 'n' && token[i+1] != 't'
                && token[i+1] != '\\' && token[i+1] != '"' )
        cout << token[i] ;
      else {
        if ( token[i+1] == 'n' )
          cout << endl ;
        else if ( token[i+1] == 't' )
          cout << "\t" ;
        else if ( token[i+1] == '\\' )
          cout << "\\" ;
        else if ( token[i+1] == '"' )
          cout << "\"" ;
        i++ ;
      } // else
    } // for
  } // CheckString()

  void Print_Tree_R( Tree_Node_Ptr & tree, int order ) {
    bool left_P = false ;
    string temp1 = "", temp2 = "" ;

    if ( tree->token.type != NIL ) {
      for ( int i = 0 ; i < order+1 ; i++ ) {
        temp1 = temp1 + "  " ;
      } // for

      temp1 = temp1 + "." ;
    } // if

    for ( int i = 0 ; i < order+1 ; i++ ) {
      temp2 = temp2 + "  " ;
    } // for

    if ( tree->token.type == STRING ) {
      cout << temp1 << endl << temp2 ;
      CheckString( tree->token.name_str ) ;
      cout << endl ;
    } // if
    else if ( tree->token.type == SYMBOL ) {
      cout << temp1 << endl << temp2 ;
      if ( IsFunction( tree->token.name_str ) && tree->eval )
        cout << "#<procedure " << tree->token.name_str << ">" << endl ;
      else
        cout << tree->token.name_str << endl ;
    } // else if
    else if ( tree->token.type == INT ) {
      cout << temp1 << endl << temp2 ;
      cout << tree->token.name_int << endl ;

    } // else if
    else if ( tree->token.type == FLOAT ) {
      if ( tree->token.name_str != "" )
        tree->token.name_float = Stof( tree->token.name_str ) ;

      cout << temp1 << endl << temp2 ;
      cout << fixed << setprecision( 3 ) << tree->token.name_float << endl ;
    } // else if
    else if ( tree->token.type == T ) {
      cout << temp1 << endl << temp2 ;
      cout << "#t" << endl ;
    } // else if
    else if ( tree->token.type == QUOTE ) {
      cout << temp1 << endl << temp2 ;
      cout << "quote" << endl ;
    } // else if

    if ( tree->left != NULL )
      Print_Tree( tree->left, order+1, left_P ) ;
    if ( tree->right != NULL )
      Print_Tree_R( tree->right, order ) ;

  } // Print_Tree_R()

  void Print_Tree( Tree_Node_Ptr & tree, int order, bool parent_Is_Left_P ) {
    bool left_P = false ;
    bool quote = false ;

    if ( !parent_Is_Left_P ) {
      for ( int i = 0 ; i < order ; i++ ) {
        cout << "  " ;
      } // for
    } // if

    if ( tree->token.type == LEFT_PAREN && tree->token.name_str == "(" ) {
      cout << "( " ;
      left_P = true ;
    } // if
    else if ( tree->token.type == STRING ) {
      CheckString( tree->token.name_str ) ;
      cout << endl ;
    } // else if
    else if ( tree->token.type == SYMBOL ) {
      if ( IsFunction( tree->token.name_str ) && tree->eval )
        cout << "#<procedure " << tree->token.name_str << ">" << endl ;
      else
        cout << tree->token.name_str << endl ;
    } // else if
    else if ( tree->token.type == INT ) {
      cout << tree->token.name_int << endl ;
    } // else if
    else if ( tree->token.type == FLOAT ) {
      if ( tree->token.name_str != "" )
        tree->token.name_float = Stof( tree->token.name_str ) ;

      cout << fixed << setprecision( 3 ) << tree->token.name_float << endl ;
    } // else if
    else if ( tree->token.type == T )
      cout << "#t" << endl ;
    else if ( tree->token.type == NIL )
      cout << "nil" << endl ;
    else if ( tree->token.type == QUOTE ) {
      cout << "quote" << endl ;
    } // else if


    if ( tree->left != NULL )
      Print_Tree( tree->left, order+1, left_P ) ;
    if ( tree->right != NULL )
      Print_Tree_R( tree->right, order ) ;

    if ( left_P || quote ) {
      for ( int i = 0 ; i < order ; i++ ) {
        cout << "  " ;
      } // for

      cout << ")" << endl ;
    } // if

  } // Print_Tree()

  void Reset() {
    m_line = 1 ;
    m_column = 1 ;
    m_tokenPlace = 0 ;
    m_errorToken = false ;
    m_eof = false ;
    m_tokenList.clear() ;
    m_tokenList_ST.clear() ;
  } // Reset()

  void Reset( int column ) {
    m_line = 1 ;
    m_column = column+1 ;
    m_tokenPlace = 0 ;
    m_errorToken = false ;
    m_eof = false ;
    m_tokenList.clear() ;
    m_tokenList_ST.clear() ;
  } // Reset()

};


int main() {
  Token token ;
  Tree_Node_Ptr tree ;
  int testnum ;
  char test ;
  bool correct, finish = false ;
  scanf( "%d", &testnum ) ;
  scanf( "%c", &test ) ;
  cout << "Welcome to OurScheme!" << endl << endl << "> " ;
  while ( !finish ) {
    token.SkipWhite_space_char() ;
    while ( cin.peek() == ';' ) { // comment
      token.SkipComment() ;
      token.SkipWhite_space_char() ;
    } // while

    if ( cin.peek() == EOF ) {
      cout << "ERROR (no more input) : END-OF-FILE encountered" ;
      finish = true ;
    } // if
    else {
      correct = token.ReadSExp() ;
      if ( !correct ) {
        if ( token.Get_m_eof() ) {
          cout << "ERROR (no more input) : END-OF-FILE encountered" ;
          finish = true ;
        } // if
        else
          token.SkipComment() ;
      } // if
      else {
        tree = token.BuildTree() ;
        if ( token.FindExit( tree ) ) {
          finish = true ;
        } // if
        else if ( token.Clean_Environment( tree ) ) {
          cout << "environment cleaned" << endl ;
        } // else if
        else {
          // token.Print_Tree( tree, 0, false ) ;
          token.Evaluate( tree ) ;
        } // else
      } // else
    } // else

    testnum = token.SkipWhite_space_char_token() ;
    if ( cin.peek() == '\n' ) {
      scanf( "%c", &test ) ;
      token.Reset() ;
    } // if
    else {
      token.Reset( testnum ) ;
    } // else

    tree = NULL ;

    if ( !finish )
      cout << endl << "> " ;
  } // while

  cout << endl << "Thanks for using OurScheme!" ;

} // main()
