#ifndef AI_PATTERN_MANAGER
#define AI_PATTERN_MANAGER

#include <map>
#include <set>
#include <memory>
#include <cassert>

#include <SFML/Graphics.hpp>

class IContainer
{
public:
	IContainer()
	{ };
	virtual ~IContainer() = default;
};

template<typename T>
class Container : public IContainer, public std::vector<T>
{
public:
	Container()
	{ };
};

class PatternManager
{
public:
	PatternManager()
		: m_current_index(0)
		, m_size(0)
	{ }

	template<typename T>
	void registerType(const std::string& name)
	{
		const char* T_name = typeid(T).name();
		assert(m_type_name.find(T_name) == m_type_name.end() && "Type has already been registered");
		assert(m_arrays.find(name) == m_arrays.end() && "String ID has already been used");
		m_type_name.emplace(T_name, name);
		m_arrays[name] = std::make_unique<Container<T>>();
		m_index[name] = 0;
		m_indices_list[name] = std::set<unsigned int>();
	}

	template<typename T>
	void push(T object)
	{
		const char* T_name = typeid(T).name();
		assert(m_type_name.find(T_name) != m_type_name.end() && "Type hasn't been registered");

		std::string name = m_type_name[T_name];
		Container<T>* vec = static_cast<Container<T>*>(m_arrays[name].get());
		vec->push_back(object);
		m_indices_list[name].insert(m_size);
		m_size++;
	}

	std::string typeOfNext() const
	{
		assert(m_size != 0 && "Call type to empty container");
		for (auto& i : m_indices_list)
			if (i.second.count(m_current_index))
				return i.first;
	}

	template<typename T>
	const T& next()
	{
		assert(m_size && "Call next() to empty array");
		const char* T_name = typeid(T).name();
		assert(m_type_name.find(T_name) != m_type_name.end() && "Type hasn't been registered");
		std::string name = m_type_name[T_name];
		assert(name == typeOfNext() && "Mismatch type");
		Container<T>* vec = static_cast<Container<T>*>(m_arrays[name].get());

		const T& ans = (*vec)[m_index[name]];
		m_index[name] = (m_index[name] + 1) % vec->size();
		m_current_index = (m_current_index + 1) % m_size;
		return ans;
	}

	unsigned int size() const
	{
		return m_size;
	}
private:
	std::map<const char*, const std::string> m_type_name;
	std::map<const std::string, std::unique_ptr<IContainer>> m_arrays;
	std::map<const std::string, unsigned int> m_index;
	std::map<const std::string, std::set<unsigned int>> m_indices_list;
	unsigned int m_current_index;
	unsigned int m_size;
};

#endif