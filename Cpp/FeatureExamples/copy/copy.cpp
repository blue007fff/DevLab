#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include "../../helpers.h"

void example_copy()
{
    helpers::PrintRepeatedChar('-', 50);
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
    helpers::PrintContainer(b);

    std::copy(a.begin(), a.end(), std::back_inserter(b));
    helpers::PrintContainer(b);

    std::cout << "copy vector<double> -> vector<int>" << std::endl;
    std::vector<int> d0;
    std::copy(a.begin(), a.end(), std::back_inserter(d0));
    helpers::PrintContainer(d0);
}

void example_copy_if()
{
    helpers::PrintRepeatedChar('-', 50);
    std::cout << __FUNCTION__ << std::endl;

    std::vector<double> a{1, 2, 3, 4, 5};
    std::vector<double> b;
    b.resize(5, 0);

    std::copy_if(a.begin(), a.end(), b.begin(), [](double d)
        { return d < 3.0; });
    helpers::PrintContainer(b);

    std::copy_if(a.begin(), a.end(), std::back_inserter(b), [](double d)
        { return d > 3.0; });
    helpers::PrintContainer(b);
}

int main()
{
    example_copy();
    example_copy_if();
    return 0;
}