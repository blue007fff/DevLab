#include <iostream>
#include <random>
#include <format>
#include <iomanip>
#include <map>
#include <windows.h>
#include <locale>
#include <iostream>
#include <memory>
#include <string>
#include <new>
#include <type_traits> // std::aligned_storage 사용을 위해 필요
#include <stdio.h>
#include <stdlib.h>
#include "../../helpers.h"



void PlacementNewTest1()
{
	helpers::PrintRepeatedChar('-', 50);
	std::cout << __FUNCTION__ << std::endl;

	struct A
	{
		int i0{};
		double d0{};

		A(int _i0, double _d0) : i0{ _i0 }, d0{ _d0 } {
			std::cout << std::format("A({}, {}) constructor\n", i0, d0);
		}
		~A() { std::cout << "A destructor" << std::endl; }
		void print() { std::cout << std::format("i0 = {}, d0 = {}\n", i0, d0); }
	};

	auto pnewA = [](void* buf) {
		A* pa = new (buf) A{ 314, 3.14 };
		return pa;
	};

	helpers::PrintRepeatedChar('-', 30);
	{
		alignas(alignof(A)) char buffer[sizeof(A)];
		A* pa = pnewA(buffer);		
		pa->print();
		pa->~A();
	}

	helpers::PrintRepeatedChar('-', 30);
	{
		auto alignment = std::align_val_t(alignof(A));
		void* buffer = ::operator new(sizeof(A), alignment);
		//A* pa = pnewA(buffer0);
		//A* pa = std::construct_at<A>(reinterpret_cast<A*>(buffer0), 50, 3.1415);
		A* pa = std::construct_at(reinterpret_cast<A*>(buffer), 50, 3.1415);
		pa->print();
		std::destroy_at(pa);
		//pa->~A();
		::operator delete(buffer, alignment);
	}

	helpers::PrintRepeatedChar('-', 30);
	{
		A a{ 10, 20.0 };
		A* pa = pnewA(&a);
		pa->print();
		pa->~A();
		a.print();
	}

}

void PlacementNewAlignTest()
{
	helpers::PrintRepeatedChar('-', 50);
	std::cout << __FUNCTION__ << std::endl;

	struct alignas(16) A
	//struct alignas(32) A
	{
		int i0{};
		double d0{};

		A(int _i0, double _d0) : i0{ _i0 }, d0{ _d0 } { 
			std::cout << "A constructor" << std::endl; 
		}
		~A() { std::cout << "A destructor" << std::endl; }
		void print() { std::cout << std::format("i0 = {}, d0 = {}\n", i0, d0); }
	};

	std::cout << std::format("A[size, alignment]: [{}, {}]\n", sizeof(A), alignof(A));
	helpers::PrintRepeatedChar('-', 30);
	{
		constexpr size_t bufferSize = sizeof(A) * 2;
		constexpr size_t offset = 1;
		//alignas(alignof(A)) char buffer0[bufferSize];
		char buffer0[bufferSize];

		void* buffer1 = buffer0 + offset;
		size_t buffer1Space = bufferSize - offset;

		// alignment 를 맞추기 위해, alignof(A) 에 정렬된 주소로 변경.
		// The adjusted value of buffer0, 
		// or null pointer value if the buffer1Space provided is too small.
		void* alignedMem = std::align(alignof(A), sizeof(A), buffer1, buffer1Space);
		if (alignedMem)
		{
			A* pa = new (alignedMem) A{ 420, 3.141592 };
			pa->print();
			std::cout << std::format("{}: buffer0\n", static_cast<void*>(buffer0));
			std::cout << std::format("{}: buffer1\n", static_cast<void*>(buffer1));
			std::cout << std::format("{}: alignedMem\n", static_cast<void*>(alignedMem));
			
			// diff 값은 buffer0 의 생성 주소에 따라 달리질 수 있음.
			auto diff = reinterpret_cast<std::uintptr_t>(buffer1) - reinterpret_cast<std::uintptr_t>(buffer0);
			std::cout << std::format("from buffer0: {}\n", diff);
			pa->~A();
		}
	}

	helpers::PrintRepeatedChar('-', 30);
	{
		// align 을 맞춰서 메모리 공간 확보.
		void* buffer = ::operator new(sizeof(A), std::align_val_t(alignof(A)));
		A* pa = static_cast<A*>(buffer);
		new (pa) A{ 520, 1.14 };
		pa->print();
		pa->~A();
		::operator delete(buffer, std::align_val_t(alignof(A)));
	}
}

void PlacementNewArrayTest()
{
	helpers::PrintRepeatedChar('-', 50);
	std::cout << __FUNCTION__ << std::endl;

	struct alignas(16) A
	{
		int i0{};
		double d0{};

		A(int _i0, double _d0) : i0{ _i0 }, d0{ _d0 } {
			std::cout << "A constructor" << std::endl;
		}
		~A() { std::cout << "A destructor" << std::endl; }
		void print() { std::cout << std::format("i0 = {}, d0 = {}\n", i0, d0); }
	};

	std::cout << std::format("A[size, alignment]: [{}, {}]\n", sizeof(A), alignof(A));
	helpers::PrintRepeatedChar('-', 30);
	{
		constexpr size_t elementCount = 3;
		void* buffer = ::operator new(
			sizeof(A) * elementCount, std::align_val_t(alignof(A)));

		A* arrA = static_cast<A*>(buffer);
		for (int i = 0; i < elementCount; ++i) {
			//new (&arrA[i]) A(i, i * 10);
			std::construct_at(&arrA[i], i, i * 10.0);
		}

		for (int i = 0; i < elementCount; ++i)
			arrA[i].print();

		for (int i = 0; i < elementCount; ++i) {
			// arrA[i].~A();
			std::destroy_at(&arrA[i]);
		}

		::operator delete(buffer, std::align_val_t(alignof(A)));
	}
}


int main()
{
	PlacementNewTest1();
	PlacementNewAlignTest();
	PlacementNewArrayTest();
	return 0;
}