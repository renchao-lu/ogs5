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
		INVALID = -1	// LB replaced symbol ERROR due to conflicts (with OpenSG?)
	};

	/// An element structure consisting of a number of nodes and a MeshType
	typedef struct
	{
		MeshType			type;
		size_t				material;
		std::vector<size_t> nodes;
	} Element;


	/// Constructor using a FEM-Mesh Object as source
	GridAdapter(const Mesh_Group::CFEMesh* mesh = NULL);

	/// Constructor using a MSH-file as source
	GridAdapter(const std::string &filename);

	~GridAdapter();

	/// Returns the total number of unique material IDs.
	size_t getNumberOfMaterials() const;

	/// Returns the vector of nodes.
	const std::vector<GEOLIB::Point*> *getNodes() const { return _nodes; }

	/// Returns the vector of elements.
	const std::vector<Element*> *getElements() const { return _elems; }

	/// Return a vector of elements for one material group only.
	const std::vector<Element*> *getElements(size_t matID) const;

	/// Returns the grid as a CFEMesh for use in OGS-FEM
	const CFEMesh* getCFEMesh() const;

	/// Returns the name of the mesh.
	const std::string getName() const { return _name; };

	/// Sets the name for the mesh.
	void setName(const std::string &name) { _name = name; };

private:
	/// Converts an FEM Mesh to a list of nodes and elements.
	int convertCFEMesh(const Mesh_Group::CFEMesh* mesh);

	/// Reads a MSH file into a list of nodes and elements.
	int readMeshFromFile(const std::string &filename);

	/// Converts a string to a MeshType
	MeshType getElementType(const std::string &t);

	/// Converts an integer to a MeshType
	MeshType getElementType(int type);

	/// Converts a GridAdapter into an CFEMesh.
	const CFEMesh* toCFEMesh() const;

	std::string _name;
	std::vector<GEOLIB::Point*> *_nodes;
	std::vector<Element*> *_elems;
	const CFEMesh* _mesh;
};

#endif // GRIDADAPTER_H
