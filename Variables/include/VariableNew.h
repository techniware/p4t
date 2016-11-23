#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <units/units.h>

using namespace units;
using namespace units::length;
using namespace units::mass;
using namespace units::time;
using namespace units::angle;
using namespace units::current;
using namespace units::temperature;
using namespace units::substance;
using namespace units::luminous_intensity;
using namespace units::solid_angle;
using namespace units::frequency;
using namespace units::velocity;
using namespace units::angular_velocity;
using namespace units::acceleration;
using namespace units::force;
using namespace units::pressure;
using namespace units::charge;
using namespace units::energy;
using namespace units::power;
using namespace units::voltage;
using namespace units::capacitance;
using namespace units::impedance;
using namespace units::magnetic_flux;
using namespace units::magnetic_field_strength;
using namespace units::inductance;
using namespace units::luminous_flux;
using namespace units::illuminance;
using namespace units::radiation;
using namespace units::torque;
using namespace units::area;
using namespace units::volume;
using namespace units::density;
using namespace units::concentration;
using namespace units::constants;
using namespace units::literals;
using namespace units::traits;

namespace p4t {
	class Variable {
	public:

		class Ptr : public std::shared_ptr<Variable> {
		public:
			Ptr(Variable *v) :std::shared_ptr<Variable>(v){
			}

			Ptr &operator= (const Ptr& other) {
				return *this = other;
			}
				
			template <typename ValueType>
			Ptr &operator= (const ValueType& value) {
				(*this)->set<ValueType>(value);
				return *this;
			}

		};

		typedef std::map<std::string, Ptr>		Map;

		template <size_t nbElement>
		class array : public std::array<Ptr, nbElement> {
		};

		template<typename Unit>
		Variable(const std::string& pName, Unit pValue)
			: mName(pName)
			, mValue(new GenericValue<Unit>(pValue))
		{ }
		Variable(const Variable&) = delete;
		Variable& operator=(const Variable&) = delete;

		template<typename V>
		Variable&   set(V pValue)
		{
			mValue->set(typeid(typename std::remove_reference<typename std::remove_cv<V>::type>::type).name(), &pValue);

			return *this;
		}
		std::string str() const
		{
			return mValue->str();
		}

	private:


		class Value
		{
		public:
			virtual         ~Value() { }
			virtual         std::string str() const = 0;
			virtual void    set(const std::string& pTypeName, const void* pValue) = 0;
		};

		template<typename V>
		class GenericValue : public Value {
		public:
			GenericValue(V pValue)
				: mVal(pValue) { 
			}
			
			virtual void    set(const std::string& pTypeName, const void* pValue) {
				template typename<T> convert[](const void *p) { return V(*static_cast<T*>(p)); };
				static const std::map<std::string, [](const void*)->V >  sConv =
				{
					{ typeid(bool).name(),         convert<const bool*> },
					{ typeid(char).name(),         [](const void* p) -> V { return V(*static_cast<const char*>     (p)); } },
					{ typeid(short).name(),        [](const void* p) -> V { return V(*static_cast<const short*>    (p)); } },
					{ typeid(int).name(),          [](const void* p) -> V { return V(*static_cast<const int*>      (p)); } },
					{ typeid(long long).name(),    [](const void* p) -> V { return V(*static_cast<const long long*>(p)); } },
					{ typeid(float).name(),        [](const void* p) -> V { return V(*static_cast<const float*>    (p)); } },
					{ typeid(double).name(),       [](const void* p) -> V { return V(*static_cast<const double*>   (p)); } }
					//{ typeid(std::string).name(),  [](const void* p) -> V { V lValue; std::istringstream lStream(*static_cast<const std::string*>(p)); lStream >> lValue; return lValue; } }
				};

				auto    lConv = sConv.find(pTypeName);
				if (lConv != sConv.end())
				{
					mVal = lConv->second(pValue);
				} 
			}

			template <typename T>
			void    set(const std::string& pTypeName, const T* pValue) {
				mVal = V(*static_cast<const double*>(pValue->to<double>()));
			}
			

			auto Get() {
				return mVal;
			}

			virtual std::string str() const
			{
				return (dynamic_cast<std::ostringstream&>(std::ostringstream() << mVal)).str();
			}
		private:
			virtual ~GenericValue() { }
			V   mVal;
		};

		std::string             mName;
		std::unique_ptr<Value>  mValue;
	};

	template<>
	void Variable::GenericValue<std::string>::set(const std::string& pTypeName, const void* pValue)
	{
		static const std::map<std::string, std::function<std::string(const void*)>>  sConv =
		{
			{ typeid(bool).name(),          std::function<std::string(const void*)>([](const void* p) -> std::string
											{ return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const bool*>     (p))).str(); }) },
			{ typeid(char).name(),          std::function<std::string(const void*)>([](const void* p) -> std::string
											{ return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const char*>     (p))).str(); }) },
			{ typeid(short).name(),         std::function<std::string(const void*)>([](const void* p) -> std::string
											{ return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const short*>    (p))).str(); }) },
			{ typeid(int).name(),           std::function<std::string(const void*)>([](const void* p) -> std::string
											{ return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const int*>      (p))).str(); }) },
			{ typeid(long long).name(),     std::function<std::string(const void*)>([](const void* p) -> std::string
											{ return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const long long*>(p))).str(); }) },
			{ typeid(float).name(),         std::function<std::string(const void*)>([](const void* p) -> std::string
											{ return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const float*>    (p))).str(); }) },
			{ typeid(double).name(),        std::function<std::string(const void*)>([](const void* p) -> std::string
											{ return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const double*>   (p))).str(); }) },
			{ typeid(std::string).name(),   std::function<std::string(const void*)>([](const void* p) -> std::string
											{ return *static_cast<const std::string*>(p); }) }
		};

		auto    lConv = sConv.find(pTypeName);
		if (lConv != sConv.end())
		{
			mVal = lConv->second(pValue);
		}

	}



	std::ostream& operator<<(std::ostream& pStream, Variable::Ptr pVar)
	{
		pStream << pVar->str();

		return pStream;
	}


}