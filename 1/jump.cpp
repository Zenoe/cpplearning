#include <vector>
#include <iostream>

using namespace std;

class Solution {
public:
    int jump(vector<int>& nums) {
      int jumps = 0;
      size_t i = 0;
      while(i < nums.size() -1){
        auto maxJumpAti = 0;
        int idx = 0;
        for(int k = 1; k <= nums[i]; ++k){
          if(i+k >= nums.size()-1){
            return jumps + 1;
          }
          if( (nums[i+k]) + k > maxJumpAti){
            idx = i+k;
            maxJumpAti = (nums[i+k] +k);
          }

        }
        i = idx;
        jumps +=1;
      }
      return jumps;
    }
};

int main(){
  Solution s;
  vector<int> v1{2,1,3,4};
  vector<int> v2{2,3,0,1,4};
  vector<int> v3{2,3,1,1,4};
  int res ;
  res = s.jump(v1);
  cout << res <<endl;

  res = s.jump(v2);
  cout << res <<endl;
  res = s.jump(v3);
  cout << res <<endl;
  return 0;
}
