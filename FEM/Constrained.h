
#ifndef CONSTRAINED_H_
#define CONSTRAINED_H_

#include "FEMEnums.h"

struct Constrained {
	double constrainedValue=0.0;
	FiniteElement::ProcessType constrainedProcessType = FiniteElement::INVALID_PROCESS;
	FiniteElement::PrimaryVariable constrainedPrimVar = FiniteElement::INVALID_PV;
	ConstrainedType::type constrainedDirection = ConstrainedType::INVALID_CONSTRAINED_TYPE;
	ConstrainedVariable::type constrainedVariable = ConstrainedVariable::INVALID_CONSTRAINED_VARIABLE;
	bool _isCompleteConstrained = false;
	bool _completeConstrainedStateOff = false;
	std::vector<bool>_constrainedNodes;
};

#endif
