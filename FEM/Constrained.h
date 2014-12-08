
#ifndef CONSTRAINED_H_
#define CONSTRAINED_H_

#include "FEMEnums.h"

struct Constrained {
	double constrainedValue;
	FiniteElement::ProcessType constrainedProcessType;
	FiniteElement::PrimaryVariable constrainedPrimVar;
	ConstrainedType::type constrainedDirection;
	ConstrainedVariable::type constrainedVariable;
	bool _isCompleteConstrained;
	bool _completeConstrainedStateOff;
	std::vector<bool>_constrainedNodes;
};

#endif
