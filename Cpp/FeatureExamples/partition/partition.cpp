#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <functional>
#include "../../helpers.h"

void example_partition()
{
    helpers::PrintRepeatedChar('-', 50);
    std::cout << __FUNCTION__ << std::endl;
    // pred 가 참인 원소들을 거짓인 원소 앞에 배치.
    // - pred 에서 당연히 v 값을 수정하면 안됨.

    std::vector<int> v = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    auto itr = std::partition(v.begin(), v.end(), [](int i)
        { return i % 2 == 0; });

    std::copy(std::begin(v), itr, std::ostream_iterator<int>(std::cout, " "));
    std::cout << " - ";
    std::copy(itr, std::end(v), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;

    std::cout.setf(std::ios_base::boolalpha);
    std::cout << std::is_partitioned(v.begin(), v.end(), [](int i)
        { return i % 2 == 0; });
}

int main()
{
    // pred 함수값이 true 인 원소들을 false 인 원소들 앞쪽에 배치.
    // 유사한 함수로, nth_element() 가 있음.
    // - partition() 은 값 기준, nth_element() 는 인덱스 기준으로 보면 될듯.

    example_partition();
    return 0;
}