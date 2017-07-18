
#ifndef INDUROP_PROPERTY_H__
#define INDUROP_PROPERTY_H__

#include <map>

#include <bicomc/object.h>

#include "detail/string_view.h"

namespace irp {

	BICOMC_INTERFACE(Property)
		BICOMC_DECL_METHOD_C(get, char const*(StringView const& name), 1)
		BICOMC_DECL_METHOD(set, void(StringView const& name, StringView const& value), 2)

	public:
		std::string get(StringView const& name, StringView const& def) const;
		bool exists(StringView const& name) const;
	BICOMC_INTERFACE_END(Property)

	inline std::string Property::get(StringView const& name, StringView const& def) const
	{
		if (char const* value = get(name))
			return value;
		return def;
	}

	inline bool Property::exists(StringView const& name) const
	{
		return get(name) != nullptr;
	}

namespace detail {

	class Property : public irp::Property
	{
		BICOMC_OVERRIDE(irp::Property)
			BICOMC_OVER_METHOD_C(get, char const*(StringView const& name))
			BICOMC_OVER_METHOD(set, void(StringView const& name, StringView const& value))
		BICOMC_OVERRIDE_END()

	public:
		typedef std::map<std::string, std::string> PropertyMap;

	public:
		Property() : BICOMC_OVERRIDE_INIT() {}
		Property(Property const& rhs)
			: BICOMC_OVERRIDE_INIT(), mPropertyMap(rhs.mPropertyMap)
		{}

	public:
		Property const& operator=(Property const& rhs)
		{
			if (this == &rhs) return *this;
			mPropertyMap = rhs.mPropertyMap;
			return *this;
		}

	public:
		char const* get(StringView const& name) const
		{
			PropertyMap::const_iterator itor = mPropertyMap.find(name);
			if (itor == mPropertyMap.end())
				return nullptr;
			return itor->second.c_str();
		}

		void set(StringView const& name, StringView const& value)
		{
			mPropertyMap[name] = value;
		}

	private:
		PropertyMap mPropertyMap;
	};

} // namespace detail
} // namespace irp

#endif // !def INDUROP_PROPERTY_H__
