#include <iostream>
#include <random>
#include <time.h>
#include <format>
#include <iomanip>
#include <map>
#include "../../helpers.h"

void ExampleUniformDistribution()
{
    helpers::PrintRepeatedChar('-', 50);
    //std::cout << __func__ << std::endl;
    std::cout << __FUNCTION__ << std::endl;

    // 시드값을 얻기 위한 random_device 생성
    std::random_device rd;
    // 난수 생성 엔진 초기화 : rd() 는 매우 느리기 때문에, 시드값 초기화에 사용.
    std::mt19937 gen(rd());
    // 균등 분포 정의 : 0~99
    std::uniform_int_distribution<int> dis(0, 99);

    std::cout << "sizeof(std::mt19937): " << sizeof(std::mt19937) << std::endl;
    for (int i = 0; i < 5; i++)
        std::cout << "random : " << dis(gen) << std::endl;
}
void ExampleNormalDistribution()
{
    helpers::PrintRepeatedChar('-', 50);
    std::cout << __FUNCTION__ << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    // 평균, 표준편차(데이터의 흩어짐 정도)
    std::normal_distribution<double> dist(0, 1);
    //std::normal_distribution<double> dist(10, 2);
    // - stddev > 0 : std::invalid_argument
    // - generator : 균일 분포 난수 사용 : std::runtime_error

    std::map<int, int> hist;
    for (int n = 0; n < 1000; ++n)
        ++hist[std::round(dist(gen))];

    for (const auto& [number, count] : hist)
    {
        std::cout << std::setw(2) << number << ' '
            << std::string(count / 10, '*')
            << " " << count << '\n';
    }
}

int main()
{
    // throw away
    // srand(time(NULL));
    // auto value = rand() % 100;

    // https://en.cppreference.com/w/cpp/numeric/random
    ExampleUniformDistribution();

    ExampleNormalDistribution();

    return 0;
}