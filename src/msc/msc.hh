#pragma once
#include "../base/array.hh"
#include "../base/mvector.hh"
#include "../base/mdrange.hh"
#include "../base/cvector.hh"
#include "../base/progress.hh"

#include <iostream>
#include <cstdint>
#include <vector>
#include <queue>
#include <algorithm>

namespace DMT
{
	using System::cVector;
	using System::Array;
	using System::MdRange;
	using System::mVector;

	// class MSC {{{1
	template <unsigned R>
	class MSC
	{
		struct Cell;

		cVector<R> 	single_box, double_box;
					// single_box is the natural geometry of the
					// source array, whereas double_box has twice
					// the resolution to store edges and cells
					// as well as vertices.

		std::vector<size_t> neighbours;
					// contains all 27 cells in a little cube

		std::vector<size_t> vertex_star;
					// contains all 27 cells in a little cube,
					// for the double box. centered on a vertex,
					// this will give the star of the vertex.

		Array<double>	data;
					// source data

		Array<size_t> 	V;
					// index into box.dx, in [0, 6)
					// vertices point to one of 6 edges
					// edges point to one of 4 faces
					// faces point to one of 2 cells

		enum cell_tag { UNPAIRED, SOURCE, TARGET, CRITICAL, 
			MINIMUM, TUNNEL, BRIDGE, MAXIMUM };

		Array<uint8_t> 	tag;

		std::priority_queue<Cell> Q;

		uint8_t rank(size_t i) const;
					// the amount of odd values in the index vector
					// gives the type of cell:
					//  0 - node | 1 - edge | 2 - face | 3 - cell

		double value(size_t i) const;
					// the value is determined by taking the maximum
					// of the involved vertices.

		uint8_t lower_star(size_t i, std::vector<Cell> &L);
					
		void push_minima();
					// finds the minima in the field and pushes their
					// cofacets onto the queue

		void push_cofacets(size_t i);
					// finds the cofacets of a cell, and pushes them
					// onto the queue, if they are unpaired.

		uint8_t unpaired_facets(size_t i);
		uint8_t unpaired_cofacets(size_t i);
					// counts the number of unpaired faces to a cell
					// if it is one, the cell is ready to be paired

		size_t make_pair(size_t i);
					// make a pair from a cell that has been proven
					// to have one unpaired face; the function returns
					// the index of its newly paired counterpart.

		mVector<int, R> make_vector(size_t i) const;

		System::ptr<Misc::ProgressBar> pb;

		public:
			MSC(unsigned bits, Array<double> data_);
			void generate_gradient();
			void print_critical_points(std::ostream &out) const;
	};
	// }}}1

	// implementation {{{1
	// struct MSC::Cell {{{2
	template <unsigned R>
	struct MSC<R>::Cell
	{
		size_t 		i;
		double		value;
		uint8_t		rank;

		Cell(size_t i_, double value_, uint8_t rank_):
			i(i_), value(value_), rank(rank_) {}

		// operator<, the priority_queue gives priority to
		// high values, we need to prioritize low values.
		// so I consistently use operator>
		bool operator<(Cell const &o) const
		{
			if (value == o.value) 
				return rank < o.rank;

			return (value > o.value);
		}
	};
	// }}}2

	// constructor {{{2
	template <unsigned R>
	MSC<R>::MSC(unsigned bits, Array<double> data_):
		single_box(bits), double_box(bits+1),
		data(data_), V(double_box.size()), tag(double_box.size())
	{
		// generate the 27-neighbourhood
		mVector<int, R> unit(1);
		for (mVector<int, R> const &dx : MdRange<R>(mVector<int, R>(3)))
		{
			neighbours.push_back(single_box.loc(dx) - single_box.loc(unit));
			vertex_star.push_back(double_box.loc(dx) - double_box.loc(unit));
		}

		std::fill(tag.begin(), tag.end(), UNPAIRED);
	}
	// }}}2

	// MSC::rank {{{2
	template <unsigned R>
	inline uint8_t MSC<R>::rank(size_t i) const
	{
		return double_box.count_odd(i);
	}
	// }}}2

	// MSC::value {{{2
	template <unsigned R>
	inline double MSC<R>::value(size_t i) const
	{
		size_t x = double_box.half_grid(i);

		double v = data[x];

		for (size_t dx : double_box.sq_i)
			v = std::max(v, data[double_box.add_half(i, dx)]);

		return v;
	}
	// }}}2

	// MSC::lower_star {{{2
	template <unsigned R>
	uint8_t MSC<R>::lower_star(size_t i, std::vector<Cell> &result)
	{
		uint8_t count = 0;
		double alpha = value(i);

		for (size_t n : vertex_star)
		{
			size_t x = double_box.add(i, n);
			double y = value(x);
			uint8_t r = rank(x);

			if (y <= alpha)
			{
				result.push_back(Cell(x, y, r));
				++count;
			}
		}

		return count;
	}
	// }}}2

	// MSC::push_cofacets {{{2
	template <unsigned R>
	void MSC<R>::push_cofacets(size_t i)
	{
		size_t j;
		for (unsigned k = 0; k < R; ++k)
		{
			// cofacets are the cells of dimension d+1
			// that border the ones of dimension d;
			// we are looking for the +/- 1 odd extensions
			// of even numbered indices in the double grid
			if (not (double_box.i(i, k) & 1U))
			{
				j = double_box.add(i, double_box.dx_i[k]);
				if (tag[j] == UNPAIRED)
					Q.push(Cell(j, value(j), rank(j)));

				j = double_box.sub(i, double_box.dx_i[k]);
				if (tag[j] == UNPAIRED)
					Q.push(Cell(j, value(j), rank(j)));
			}
		}
	}
	// }}}2

	// MSC::push_minima {{{2
	template <unsigned R>
	void MSC<R>::push_minima()
	{
		for (size_t i = 0; i < single_box.size(); ++i)
		{
			double v = data[i];
			size_t x = i;

			for (size_t dx : neighbours)
			{
				size_t p = single_box.add(i, dx);
				if (data[p] < v)
				{
					x = p;
					break;
				}
			}

			if (x == i)
			{
				// std::cerr << single_box.c2m(i) << ": minimal\n";
				x = single_box.double_grid(i);
				tag[x] = CRITICAL;
				if (pb) pb->tic();
				push_cofacets(x);
			}
		}
	}
	// }}}2

	// MSC::unpaired_faces {{{2
	template <unsigned R>
	uint8_t MSC<R>::unpaired_facets(size_t i)
	{
		uint8_t n = 0;
		for (unsigned k = 0; k < R; ++k)
		{
			if (double_box.i(i, k) & 1U)
			{
				if (tag[double_box.add(i, double_box.dx_i[k])] == UNPAIRED)
					++n;

				if (tag[double_box.sub(i, double_box.dx_i[k])] == UNPAIRED)
					++n;
			}
		}
		return n;
	}
	// }}}2

	// MSC::unpaired_cofacet {{{2
	template <unsigned R>
	uint8_t MSC<R>::unpaired_cofacets(size_t i)
	{
		uint8_t n = 0;
		for (unsigned k = 0; k < R; ++k)
		{
			if (not (double_box.i(i, k) & 1U))
			{
				if (tag[double_box.add(i, double_box.dx_i[k])] == UNPAIRED)
					++n;

				if (tag[double_box.sub(i, double_box.dx_i[k])] == UNPAIRED)
					++n;
			}
		}
		return n;
	}
	// }}}2

	// MSC::make_pair {{{2
	// d-cell i is being paired with its single remaining
	// unpaired coface.
	template <unsigned R>
	size_t MSC<R>::make_pair(size_t i)
	{
		size_t j = 0, q = 0;

		bool paired = false;
		for (unsigned k = 0; k < R; ++k)
		{
			if (double_box.i(i, k) & 1U)
			{
				j = double_box.add(i, double_box.dx_i[k]);
				if (tag[j] == UNPAIRED) 
				{
					if (paired)
					{
						std::cerr << "trying to pair, but multiple faces are still unpaired\n";
						throw "error!";
					}
					paired = true;
					q = j;
				}

				j = double_box.sub(i, double_box.dx_i[k]);
				if (tag[j] == UNPAIRED) 
				{
					if (paired)
					{
						std::cerr << "trying to pair, but multiple faces are still unpaired\n";
						throw "error!";
					}
					paired = true;
					q = j;
				}
			}
		}

		if (not paired)
		{
			std::cerr << "trying to pair, but no faces are unpaired\n";
			throw "error!";
		}

		V[q] = i;
		V[i] = q;
		tag[i] = TARGET; tag[q] = SOURCE;
		if (pb) { pb->tic(); pb->tic(); }

		return q;
	}
	// }}}2
	
	// MSC::generate_gradient {{{2
	template <unsigned R>
	void MSC<R>::generate_gradient()
	{
		pb.reset(new Misc::ProgressBar(double_box.size(), "finding critical points"));

		push_minima();
		size_t cnt = 0;
		while (not Q.empty())
		{
			Cell c = Q.top(); Q.pop();
			if (tag[c.i] != UNPAIRED)
				continue;

			//std::cout << double_box.c2m(c.i) << " " << cnt++ << " " << c.value << std::endl;

			uint8_t z = unpaired_facets(c.i);
			if (z == 1)
			{
				size_t j = make_pair(c.i);
				//std::cout << double_box.c2m(j) << " " << cnt++ << " " << value(j) << std::endl;
				push_cofacets(c.i);
				push_cofacets(j);
			}
			
			if (z == 0)
			{
				push_cofacets(c.i);
				tag[c.i] = CRITICAL;
				if (pb) pb->tic();
			}
		}

		if (pb) pb->finish();
		//std::cout << "\n\n";
	}
	// }}}2

	// MSC::print_critical_points {{{2
	template <unsigned R>
	mVector<int, R> MSC<R>::make_vector(size_t i) const
	{
		auto v = double_box.dvec(double_box.sub(V[i], i));
		int N = double_box.extent();
		for (unsigned k = 0; k < R; ++k)
			if (v[k] > N/2)
				v[k] -= N;
		return v;
	}

	template <unsigned R>
	void MSC<R>::print_critical_points(std::ostream &out) const
	{
	/*	for (size_t x = 0; x < single_box.size(); ++x)
		{
			if (single_box.i(x, 0) == 0) out << std::endl;
			out << data[x] << " ";
		}

		out << "\n\n\n";

		for (size_t x = 0; x < double_box.size(); ++x)
		{
			if ((tag[x] == SOURCE) and (V[x] <= double_box.size()))
				out << double_box.c2m(x) << " " << make_vector(x) << "\n";
		}

		out << "\n\n\n";
		*/

		for (size_t x = 0; x < double_box.size(); ++x)
		{
			if (tag[x] == CRITICAL)
				out << double_box.dvec(x) << " " << int(rank(x)) << std::endl;
		}
	}
	// }}}2
	// }}}1
}

// vim:sw=4:ts=4:fdm=marker
