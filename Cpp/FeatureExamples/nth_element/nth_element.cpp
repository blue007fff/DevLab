#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <functional>
#include <string>
#include <format>

namespace
{
    auto PrintSplitLines = []() {std::cout << std::format("{:-<{}}\n", "", 50); };
}

template <typename T>
void print(const std::vector<T>& list)
{
    // for (T i : list)
    //     std::cout << i << " ";
    // std::cout << std::endl;

    std::copy(list.begin(), list.end(),
        std::ostream_iterator<T>(std::cout, ", "));
    std::cout << '\n';
}

void example_nth_element()
{
    PrintSplitLines();
	std::cout << __FUNCTION__ << std::endl;
    std::vector v0{5, 10, 6, 4, 3, 2, 6, 7, 9, 3};
    print(v0);

    std::vector<int> v = v0;
    auto print_element = [&v](size_t idx)
    {
        std::cout << "[" << idx << "]"
            << "=" << v[idx] << " : ";
    };

    size_t mididx = v.size() / 2;
    std::nth_element(v.begin(), v.begin() + mididx, v.end());
    print_element(mididx);
    print(v);

    v = v0;
    std::nth_element(v.begin(), v.begin() + 1, v.end(), std::greater<int>());
    print_element(1);
    print(v);

    v = v0;
    std::nth_element(v.begin(), v.begin(), v.end()); //, std::greater<int>());
    print_element(0);
    print(v);

    v = v0;
    std::nth_element(v.begin(), v.end() - 1, v.end(), std::greater<int>());
    print_element(v.size() - 1);
    print(v);

    v = v0;
    std::nth_element(v.begin(), v.end() - 2, v.end(), std::greater<int>());
    print_element(v.size() - 2);
    print(v);
}

int main()
{
    // 1. n번째 원소 값이 정렬되었을때 기준 값으로 바뀜.
    // - n번째 원소만 보장, 다른 원소는 보장하지 않음.
    // - 수행 후, 다음의 조건을 만족.
    // - i = [first, n) j = [n, end)
    // - *j < *i == false, comp(*j, *i) == false 를 보장.
    // 2. partition 과 비슷하다고 볼수 있는데,
    // - partition 은 값 기준, nth_element 는 인덱스 기준으로 보면 될듯.

    example_nth_element();
    return 0;
}