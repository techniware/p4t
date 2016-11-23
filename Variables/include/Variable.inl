#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeinfo>

namespace p4t {
    class Variable::Impl {
    public:


        template<typename T>
        Impl(const std::string& pName, T pValue)
            : mName(pName)
            , mValue(new GenericValue<T>(pValue))
        { }

        template<typename V>
        void set(V pValue) {
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
        class GenericValue : public Value
        {
        public:
            GenericValue(V pValue)
                : mVal(pValue)
            { }
    virtual void    set(const std::string& pTypeName, const void* pValue)
            {
		template typename<T> convert[](const void *p) { return V(*static_cast<T*>(p)); };
		static const std::map<std::string, [](const void*)->V >  sConv =
		{
			{ typeid(bool).name(),         convert<const bool*> },{ typeid(char).name(),          std::function<V(const void*)>([](const void* p) -> V { return V(*static_cast<const char*>     (p)); }) },
                    { typeid(short).name(),         std::function<V (const void*)>([](const void* p) -> V { return V(*static_cast<const short*>    (p)); }) },
                    { typeid(int).name(),           std::function<V (const void*)>([](const void* p) -> V { return V(*static_cast<const int*>      (p)); }) },
                    { typeid(long long).name(),     std::function<V (const void*)>([](const void* p) -> V { return V(*static_cast<const long long*>(p)); }) },
                    { typeid(float).name(),         std::function<V (const void*)>([](const void* p) -> V { return V(*static_cast<const float*>    (p)); }) },
                    { typeid(double).name(),        std::function<V (const void*)>([](const void* p) -> V { return V(*static_cast<const double*>   (p)); }) },
                    { typeid(std::string).name(),   std::function<V (const void*)>([](const void* p) -> V { V lValue; std::istringstream    lStream(*static_cast<const std::string*>(p)); lStream >> lValue; return lValue; }) }
                };

                auto    lConv = sConv.find(pTypeName);
                if (lConv != sConv.end())
                {
                    mVal = lConv->second(pValue);
                }
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
        static const std::map<std::string, std::function<std::string (const void*)>>  sConv =
        {
            { typeid(bool).name(),          std::function<std::string (const void*)>([](const void* p) -> std::string
                                            { return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const bool*>     (p))).str(); }) },
            { typeid(char).name(),          std::function<std::string (const void*)>([](const void* p) -> std::string
                                            { return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const char*>     (p))).str(); }) },
            { typeid(short).name(),         std::function<std::string (const void*)>([](const void* p) -> std::string            
                                            { return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const short*>    (p))).str(); }) },
            { typeid(int).name(),           std::function<std::string (const void*)>([](const void* p) -> std::string
                                            { return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const int*>      (p))).str(); }) },
            { typeid(long long).name(),     std::function<std::string (const void*)>([](const void* p) -> std::string
                                            { return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const long long*>(p))).str(); }) },
            { typeid(float).name(),         std::function<std::string (const void*)>([](const void* p) -> std::string
                                            { return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const float*>    (p))).str(); }) },
            { typeid(double).name(),        std::function<std::string (const void*)>([](const void* p) -> std::string
                                            { return (dynamic_cast<std::ostringstream&>(std::ostringstream() << *static_cast<const double*>   (p))).str(); }) },
            { typeid(std::string).name(),   std::function<std::string (const void*)>([](const void* p) -> std::string
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

