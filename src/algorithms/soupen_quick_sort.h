#define INLINE inline __attribute__((always_inline))
template<typename T>
class MyCompareOperator
{
public:
  INLINE bool operator() (const T &a, const T &b) const { return a < b;}
};
template<typename T, typename CompareOperator, int64_t threshold = 20>
class SoupenSort
{
public:
  static void sort(T *data, int64_t size);
private:
  static void sort_(T *data, int64_t left, int64_t right, const CompareOperator &co);
  static void insertion_sort(T *data, int64_t left, int64_t right, const CompareOperator &co);
  static const T& get_pivot(T *data, int64_t left, int64_t right, const CompareOperator &co);
};
template<typename T, typename CompareOperator, int64_t threshold>
INLINE void SoupenSort<T, CompareOperator, threshold>::sort(T *data, int64_t size)
{
  CompareOperator co;
  sort_(data, 0, size - 1, co);
}

template<typename T, typename CompareOperator, int64_t threshold>
void SoupenSort<T, CompareOperator, threshold>::sort_(T *data, int64_t left, int64_t right, const CompareOperator &co)
{
  if(right - left > threshold) {
    const T& pivot = get_pivot(data, left, right, co);
    int64_t i = left;
    int64_t j = right - 1;
    while(true) {
      do
      {
        ++i;
      }while(co(data[i], pivot));
      do
      {
        --j;
      }while(co(pivot, data[j]));
      if (i < j) {
        std::swap(data[i], data[j]);
      } else {
        break;
      }
    }
    std::swap(data[i], data[right - 1]);//restore pivot
    sort_(data, left, i - 1, co);
    sort_(data, i + 1, right, co);
  } else {
    insertion_sort(data, left, right, co);
  }
}

template<typename T, typename CompareOperator, int64_t threshold>
INLINE void SoupenSort<T, CompareOperator, threshold>::insertion_sort(T *data, int64_t left, int64_t right, const CompareOperator &co)
{
  int64_t begin = left + 1;
  int64_t end = right + 1;
  for (int64_t i = begin; i < end; i++) {
    //insert data[i]. data[left to i-1] are ordered already
    int64_t j = i - 1;
    T tmp = data[i];
    while(j >-1 && co(tmp, data[j])) {
      data[j+1] = data[j];
      j--;
    }
    data[j+1] = tmp;
  }
}

template<typename T, typename CompareOperator, int64_t threshold>
INLINE const T& SoupenSort<T, CompareOperator, threshold>::get_pivot(T *data, int64_t left, int64_t right, const CompareOperator &co)
{
  int64_t mid = (left + right) / 2;
  if (co(data[mid], data[left])) {
    std::swap(data[mid], data[left]);
  }
  if (co(data[right], data[mid])) {
    std::swap(data[mid], data[right]);
  }
  if (co(data[mid], data[left])) {
    std::swap(data[mid], data[left]);
  }
  //Store pivot there to facilitate bound processing in sort_
  //data[right - 1] <= data[right]
  std::swap(data[mid], data[right - 1]);
  return data[right - 1];
}
