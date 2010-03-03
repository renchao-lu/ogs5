/**
 * \file Enums.h
 * 24/9/2009 LB Initial implementation
 * 
 * Definition of some enumerations used in the DataView classes.
 */
#ifndef ENUMS_H
#define ENUMS_H

// ** INCLUDES **

/**
 * Enum for viewplanes in graphics views.
 */
enum EViewPlane
{
	VIEWPLANE_XY = 0,
	VIEWPLANE_XZ,
	VIEWPLANE_YZ
};

/**
 *	Enum if an graphics item should be scaled or not
 */
enum EViewItemScaling
{
	VIEWITEMSCALING_NONE = 0,
	VIEWITEMSCALING_NORMAL
};

/// Item types. Used for the OpenSG traversal mask (which items are
/// rendered).
enum EItemType
{
	INVALID_ITEM	= 0,
	PNT_ITEM		= (1<<0),
	LINE_ITEM		= (1<<1),
	STATION_ITEM	= (1<<2),
	MSH_NODE_ITEM	= (1<<3),
	MSH_ELEM_ITEM	= (1<<4),
	MSH_ITEM		= MSH_NODE_ITEM | MSH_ELEM_ITEM,
	HELPER_ITEM		= (1<<5)
};

#endif // ENUMS_H
