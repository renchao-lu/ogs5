/**
 * \file GridAdapter.h
 * 24/03/2010 KR Initial implementation
 *
 */


#ifndef GRIDADAPTER_H
#define GRIDADAPTER_H

// ** INCLUDES **
#include "msh_mesh.h"

namespace Mesh_Group
{
	class CFEMesh;
	class CNode;
}

/**
 * \brief Adapter class to convert FEM Mesh to a representation more suited for visualisation purposes
 */
class GridAdapter
{
public:
	/// The possible types of mesh elements.
	enum MeshType {	// values are the same as in CElem::geo_type
		LINE = 1,
		QUAD = 2,
		HEXAHEDRON = 3,
		TRIANGLE = 4,
		TETRAEDER = 5,
		PRISM = 6,
		ERROR = -1
	};

	/// An element structure consisting of a number of nodes and a MeshType
	typedef struct 
	{
		MeshType type;
		std::vector<size_t> nodes;
	} Element;


	/// Constructor using a FEM-Mesh Object as source
	GridAdapter(const Mesh_Group::CFEMesh* mesh = NULL);

	/// Constructor using a MSH-file as source
	GridAdapter(const std::string &filename);

	~GridAdapter() {};

	/// Returns the vector of nodes.
	const std::vector<GEOLIB::Point*> *getNodes() const { return _nodes; }

	/// Returns the vector of elements.
	const std::vector<Element*> *getElements() const { return _elems; }

private:
	/// Converts an FEM Mesh to a list of nodes and elements.
	int convertCFEMesh(const Mesh_Group::CFEMesh* mesh);

	/// Reads a MSH file into a list of nodes and elements.
	int readMeshFromFile(const std::string &filename);

	/// Converts a string to a MeshType
	MeshType getElementType(const std::string &t);

	/// Converts an integer to a MeshType
	MeshType getElementType(int type);

	std::vector<GEOLIB::Point*> *_nodes;
	std::vector<Element*> *_elems;

};

#endif // GRIDADAPTER_H
