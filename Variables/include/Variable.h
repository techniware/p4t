#include <functional>
#include <iostream>
#include <map>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeinfo>       // operator typeid
#include <typeindex>      // std::type_index
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
				
			template <typename T>
			Ptr &operator= (const T& value) {
				(*this)->set<T>(value);
				return *this;
			}

		};

		typedef std::map<std::string, Ptr> Dictionary;

		template <size_t nbElement>
		class Array : public std::array<Ptr, nbElement> {
		};

		template<typename Unit>
		Variable(const std::string& pName, Unit pValue)
			: mName(pName)
			, mValue(new GenericValue<Unit>(pValue)){ 
		}

	
		Variable(const Variable&) = delete;
		Variable& operator=(const Variable&) = delete;

		template<typename ValueType>
		Variable&   set(const ValueType &pValue) {
			mValue->set(pValue);
			return *this;
		}


		std::string str() const
		{
			return mValue->str();
		}

	private:
		//template <typename T> T value;
		class Value {
		public:
			template <typename T>
			void set(const T& v) {
				auto &id = typeid(typename std::remove_reference<typename std::remove_cv<T>::type>::type);
				doSet(id, &v);

			};
			virtual std::string str() const = 0;

			virtual ~Value() {}
		private:
			virtual void doSet(const std::type_info &, const void *) = 0;
		};

		template <typename ValueType>
		class GenericValue : public Value {
		public:
			GenericValue(const ValueType&value):mVal(value) {
			}

			typedef ValueType (*ConversionFunction)(const void *);

			void doSet(const std::type_info &ti, const void *v) {

				static std::map <std::type_index, ConversionFunction > conversions{
					{ typeid(bool),         [](const void * v) -> ValueType { return ValueType(*static_cast<const bool*> (v)); } },
					{ typeid(char),         [](const void * v) -> ValueType { return ValueType(*static_cast<const char*> (v)); } },
					{ typeid(short),        [](const void * v) -> ValueType { return ValueType(*static_cast<const short*> (v)); } },
					{ typeid(int),          [](const void * v) -> ValueType { return ValueType(*static_cast<const int*> (v)); } },
					{ typeid(long long),    [](const void * v) -> ValueType { return ValueType(*static_cast<const long long*> (v)); } },
					{ typeid(float),        [](const void * v) -> ValueType { return ValueType(*static_cast<const float*> (v)); } },
					{ typeid(double),       [](const void * v) -> ValueType { return ValueType(*static_cast<const double*> (v)); } },
					{ typeid(std::string),  [](const void * v) -> ValueType { ValueType lValue;
																			   std::istringstream lStream(*static_cast<const std::string*>(v));
																			   lStream >> lValue;
																			   return lValue; } }
				};
				auto    lConv = conversions.find(ti);
				if (lConv != conversions.end()) {
					std::cout << "Setting from: " << ti.name() << std::endl;
					mVal = lConv->second(v);
				}
				else {
					std::cout << "Unknown  type: " << ti.name() << std::endl;
				}
			}
			virtual std::string str() const {
				return (dynamic_cast<std::ostringstream&>(std::ostringstream() << mVal)).str();
			};

			virtual ~GenericValue() { }
		private:
			ValueType mVal;
		};


		std::string             mName;
		std::unique_ptr<Value>  mValue;
	};

	template <typename type>
	std::string ConvertToString(const void *v) {
		return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const type*> (v))).str();
	};

	template<>
	void Variable::GenericValue<std::string>::doSet(const std::type_info &ti, const void *v)
	{
		typedef  std::string (*StringConversion)(const void *);


		static std::map <std::type_index, StringConversion > conversions{
			{ typeid(bool),       ConvertToString<bool>  },
			{ typeid(char),       ConvertToString<char>  },
			{ typeid(short),      ConvertToString<short> },
			{ typeid(int),        ConvertToString<int>   },
			{ typeid(long long),  ConvertToString<long long> },
			{ typeid(float),      ConvertToString<float>},
			{ typeid(double),     ConvertToString<double>},
			{ typeid(std::string),ConvertToString<std::string>} };
		auto    lConv = conversions.find(ti);
		if (lConv != conversions.end())
		{
			std::cout << "Setting from: " << ti.name() << std::endl;
			mVal = lConv->second(v);
		}
		else {
			std::cout << "Unknown  type: " << ti.name() << std::endl;
		}


	}

	template <typename type>
	std::string ConvertToUnit(const void *v) {
		return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const type*> (v))).str();
	};

	template<>
	void Variable::GenericValue< units::detail::_unit>::doSet(const std::type_info &ti, const void *v)
	{
		//unit_cast()
		std::cout << "!!!!!!! Setting from unit" << std::endl;

	};

	template<>
	std::string Variable::GenericValue< units::detail::_unit>::str() const {
	}

	/*
	// must be after the definition of the GenericValue template, 
	// because dynamic_cast requires a complete type to target
	template <typename T>
	void Variable::Value::set(const T& t) {
		// throws on bad conversion like we want
		auto & castThis = dynamic_cast<GenericValue<T>&>(*this);
		castThis.set(t);
	}
	*/
	std::ostream& operator<<(std::ostream& pStream, Variable::Ptr pVar)
	{
		pStream << pVar->str();

		return pStream;
	}


}