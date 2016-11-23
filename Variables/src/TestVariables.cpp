#include "Variable.h"
#include <iostream>

using namespace p4t;
#include <iostream>

struct Foo {
	template < class T,
		class std::enable_if < !std::is_integral_v<T>, int >::type = 0 >
		void f(const T& value)
	{
		std::cout << "Not int" << std::endl;
	}

	template<class T,
		class std::enable_if<std::is_integral_v<T>, int>::type = 0>
		void f(const T& value)
	{
		std::cout << "Int" << std::endl;
	}
};

int main()
{
	Foo foo;
	foo.f(1);
	foo.f(1.1);


	Variable::Ptr lV1(new Variable("nom1", 2));
	Variable::Ptr lV2(new Variable("nom2", std::string("3.1415926535897932384626433832795")));
	Variable::Ptr lV3(new Variable("nom3", 3.1415926535897932384626433832795));
	Variable::Ptr lV4(new Variable("nom4", 1_mA));
	Variable::Ptr lV5 = lV4;


	std::cout << "V1 = " << lV1 << "; V2 = " << lV2 << "; V3 = " << lV3 << "; V4 = " << lV4 <<  std::endl;
	lV4 = 2_mA;
	lV4->set(2_mA);
	std::cout << "V1 = " << lV1 << "; V2 = " << lV2 << "; V3 = " << lV3 << "; V5 = " << lV5 << std::endl;

	lV1->set(3.2);
	lV2->set(2.7182818284590452353602874713527);
	lV3->set(std::string("2.7182818284590452353602874713527"));

	std::cout << "V1 = " << lV1 << "; V2 = " << lV2 << "; V3 = " << lV3 << std::endl;

	milliampere_t m = 1_mA;
	double v = m.to<double>();
	std::cout << typeid(milliampere_t::unit_type::base_unit_type).name() << std::endl;
	return 0;

}