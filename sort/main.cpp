using namespace std;
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>

int partitonHoare(vector<int>& arr, int low, int high){
  int i = low-1;
  int j = high + 1;
  int pivot = arr[low+(low+high)/2];
  while(true){
    do {
      ++i;
    } while (arr[i] < pivot);
    do {
      --j;
    } while (arr[j] > pivot);

    if(i>=j) return j;
    swap(arr[i], arr[j]);
  }
}
int partition(vector<int>& arr, int low, int high){
  int pivot = arr[high];
  int i = low -1;
  for(int j= low; j < high; ++j){
    if(arr[j] < pivot){
      // find the first ele < pivot, swap with i, i point the last ele < pivot
      ++i;
      swap(arr[i], arr[j]);
    }
  }
  swap(arr[i + 1], arr[high]);
  return i + 1;
}
void printVector(const vector<int> & arr){
   for(auto i: arr){
    cout << i << " ";
  }
  cout << endl;
}

void quickSort(vector<int> & arr, int low, int high){
  if(low < high){
    int piviot = partition(arr, low, high);
    quickSort(arr, low, piviot-1);
    quickSort(arr, piviot+1, high);
  }
}
void quickSortH(vector<int> &arr, int low, int high) {
  if (low < high) {
    int piviot = partitonHoare(arr, low, high);
    quickSort(arr, low, piviot - 1);
    quickSort(arr, piviot + 1, high);
  }
}

void quickSortTest(){
  vector<int> arr = {10,7,8,9,1,5};
  vector<int> arr2 = {10,10,7,8,9,1,5, 9,8,8,8,0,-1};
  printVector(arr);
  quickSort(arr, 0, arr.size()-1);
  quickSortH(arr2, 0, arr2.size()-1);
  printVector(arr);
  printVector(arr2);
}

void merge(vector<int>& arr, int low, int mid, int high){
  vector<int> tmp(high-low+1);
  int i=low,j=mid+1,k=0;
  while(i <= mid && j <= high){
    if(arr[i] < arr[j]){
      tmp[k++] = arr[i++];
    }else{
      tmp[k++] = arr[j++];
    }
  }

  while(i <= mid){
    tmp[k++] = arr[i++];
  }
  while (j <= high) {
    tmp[k++] = arr[j++];
  }
  for(int p = 0; p < tmp.size(); ++p){
    // arr[p] = tmp[p];
    arr[low+p] = tmp[p];
  }
}

// better
void mergeSortIterativeDS(vector<int>& arr) {
    int n = arr.size();
    vector<int> temp(n); // 提前分配一个临时数组，避免在循环中重复分配

    // size 表示当前要合并的子数组的大小，从1开始，每次翻倍
    for (int size = 1; size < n; size *= 2) {
        // left 表示每次要合并的两个子数组的起始位置
        for (int left = 0; left < n - size; left += 2 * size) {
            int mid = left + size - 1;
            int right = min(left + 2 * size - 1, n - 1); // 防止越界

            // 合并 [left, mid] 和 [mid+1, right]
            // ... 这里使用与递归版本相同的 merge 函数逻辑，
            // 但需要稍作修改，使用预先分配的 temp 数组和正确的索引范围。
            // 下面是迭代版本的 merge 过程（内嵌）：

            int i = left;
            int j = mid + 1;
            int k = left;

            // 将数据从 arr 拷贝到 temp 的对应区间，以便合并回 arr
            for (int idx = left; idx <= right; idx++) {
                temp[idx] = arr[idx];
            }

            while (i <= mid && j <= right) {
                if (temp[i] <= temp[j]) {
                    arr[k++] = temp[i++];
                } else {
                    arr[k++] = temp[j++];
                }
            }

            while (i <= mid) {
                arr[k++] = temp[i++];
            }
            while (j <= right) {
                arr[k++] = temp[j++];
            }
        }
    }
}
void mergeBottomModifyByAI(vector<int>& arr){
  int n = arr.size();
  vector<int> tmp(n);

  for(int step = 2; step <= n*2; step *= 2){ // step = 2, 4, 8, ... up to at least n
    for(int seg = 0; seg < n; seg += step){
      int left = seg;
      int mid  = min(seg + step/2, n);
      int right = min(seg + step, n);

      int i = left, j = mid, k = left;

      while(i < mid && j < right){
        if(arr[i] < arr[j]){
          tmp[k++] = arr[i++];
        }else{
          tmp[k++] = arr[j++];
        }
      }
      while(i < mid) tmp[k++] = arr[i++];
      while(j < right) tmp[k++] = arr[j++];
    }
    arr = tmp; // Copy back (could optimize with pointer swaps)
  }
  printVector(arr);
}

void mergeBottom(vector<int>& arr){
  int n = arr.size();
  vector<int> tmp(n);
  for(int step = 2; step < n*2; step *= 2){  // imp!! step < n*2
    for(int seg = 0; seg < ceil(n / (float)step); ++seg){
      int left1 = seg*step;
      int left2 = min(left1+step/2, n-1);  // imp!! left2 must not exceed n
      int k=0;
      int s1 = left1;
      int s2 = left2;
      while(left1 < s2 && left2 < s2+step / 2 && left2 < n){
        if(arr[left1] < arr[left2]){
          tmp[s1+k] = arr[left1++];
        }else{
          tmp[s1+k] = arr[left2++];
        }
        ++k;
      }
      while(left1 < s2){
        tmp[s1+k] = arr[left1++];
        k++;
      }
      while (left2 < s2+step / 2 && left2 < n) {
        tmp[s1 + k] = arr[left2++];
        k++;
      }
    }
    arr = tmp;
  }

  printVector(arr);
}
void mergeHelper(vector<int>& arr, int low, int high){
  if(low >= high) return;
  int mid = low + (high-low)/2;
  mergeHelper(arr, low, mid);
  mergeHelper(arr,mid+1, high);
  // 合并两个已经排序的部分
  merge(arr, low, mid, high);
}
void mergeSort(vector<int>& arr){
  mergeHelper(arr, 0, arr.size()-1);
}

void mergeSortTest(){
      std::vector<int> testCases[] = {
        {64, 34, 25, 12, 22, 11, 90},
        {5, 2, 8, 1, 9},
        {1},
        {},
        {3, 3, 3, 3},
        {9, 8, 7, 6, 5, 4, 3, 2, 1}
    };
      for (auto &arr : testCases) {
    cout << "old array: ";
    printVector(arr);
    mergeSort(arr);
    cout << "new array: ";
    printVector(arr);
      }
}
void mergeSortTest2(){
      std::vector<int> testCases[] = {
        {6,5,4},
        {64, 34, 25, 12, 22, 11, 90},
        {6,5,4,1,2,3},
        {6,5,4,1,2,3,10,7},
        {64, 34, 25, 12, 22, 11},
        {64, 34, 25, 12, 22, 11, 90},
        {5, 2, 8, 1, 9},
        {1},
        {},
        {3, 3, 3, 3},
        {9, 8, 7, 6, 5, 4, 3, 2, 1}
    };
      for (auto &arr : testCases) {
    cout << "old array: ";
    printVector(arr);
    mergeBottom(arr);
      }
}

// 最小堆
void heapify(vector<int>& arr, int n, int i){
  int left = 2*i+1;
  int right = 2*i+2;
  int smallest = i;
  if(left < n && arr[left] < arr[smallest]){
    smallest = left;
  }
  if(right < n && arr[right] < arr[smallest]){
    smallest = right;
  }
  if(smallest != i){
    swap(arr[smallest], arr[i]);
    heapify(arr, n, smallest);
  }
}
// 最大堆
void heapify2(vector<int>& arr, int n, int i){
  int left = 2*i+1;
  int right = 2*i+2;
  int largest = i;
  if(left < n && arr[left] > arr[largest]) {
    largest = left;
  }
  if (right < n && arr[right] > arr[largest]) {
    largest = right;
  }
  if(largest != i){
    swap(arr[i], arr[largest]);
    heapify2(arr, n, largest);  // 递归受影响的子树, largest 位置上的值是swap后的新值
  }
}

void heapSort(vector<int>& arr){
  int n = arr.size();
  for(int i = n/2-1; i>=0; i--){
    heapify(arr, n, i);
  }

  // 逐个提取最大元素
  for(int i=n-1; i>0; i--){
    // 将堆顶元素（最大值）与当前末尾元素交换
    swap(arr[0], arr[i]);
    // 对剩余元素重新堆化
    heapify(arr, i, 0);
  }

  printVector(arr);
}

void insertionSort(vector<int> arr){
  int n = arr.size();
  for(int i=1; i < n; i++){
    int key = arr[i];
    int j = i-1;
    while(j >=0 && arr[j] > key){
      arr[j+1] = arr[j];
      j--;
    }
    arr[j+1] = key;
  }
}
// 使用二分查找找到插入位置
void insertionSortBinary(vector<int>& arr) {
    int n = arr.size();

    for (int i = 1; i < n; i++) {
        int key = arr[i];

        // 使用二分查找找到插入位置
        int left = 0, right = i - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (arr[mid] > key) {
                right = mid - 1;
            } else {
                left = mid + 1;
            }
        }

        // 移动元素
        for (int j = i - 1; j >= left; j--) {
            arr[j + 1] = arr[j];
        }

        // 插入元素
        arr[left] = key;
    }
}
void insertionSortWithSentinel(vector<int>& arr) {
    int n = arr.size();
    if (n <= 1) return;

    // 找到最小元素放到最前面作为哨兵
    int minIndex = 0;
    for (int i = 1; i < n; i++) {
        if (arr[i] < arr[minIndex]) {
            minIndex = i;
        }
    }
    swap(arr[0], arr[minIndex]);

    // 现在arr[0]是最小元素，可以作为哨兵
    for (int i = 2; i < n; i++) {
        int key = arr[i];
        int j = i - 1;

        // 不需要检查j>=0，因为arr[0]是哨兵
        while (arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

std::vector<int> testArrays[] = {
    {64, 34, 25, 12, 22, 11, 90}, {5, 2, 8, 1, 9, 3}, {1}, {}, {3, 3, 3, 3, 3},
    {9, 8, 7, 6, 5, 4, 3, 2, 1}};

void heapSortTest(){
  for(auto& arr: testArrays){
    heapSort(arr);
  }
}
int main() {
  // mergeSortTest2();
  heapSortTest();
  return 0;
}
