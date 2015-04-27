class Token {
public:
  void add(char c);
  char const *contents();
  void reset();
  Token();
  ~Token();

  static const int max_length = 100;

private:
  int pos;
  char _contents[max_length+1];
};

int parse_string(const char **_str, Token *t );
