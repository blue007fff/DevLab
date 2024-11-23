#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
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

    // std::copy(list.begin(), list.end(),
    //           std::ostream_iterator<T>(std::cout, ", "));
    // std::cout << '\n';

    std::for_each(list.begin(), list.end(), [](const T& v)
        { std::cout << v << ", "; });
    std::cout << '\n';
}

void example_copy()
{
    PrintSplitLines();
    std::cout << __FUNCTION__ <<  std::endl;

    std::cout << "copy array" << std::endl;
    int32_t i0[]{ 1, 2, 3, 4, 5 };
    int32_t i1[5];
    std::copy(i0, i0 + 5, i1);
    std::copy(i1, i1 + 5, std::ostream_iterator<int32_t>(std::cout, ", "));
    std::cout << '\n';

    std::cout << "copy vector" << std::endl;
    std::vector<double> a{1.1, 2.1, 3.1, 4.1, 5.1};
    std::vector<double> b;

    b.resize(a.size());
    std::copy(a.begin(), a.end(), b.begin());
    print(b);

    std::copy(a.begin(), a.end(), std::back_inserter(b));
    print(b);

    std::cout << "copy vector<double> -> vector<int>" << std::endl;
    std::vector<int> d0;
    std::copy(a.begin(), a.end(), std::back_inserter(d0));
    print(d0);
}

void example_copy_if()
{
    std::cout << __FUNCTION__ << "==========================" << std::endl;

    std::vector<double> a{1, 2, 3, 4, 5};
    std::vector<double> b;
    b.resize(5, 0);

    std::copy_if(a.begin(), a.end(), b.begin(), [](double d)
        { return d < 3.0; });
    print(b);

    std::copy_if(a.begin(), a.end(), std::back_inserter(b), [](double d)
        { return d > 3.0; });
    print(b);
}

int main()
{
    example_copy();
    example_copy_if();
    return 0;
}