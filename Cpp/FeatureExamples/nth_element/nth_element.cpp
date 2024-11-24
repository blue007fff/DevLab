#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <functional>
#include "../../helpers.h"

void example_nth_element()
{
    helpers::PrintRepeatedChar('-', 50);
	std::cout << __FUNCTION__ << std::endl;
    std::vector v0{5, 10, 6, 4, 3, 2, 6, 7, 9, 3};
    helpers::PrintContainer(v0);

    std::vector<int> v = v0;
    auto print_element = [&v](size_t idx)
    {
        std::cout << "[" << idx << "]"
            << "=" << v[idx] << " : ";
    };

    size_t mididx = v.size() / 2;
    std::nth_element(v.begin(), v.begin() + mididx, v.end());
    print_element(mididx);
    helpers::PrintContainer(v);

    v = v0;
    std::nth_element(v.begin(), v.begin() + 1, v.end(), std::greater<int>());
    print_element(1);
    helpers::PrintContainer(v);

    v = v0;
    std::nth_element(v.begin(), v.begin(), v.end()); //, std::greater<int>());
    print_element(0);
    helpers::PrintContainer(v);

    v = v0;
    std::nth_element(v.begin(), v.end() - 1, v.end(), std::greater<int>());
    print_element(v.size() - 1);
    helpers::PrintContainer(v);

    v = v0;
    std::nth_element(v.begin(), v.end() - 2, v.end(), std::greater<int>());
    print_element(v.size() - 2);
    helpers::PrintContainer(v);
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