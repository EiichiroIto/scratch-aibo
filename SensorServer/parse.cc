#include "parse.h"

Token::Token()
{
  reset();
}

Token::~Token()
{
}

void
Token::add(char c)
{
  if ( pos >= max_length ) {
    return;
  }
  _contents[pos++] = c;
  _contents[pos]='\0';
}

char const *
Token::contents()
{
  return _contents;
}

void
Token::reset()
{
  pos = 0;
  _contents[pos]='\0';
}

enum ParseState {
  normal, quoted1, quoted2, quoted3
};

int
parse_string(const char **_str, Token *t )
{
  const char *str = *_str;
  char c;
  int skip_spaces = 1;
  ParseState state = normal;

  t->reset();
  while ( (c = *str++) != 0 ) {
    switch ( state ) {
    case normal:
      if ( c == '\"' ) {
	skip_spaces = 0;
	state = quoted1;
      } else if ( c > ' ' ) {
	skip_spaces = 0;
	t->add(c);
      } else if ( !skip_spaces ) {
	*_str = str;
	return 1;
      }
      break;
    case quoted1:
      t->add(c);
      state = (c == '\"') ? normal : quoted2;
      break;
    case quoted2:
      if ( c == '\"' ) {
	state = quoted3;
      } else {
	t->add(c);
      }
      break;
    case quoted3:
      if ( c == '\"' ) {
	t->add(c);
	state = quoted2;
      } else {
	*_str = str - 1;
	return 1;
      }
      break;
    }
  }
  return 0;
}

#ifdef MAIN
#include <iostream>

int
main()
{
  Token t1;
  char *test1 = "abc";
  char *test2 = "\"abc def\" ghi";
  char *test3 = "\"abc def ghi";
  char *test4 = "abc\"\"def ghi";
  char *test5 = "\"abc\"\" \"\"def\" ghi";
  char *test6 = "\"abc def\"ghi";
  char *p;
  int ret;

  p = test1;
  ret = parse_string(&p, &t1);
  std::cout << ret;
  std::cout << t1.contents();
  std::cout << "|\n";

  p = test2;
  do {
    ret = parse_string(&p, &t1);
    std::cout << ret;
    std::cout << t1.contents();
    std::cout << "|\n";
  } while ( ret );

  p = test3;
  do {
    ret = parse_string(&p, &t1);
    std::cout << ret;
    std::cout << t1.contents();
    std::cout << "|\n";
  } while ( ret );

  p = test4;
  do {
    ret = parse_string(&p, &t1);
    std::cout << ret;
    std::cout << t1.contents();
    std::cout << "|\n";
  } while ( ret );

  p = test5;
  do {
    ret = parse_string(&p, &t1);
    std::cout << ret;
    std::cout << t1.contents();
    std::cout << "|\n";
  } while ( ret );

  p = test6;
  do {
    ret = parse_string(&p, &t1);
    std::cout << ret;
    std::cout << t1.contents();
    std::cout << "|\n";
  } while ( ret );

  return 0;
}
#endif /* MAIN */
