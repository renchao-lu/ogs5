
#include "FEMEnums.h"

struct Constrained {
	double constrainedBCValue;
	FiniteElement::ProcessType constrainedProcessType;
	FiniteElement::PrimaryVariable constrainedPrimVar;
	ConstrainedType::type constrainedDirection;
	ConstrainedVariable::type constrainedVariable;
};

