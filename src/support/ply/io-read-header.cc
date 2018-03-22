#include "io.hh"
#include <sstream>

using namespace PLY;

optional<std::pair<std::string, std::string>> split(std::string const &line, char c)
{
	using pair = std::pair<std::string, std::string>;

	size_t pos = line.find(c);

	if (pos == std::string::npos)
		return nullopt;
	else
		return optional<pair>(in_place,
			line.substr(0, pos), 
			line.substr(pos+1, line.length() - pos - 1));
}

optional<std::pair<std::string, std::string>> split_right(std::string const &line, char c)
{
	using pair = std::pair<std::string, std::string>;

	size_t pos = line.find_last_of(c);

	if (pos == std::string::npos)
		return nullopt;
	else
		return optional<pair>(in_place,
			line.substr(0, pos), 
			line.substr(pos+1, line.length() - pos - 1));
}

template <typename T>
T from_string(std::string const &s)
{
	std::istringstream ss(s);
	T value;
	ss >> value;
	return value;
}

std::istream &PLY::operator>>(std::istream &fi, Header &header)  // throw (Exception)
{
	std::string line;

	std::getline(fi, line);
	if (line != "ply")
		throw Exception("this is not a .ply file.");

	while (true)
	{
		std::getline(fi, line);
		auto S = split(line, ' ');
		if (not S)
		{
			if (line == "end_header")
				break;
			else
				throw Exception("error in header of .ply file.");
		}
	
		if (S->first == "comment")
		{
			header.comments.push_back(S->second);
			continue;
		}

		if (S->first == "format")
		{
			auto S2 = split(S->second, ' ');
			if (not S2)
				throw Exception("error in 'format' clause in header of .ply file: " + line);

			if (S2->first == "ascii")
			{
				header.format = ASCII;
				continue;
			}

			if (S2->first == "binary_little_endian")
			{
				header.format = BINARY;
				continue;
			}

			if (S2->first == "binary_big_endian")
			{
				throw Exception("sorry big-endian files are not yet supported");
			}

			throw Exception("error, could not parse format-string: " + line);
		}

		if (S->first == "element")
		{
			auto S2 = split(S->second, ' ');
			if (not S2)
				throw Exception("error in 'element' clause in header of .ply file: " + line);

			std::string name = S2->first;
			size_t N = from_string<size_t>(S2->second);

			header.emplace_back(name, RecordSpec(), N);

			continue;
		}

		if (S->first == "property")
		{
			auto S2 = split_right(S->second, ' ');
			if (not S2)
				throw Exception("error in 'property' clause in header of .ply file: " + line);

			std::string name = S2->second;
			auto S3 = split(S2->first, ' ');
			if (not S3)
			{
				if (Type_id_map.count(S2->first) == 0)
					throw Exception("unrecognized type in 'property' clause in header of .ply file: " + line);

				header.back().spec.push_back(Field(Type_id_map[S2->first], name));
			}
			else
			{
				auto S4 = split(S3->second, ' ');

				if ((not S4) or (S3->first != "list"))
					throw Exception("unrecognized type in 'property' clause in header of .ply file: " + line);

                header.back().spec.push_back(Field(
                    Type_id_map[S4->second], 
                    Type_id_map[S4->first],
                    name));
			}

			continue;
		}

		throw Exception("error in header of .ply file: " + line);
	} 

	return fi;
}

