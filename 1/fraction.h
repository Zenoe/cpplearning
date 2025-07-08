#ifndef FRACTION_H_
#define FRACTION_H_
class Fraction {
  int numer_;
  int denom_;

  public:
  explicit constexpr Fraction(int n, int d) : numer_(n), denom_(d) {
      if(d == 0){
          throw std::invalid_argument{"denominator must not be 0"};
      }
  }
};

#endif // FRACTION_H_
