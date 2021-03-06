#ifndef NBODY_SOLVER_TRAPEZE_H
#define NBODY_SOLVER_TRAPEZE_H

#include "nbody_solver.h"

class NBODY_DLL nbody_solver_trapeze : public nbody_solver
{
	nbody_engine::memory_array	m_f;
	nbody_engine::memory*		m_predictor;
	size_t						m_refine_steps_count;
public:
	nbody_solver_trapeze();
	~nbody_solver_trapeze();
	const char* type_name() const override;
	void set_refine_steps_count(size_t);
	void advise(nbcoord_t dt) override;
	void print_info() const override;
};

#endif // NBODY_SOLVER_TRAPEZE_H
