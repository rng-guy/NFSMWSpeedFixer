#pragma once

#include <map>
#include <ranges>
#include <vector>
#include <string>
#include <concepts>
#include <charconv>
#include <optional>
#include <iostream>
#include <algorithm>
#include <string_view>
#include <system_error>



namespace BasicParser
{

	// Concepts -------------------------------------------------------------------------------------------------------------------------------------

	template <typename T>
	concept IsStringOrView = (std::same_as<T, std::string> or std::same_as<T, std::string_view>);

	template <typename T>
	concept IsStringType = (IsStringOrView<T> or std::same_as<T, const char*>);





	// Trimming functions ---------------------------------------------------------------------------------------------------------------------------

	bool IsNotSpace(const unsigned char ch)
	{
		return (not std::isspace(ch));
	}



	std::string& TrimLeft(std::string& string)
	{
		string.erase(string.begin(), std::find_if(string.begin(), string.end(), IsNotSpace));

		return string;
	}


	std::string_view TrimLeft(std::string_view view)
	{
		view.remove_prefix(std::distance(view.begin(), std::ranges::find_if(view, IsNotSpace)));

		return view;
	}



	std::string& TrimRight(std::string& string)
	{
		string.erase(std::find_if(string.rbegin(), string.rend(), IsNotSpace).base(), string.end());

		return string;
	}


	std::string_view TrimRight(std::string_view view)
	{
		const auto reversed = view | std::views::reverse;
		view.remove_suffix(std::distance(reversed.begin(), std::ranges::find_if(reversed, IsNotSpace)));

		return view;
	}



	std::string& Trim(std::string& string)
	{
		return TrimRight(TrimLeft(string));
	}


	std::string_view Trim(const std::string_view view)
	{
		return TrimRight(TrimLeft(view));
	}





	// Extraction functions -------------------------------------------------------------------------------------------------------------------------

	template <typename T>
	requires std::is_arithmetic_v<T>
	bool ExtractFromView
	(
		const std::string_view view, 
		T&                     value
	) {
		if (view.empty()) return false;

		T result{};

		const auto viewEnd = view.data() + view.size();

		const auto [readEnd, error] = std::from_chars(view.data(), viewEnd, result);
		if ((error != std::errc{}) or (readEnd != viewEnd)) return false;

		value = result;

		return true;
	}



	template<>
	bool ExtractFromView<bool>
	(
		const std::string_view view,
		bool&                  value
	) {
		if (view == "true")
		{
			value = true;
			return true;
		}

		if (view == "false")
		{
			value = false;
			return true;
		}

		return false;
	}



	template <typename T>
	requires IsStringOrView<T>
	bool ExtractFromView
	(
		const std::string_view view,
		T&                     value
	) {
		value = view;
		return true;
	}





	// File parser ----------------------------------------------------------------------------------------------------------------------------------

	class Parser
	{
	private:

		static bool PreProcessLine(std::string& line) 
		{
			TrimLeft(line);

			// Remove (trailing) comment if there is one
			constexpr auto IsComment = [](const char ch) {return (ch == ';');};
			line.erase(std::find_if(line.begin(), line.end(), IsComment), line.end());

			TrimRight(line);

			return (not line.empty());
		}


		static std::optional<std::string_view> ExtractSectionName(const std::string_view line)
		{
			if (not line.starts_with('[')) return std::nullopt;
			if (not line.ends_with  (']')) return std::nullopt;

			return line.substr(1, line.length() - 2);
		}


		static std::optional<size_t> FindUniqueAssign(const std::string_view line)
		{
			constexpr char assign = '=';
			
			const size_t leftAssign = line.find(assign);

			if (leftAssign == std::string_view::npos) return std::nullopt;
			if (leftAssign != line.rfind(assign))     return std::nullopt;

			return leftAssign;
		}



	public:

		using Section  = std::map<std::string, std::string, std::less<>>;
		using Sections = std::map<std::string, Section,     std::less<>>;


		Sections sections;


		void ParseStream(std::istream& stream) 
		{
			std::string line;

			Section* currentSection = nullptr;

			while (std::getline(stream, line))
			{
				if (not this->PreProcessLine(line)) continue;

				// Check for new section
				if (const auto section = this->ExtractSectionName(line))
				{
					currentSection = &(this->sections[std::string(*section)]);

					continue;
				}
				else if (not currentSection) continue;

				// Parse key-value pair
				if (const auto foundAssign = this->FindUniqueAssign(line))
				{
					const std::string_view key = TrimRight({line.data(), *foundAssign});
					if (key.empty()) continue;

					const std::string_view value = TrimLeft({line.data() + (*foundAssign + 1), line.size() - (*foundAssign + 1)});
					if (value.empty()) continue;

					currentSection->try_emplace(std::string(key), std::string(value));
				}
			}
		}


		explicit Parser() = default;

		Parser(std::istream& fileStream)
		{
			this->ParseStream(fileStream);
		}


		template <typename T>
		static bool ExtractFromSection
		(
			const Section&         section,
			const std::string_view key,
			T&                     value
		) {
			const auto foundKey = section.find(key);
			if (foundKey == section.end()) return false;

			return ExtractFromView<T>(foundKey->second, value);
		}


		template <>
		static bool ExtractFromSection<const char*>
		(
			const Section&         section,
			const std::string_view key,
			const char*&           value
		) {
			const auto foundKey = section.find(key);
			if (foundKey == section.end()) return false;

			value = (foundKey->second).c_str();

			return true;
		}


		template <typename T>
		bool ExtractFromSection
		(
			const std::string_view section,
			const std::string_view key,
			T&                     value
		)
			const
		{
			const auto foundSection = this->sections.find(section);
			if (foundSection == this->sections.end()) return false;

			return this->ExtractFromSection<T>(foundSection->second, key, value);
		}
		 

		template <typename T, typename K>
		requires IsStringType<K>
		static size_t ExtractSection
		(
			const Section&  section,
			std::vector<K>& keys,
			std::vector<T>& values
		) {
			T result{};

			size_t numReads = 0;

			keys  .reserve(keys.size()   + section.size());
			values.reserve(values.size() + section.size());

			for (auto const& [key, value] : section)
			{
				bool isRead;

				// Check special case (C-style literal)
				if constexpr (std::same_as<T, const char*>)
				{
					result = value.c_str();
					isRead = true;
				}
				else isRead = ExtractFromView<T>(value, result);

				if (isRead)
				{
					++numReads;

					// Check special case (C-style literal)
					if constexpr (std::same_as<K, const char*>)
						keys.push_back(key.c_str());

					else
						keys.emplace_back(key);

					values.push_back(std::move(result));
				}
			}

			return numReads;
		}


		template <typename T, typename K>
		requires IsStringType<K>
		size_t ExtractSection
		(
			const std::string_view section,
			std::vector<K>&        keys,
			std::vector<T>&        values
		)
			const
		{
			const auto foundSection = this->sections.find(section);
			if (foundSection == this->sections.end()) return 0;

			return this->ExtractSection<T, K>(foundSection->second, keys, values);
		}


		void Clear() 
		{
			sections.clear();
		}
	};
}