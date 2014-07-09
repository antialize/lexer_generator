 #ifndef REGULAR_EXPRESSION_HH_GUARD
#define REGULAR_EXPRESSION_HH_GUARD

#include <memory>
#include <iostream>
#include <limits>
#include <unordered_set>

#include "lexer_common.hh"
#include "NFA.hh"

namespace lexer {

class RegularExpression {

public:

  virtual void printType(std::ostream &os) {
    os << "error, visiting 'RegularExpression' type" << std::endl;
  }

  virtual lexer::NFA getNFA(acceptType at) const {
    return lexer::NFA(1, {{0,1}}, 0, {});    
  }
  
};

struct RegExpConcat : public RegularExpression {
  std::shared_ptr<RegularExpression> left, right;
  RegExpConcat(std::shared_ptr<RegularExpression> left, 
	       std::shared_ptr<RegularExpression> right) : left(left), right(right) {};  

  virtual void printType(std::ostream &os) {
    os << "{\"type\":\"concat\"," << std::endl;
    os << "\"left\":" << std::endl;
       
    left->printType(os);
    
    os << ",\"right\":" << std::endl;
    right->printType(os);
    os << "}" << std::endl;
    
  }

  // add lambda transition for each accept in left to initial in right

  virtual lexer::NFA getNFA(acceptType at) {
    NFA l = std::move(left->getNFA(at));
    NFA r = std::move(right->getNFA(at));
    return NFA::concat(l, r);
  }

};

struct RegExpPlus : public RegularExpression {
  std::shared_ptr<RegularExpression> inner;

  RegExpPlus(std::shared_ptr<RegularExpression> inner_) : inner(inner_) {};

  virtual void printType(std::ostream &os) {
    os << "{\"type\":\"plus\"," << std::endl << "\"inner\":";
    inner->printType(os);
    os << "}" << std::endl;
  }
 
  // add lambda transition for each accept to initial

  virtual lexer::NFA getNFA(acceptType at) {
    NFA in = std::move(inner->getNFA(at));
    return NFA::addPlus(in);
  }

};

struct RegExpStar : public RegularExpression {
  std::shared_ptr<RegularExpression> inner;
  RegExpStar(std::shared_ptr<RegularExpression> inner_) : inner(inner_) {};


  virtual void printType(std::ostream &os) {
    os << "{\"type\":\"star\"," << std::endl << "\"inner\":";
    inner->printType(os);
    os << "}" << std::endl;
  }

  // add lambda transition for each accept to initial and make initial accept

  virtual lexer::NFA getNFA(acceptType at) {
    NFA in = std::move(inner->getNFA(at));
    return NFA::addStar(in, at);
  }

};

struct RegExpOr : public RegularExpression {
  std::shared_ptr<RegularExpression> left, right;
  RegExpOr(std::shared_ptr<RegularExpression> left, 
	       std::shared_ptr<RegularExpression> right) : left(left), right(right) {};

  virtual void printType(std::ostream &os) {
    os << "{\"type\":\"or\"," << std::endl;
    os << "\"left\":" << std::endl;
       
    left->printType(os);
    
    os << ",\"right\":" << std::endl;
    right->printType(os);
    os << "}" << std::endl;
  }

  // join the two automatons.

  virtual lexer::NFA getNFA(acceptType at) {
    NFA l = std::move(left->getNFA(at));
    NFA r = std::move(right->getNFA(at));
    return NFA::join(l, r);
  }

};

struct RegExpChars : public RegularExpression {
  std::unordered_set<symbol> chars;

  RegExpChars(std::unordered_set<symbol> chars, bool invert) {
    if (invert) {
      for (symbol x = std::numeric_limits<symbol>::min();
	   x != std::numeric_limits<symbol>::max(); ++x) {
	if (!chars.count(x)) this->chars.insert(x);
      }
      if (!chars.count(std::numeric_limits<symbol>::max())) {
	this->chars.insert(std::numeric_limits<symbol>::max());
      }
    } else {
      this->chars = std::move(chars);
    }
  }

  // Make a two state machine that accepts exactly one occurrence of a character in chars.

  virtual void printType(std::ostream &os) {
    os << "{\"type\":\"chars\"," << std::endl;
    os << "\"value\":\"";
    for (auto x : chars) {
      if (x < 32 || x > 126) continue;
      if (x == '\\' || x == '"')
	os << "\\";
      os << x;
    }
    os << "\"}" << std::endl;
  }

  virtual NFA getNFA(acceptType at) {
    return NFA::simpleAccept(chars, at);
  }

};

} // end namespace lexer

#endif // end REGULAR_EXPRESSION_HH_GUARD