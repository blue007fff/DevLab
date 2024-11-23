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

void example_transform1()
{
    PrintSplitLines();
    std::cout << __FUNCTION__  << std::endl;

    std::vector<double> a{1.1, 2.1, 3.1, 4.1, 5.1};
    std::vector<double> b;
    std::transform(a.begin(), a.end(), std::back_inserter(b), [](double d)
        { return d * 2; });
    print(b);

    struct Ax3
    {
        double operator()(double d0) { return d0 * 3; }
    };
    b.clear();
    std::transform(a.begin(), a.end(), std::back_inserter(b), Ax3());
    print(b);

    struct AxB
    {
        double operator()(double d0, double d1) { return d0 * d1; }
    };
    auto Ax4 = std::bind(AxB(), std::placeholders::_1, 4.0);

    b.clear();
    std::transform(a.begin(), a.end(), std::back_inserter(b), Ax4);
    print(b);

    std::vector<int> vec1{1, 2, 3, 4, 5};
    std::vector<int> vec2(vec1.rbegin(), vec1.rend());
    std::vector<int> vec3(vec1.size());
    // double avg = std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
    std::transform(vec1.begin(), vec1.end(), vec2.begin(), vec3.begin(), [](int x, int y)
        { return x + y; });
    print(vec3);

    std::transform(vec1.begin(), vec1.end(), vec2.begin(), vec3.begin(), std::plus<int>());
    print(vec3);

    std::string lower = "abcde";
    std::transform(lower.begin(), lower.end(), lower.begin(), ::toupper);
    std::cout << lower << std::endl;
}

int main()
{
    example_transform1();
    return 0;
}