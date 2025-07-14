#ifndef ASCENDING_SEQUENCE_H_
#define ASCENDING_SEQUENCE_H_
#include <iostream>   // std::cout
#include <vector>     // std::vector
#include <algorithm>  // std::lower_bound

class ascending_sequence {
public:
  using value_type = int;
private:
  using storage_t = std::vector<value_type>;
  storage_t seq_;
public:
  using size_type = storage_t::size_type;
  void insert (value_type x) {
    // use binary search to find insert position
    seq_.insert(std::lower_bound(seq_.begin(), seq_.end(), x), x);
  }
  [[nodiscard]] value_type operator [] (size_type idx) const noexcept {
    return seq_[idx]; }
  [[nodiscard]] size_type size () const noexcept { return seq_.size(); }
  // enable range based iteration
  [[nodiscard]] auto begin () const noexcept { return seq_.begin(); }
  [[nodiscard]] auto end ()   const noexcept { return seq_.end(); }
};

/* int main () { */
/*   ascending_sequence s;  // s.seq_:   */
/*   s.insert(7);           // s.seq_: 7 */
/*   s.insert(2);           // s.seq_: 27 */
/*   s.insert(4);           // s.seq_: 247 */
/*   s.insert(9);           // s.seq_: 2479 */
/*   s.insert(5);           // s.seq_: 24579 */
/*   std::cout << s[3];     // prints 7 */
/*   for (auto x : s) { */
/*     std::cout << x <<' ';  // 2 4 5 7 9 */
/*   } */
/*   // use type aliases */
/*   ascending_sequence::value_type x = 1; */
/*   ascending_sequence::size_type  n = 2; */
/* } */
#endif // ASCENDING_SEQUENCE_H_
