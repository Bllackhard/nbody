#include "nbody_space_heap.h"

nbody_space_heap::nbody_space_heap() :
	m_distance_to_node_radius_ratio(0)
{
}

void nbody_space_heap::build(size_t count, const nbcoord_t* rx, const nbcoord_t* ry, const nbcoord_t* rz,
							 const nbcoord_t* mass, nbcoord_t distance_to_node_radius_ratio)
{
	std::vector<size_t>	bodies_indites;

	bodies_indites.resize(count);
	for(size_t i = 0; i != count; ++i)
	{
		bodies_indites[i] = i;
	}
	size_t	heap_size = 2 * count - 1;
	m_mass_center.resize(heap_size);
	m_mass.resize(heap_size);
	m_radius_sqr.resize(heap_size);
	m_body_n.resize(heap_size);
	std::fill(m_body_n.begin(), m_body_n.end(), std::numeric_limits<size_t>::max());

	m_distance_to_node_radius_ratio = distance_to_node_radius_ratio;

	#pragma omp parallel
	#pragma omp single
	build(count, bodies_indites.data(), rx, ry, rz, mass, 0, 0);

	#pragma omp parallel for
	for(size_t n = 0; n < m_radius_sqr.size(); ++n)
	{
		m_radius_sqr[n] *= distance_to_node_radius_ratio;
	}
}

nbvertex_t nbody_space_heap::traverse(const nbody_data* data, const nbvertex_t& v1, const nbcoord_t mass1) const
{
	nbvertex_t			total_force;

	size_t	stack_data[MAX_STACK_SIZE] = {};
	size_t*	stack = stack_data;
	size_t*	stack_head = stack;

	*stack++ = 0;
	while(stack != stack_head)
	{
		size_t				curr = *--stack;
		const nbcoord_t		distance_sqr((v1 - m_mass_center[curr]).norm());

		if(distance_sqr > m_radius_sqr[curr])
		{
			total_force += data->force(v1, m_mass_center[curr], mass1, m_mass[curr]);
		}
		else
		{
			size_t	left(left_idx(curr));
			size_t	rght(rght_idx(curr));
			if(rght < m_body_n.size())
			{
				*stack++ = rght;
			}
			if(left < m_body_n.size())
			{
				*stack++ = left;
			}
		}
	}
	return total_force;
}

const std::vector<nbvertex_t>& nbody_space_heap::get_mass_center() const
{
	return m_mass_center;
}

const std::vector<nbcoord_t>& nbody_space_heap::get_mass() const
{
	return m_mass;
}

const std::vector<nbcoord_t>& nbody_space_heap::get_radius_sqr() const
{
	return m_radius_sqr;
}

const std::vector<size_t>& nbody_space_heap::get_body_n() const
{
	return m_body_n;
}

void nbody_space_heap::build(size_t count, size_t* indites, const nbcoord_t* rx, const nbcoord_t* ry,
							 const nbcoord_t* rz, const nbcoord_t* mass, size_t idx, size_t dimension)
{
	if(count == 1) // It is a leaf
	{
		m_mass_center[idx] = nbvertex_t(rx[*indites], ry[*indites], rz[*indites]);
		m_mass[idx] = mass[*indites];
		m_body_n[idx] = *indites;
		return;
	}

	size_t	left_size = count / 2;
	size_t	right_size = count - left_size;
	size_t*	median = indites + left_size;
	auto comparator_x = [rx](size_t a, size_t b) { return rx[a] < rx[b];};
	auto comparator_y = [ry](size_t a, size_t b) { return ry[a] < ry[b];};
	auto comparator_z = [rz](size_t a, size_t b) { return rz[a] < rz[b];};

	switch(dimension)
	{
	case DIM_NUM_X:
		std::nth_element(indites, median, indites + count, comparator_x);
		break;
	case DIM_NUM_Y:
		std::nth_element(indites, median, indites + count, comparator_y);
		break;
	case DIM_NUM_Z:
		std::nth_element(indites, median, indites + count, comparator_z);
		break;
	default:
		qDebug() << "Unexpected dimension";
		break;
	}

	size_t	next_dimension((dimension + 1) % SPACE_DIMENSIONS);
	size_t	left(left_idx(idx));
	size_t	rght(rght_idx(idx));

	if(count > NBODY_DATA_BLOCK_SIZE)
	{
		#pragma omp task
		build(left_size, indites, rx, ry, rz, mass, left, next_dimension);
		#pragma omp task
		build(right_size, median, rx, ry, rz, mass, rght, next_dimension);
		#pragma omp taskwait
	}
	else
	{
		build(left_size, indites, rx, ry, rz, mass, left, next_dimension);
		build(right_size, median, rx, ry, rz, mass, rght, next_dimension);
	}

	m_mass[idx] = m_mass[left] + m_mass[rght];
	m_mass_center[idx] = (m_mass_center[left] * m_mass[left] +
						  m_mass_center[rght] * m_mass[rght]) / m_mass[idx];
	m_radius_sqr[idx] = sqrt(m_radius_sqr[left]) + sqrt(m_radius_sqr[rght]) +
						m_mass_center[left].distance(m_mass_center[rght]);
	m_radius_sqr[idx] = m_radius_sqr[idx] * m_radius_sqr[idx];
}
