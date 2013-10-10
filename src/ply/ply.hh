#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <utility>
#include <memory>
#include <vector>
#include <typeinfo>

namespace Misc
{
	// PLY {{{1
	class PLY
	{
		template <typename T> using ptr = std::unique_ptr<T>;

		template <typename T>
		struct Type { static std::string name; };

		public:
			enum Format { ASCII, BINARY };

			// Header {{{2
			class Header
			{
				public:
					// Property {{{3
					class Property
					{
						std::string m_name;

						public:
							Property(std::string const &name_): m_name(name_) {}
							std::string const &name() const { return m_name; }
							virtual std::string type_expression() const = 0;
							virtual ptr<Property> copy() const = 0;
					};
					// }}}3

					// Scalar {{{3
					template <typename T>
					class Scalar: public Property
					{
						public:
							using Property::Property;

							std::string type_expression() const
							{ return Type<T>::name; }

							virtual ptr<Property> copy() const
							{
								return ptr<Property>(
									new Scalar<T>(name()));
							};
					};
					// }}}3

					// List {{{3
					template <typename T, typename length_type = uint8_t>
					class List: public Property
					{
						public:
							using Property::Property;

							std::string type_expression() const
							{ return "list " + Type<length_type>::name + " " 
								+ Type<T>::name; }

							virtual ptr<Property> copy() const
							{
								return ptr<Property>(
									new List<T, length_type>(name()));
							};
					};
					// }}}3

					// Element {{{3
					class Element
					{
						std::string 							m_name;
						size_t									m_count;
						std::vector<ptr<Property>> 	m_properties;

						public:
							Element(std::string const &name_): m_name(name_), m_count(0) {}
							std::string const &name() const { return m_name; }
							size_t count() const { return m_count; }

							void add_property(Property const &property)
							{
								m_properties.push_back(property.copy());
							}

							std::vector<ptr<Property>> const &properties() const
							{
								return m_properties;
							}

							void add_item()
							{
								++m_count;
							}
					};
					// }}}3

					typedef std::string Comment;

				private:
					mutable Format							m_format;
					std::vector<Comment> 					m_comments;
					std::vector<ptr<Element>>	m_elements;

				public:
					Format format() const 
						{ return m_format; }
					std::vector<Comment> const &comments() const 
						{ return m_comments; }
					std::vector<ptr<Element>> const &elements() const 
						{ return m_elements; }

					void set_format(Format format_) const
					{
						m_format = format_;
					}

					void add_comment(std::string const &comment)
					{
						m_comments.push_back(comment);
					}

					void add_element(std::string const &name)
					{
						m_elements.push_back(ptr<Element>(new Element(name)));
					}

					void add_property(Property const &property)
					{
						m_elements.back()->add_property(property);
					}

					void add_item()
					{
						m_elements.back()->add_item();
					}
			};
			// }}}2 PLY::Header
			typedef typename Header::Property Property;

			// Data {{{2
			class Data
			{
				public:
					// Datum {{{3
					class Datum
					{
						public:
							virtual void write_binary(std::ostream &out) const = 0;
							virtual void write_ascii(std::ostream &out) const = 0;
							virtual ptr<Datum> copy() const = 0;
					};
					// }}}3

					// DatumSeparator {{{3
					class DatumSeparator: public Datum
					{
						public:
							void write_binary(std::ostream &out) const
							{}

							void write_ascii(std::ostream &out) const
							{
								out << " ";
							}

							virtual ptr<Datum> copy() const
							{
								return ptr<Datum>(new DatumSeparator());
							}
					};
					// }}}3

					// ItemSeparator {{{3
					class ItemSeparator: public Datum
					{
						public:
							void write_binary(std::ostream &out) const
							{}

							void write_ascii(std::ostream &out) const
							{
								out << std::endl;
							}

							virtual ptr<Datum> copy() const
							{
								return ptr<Datum>(new ItemSeparator());
							}
					};
					// }}}3

					// Scalar {{{3
					template <typename T>
					class Scalar: public Datum
					{
						T	m_value;

						public:
							Scalar(T t): m_value(t) {}

							void write_binary(std::ostream &out) const
							{
								out.write(reinterpret_cast<char const *>(&m_value), sizeof(T));
							}

							void write_ascii(std::ostream &out) const
							{
								if (typeid(T) == typeid(uint8_t))
									out << int(m_value);
								else
									out << m_value;
							}

							virtual ptr<Datum> copy() const
							{
								return ptr<Datum>(new Scalar<T>(m_value));
							}
					};
					// }}}3

					// List {{{3
					template <typename T, typename length_type = uint8_t>
					class List: public Datum
					{
						std::vector<T> m_value;

						public:
							template <typename U>
							List(U const &u): m_value(u.begin(), u.end()) {}

							void write_binary(std::ostream &out) const
							{
								length_type length = m_value.size();
								out.write(reinterpret_cast<char *>(&length), sizeof(length_type));
								for (T v : m_value) 
									out.write(reinterpret_cast<char *>(&v), sizeof(T));
							}

							void write_ascii(std::ostream &out) const
							{
								out << m_value.size();
								for (T v : m_value)
									if (typeid(T) == typeid(uint8_t))
										out << " " << int(v);
									else
										out << " " << v;
							}

							virtual ptr<Datum> copy() const
							{
								return ptr<Datum>(new List<T, length_type>(m_value));
							}
					};
					// }}}3
				
				private:
					mutable Format				m_format;
					std::vector<ptr<Datum>> 	m_data;

				public:
					void set_format(Format format_) const
					{
						m_format = format_;
					}

					Format format() const
					{
						return m_format;
					}

					void put_datum(Datum const &datum) 
					{ 
						m_data.push_back(datum.copy());
					}

					void put_datum_separator() 
					{ 
						m_data.push_back(ptr<Datum>(
							new DatumSeparator()));
					}

					void put_item_separator() 
					{ 
						m_data.push_back(ptr<Datum>(
							new ItemSeparator()));
					}

					void write_binary(std::ostream &out) const
					{
						for (auto &p_datum : m_data)
							p_datum->write_binary(out);
					}

					void write_ascii(std::ostream &out) const
					{
						for (auto &p_datum : m_data)
							p_datum->write_ascii(out);
					}
			};
			// }}}2
			typedef typename Data::Datum Datum;

		private:
			Header 			m_header;
			Data			m_data;

		public:
			template <typename T>
			static Header::Scalar<T> scalar_type(std::string const &name)
			{ return Header::Scalar<T>(name); }

			template <typename T>
			static Header::List<T> list_type(std::string const &name)
			{ return Header::List<T>(name); }

			template <typename T>
			static Data::Scalar<T> scalar(T const &v)
			{ return Data::Scalar<T>(v); }

			template <typename T, typename Seq>
			static Data::List<T> list(Seq const &v)
			{ return Data::List<T>(v); }

			template <typename ...Args>
			void add_element(std::string const &name, Args &&...properties)
			{
				m_header.add_element(name);
				add_properties(std::forward<Args>(properties)...);
			}

			void add_comment(std::string const &comment)
			{
				m_header.add_comment(comment);
			}

			void put_data(Datum const &datum)
			{
				m_data.put_datum(datum);
				m_data.put_item_separator();
				m_header.add_item();
			}

			template <typename ...Args>
			void put_data(Datum const &datum, Args &&...args)
			{
				m_data.put_datum(datum);
				m_data.put_datum_separator();
				put_data(std::forward<Args>(args)...);
			}

			void write(std::string const &filename, Format format = ASCII) const;

		private:
			void add_properties(Property const &prop)
			{
				m_header.add_property(prop);
			}

			template <typename ...Args>
			void add_properties(Property const &prop, Args &&...args)
			{
				m_header.add_property(prop);
				add_properties(std::forward<Args>(args)...);
			}
	};
	// }}}1 PLY

	extern std::ostream &operator<<(std::ostream &out, PLY::Header::Property const &property);
	extern std::ostream &operator<<(std::ostream &out, PLY::Header::Element const &element);
	extern std::ostream &operator<<(std::ostream &out, PLY::Header const &header);
	extern std::ostream &operator<<(std::ostream &out, PLY::Data const &data);
}

// vim:ts=4:sw=4:fdm=marker
