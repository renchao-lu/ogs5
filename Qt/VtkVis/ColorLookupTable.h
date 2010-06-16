/**
 * \file ColorLookupTable.h
 * 23/04/2010 KR Initial implementation
 *
 */


#ifndef COLORLOOKUPTABLE_H
#define COLORLOOKUPTABLE_H

// ** INCLUDES **
#include <vector>
#include "Color.h"

/**
 * \brief Calculates and stores a colour lookup table.
 *
 * Based on a start colour and an end colour, RGB-values are interpolated and stored in vector of GEOLIB::Color. If no
 * colours are set, default values are used for start (blue) and end (red). The number of entries of the colour table can
 * be set in the constructor, the default value is 256. If additional colours are inserted into the table using setColor() 
 * the interpolation will be calculated iteratively between set colour values. Interpolation can be linear (default) or
 * exponential. Based on the set range of values, colour values can be retrieved using getColor().
 */
class ColorLookupTable
{
public:
	/// Interpolation methods
	enum LUTType {
		LINEAR = 0,
		EXPONENTIAL = 1,
		SIGMOID = 2	// not yet implemented
	};
	
	static const int DEFAULTMINVALUE = -9999;
	static const int DEFAULTMAXVALUE =  9999;

	/// Constructor
	ColorLookupTable(double rangeBegin = DEFAULTMINVALUE, double rangeEnd = DEFAULTMAXVALUE, size_t numberOfEntries = 256);

	/// Destructor
	~ColorLookupTable();

	/// Builds the colour table based on the previously set parameters. This method should only be called after all options have been set.
	void build();

	/**
	 * \brief Returns the colour for the value pos.
	 * If pos is smaller than the minimum range boundary, the first value of the lookup table is returned. Conversely, if pos is larger
	 * than the maximum range boundary, the last value of the lookup table is returned.
	 */
	GEOLIB::Color* getColor(size_t pos) const;
	
	/// Returns the colour at value pos in a char array. This is just a convenience function.
	void getColor(size_t pos, unsigned char* color) const;

	/* \brief Sets the given colour is a constant in the colour lookup table. 
	 * The colour will subsequently be considered in the interpolation process when the lookup table is built. Note that pos is only a
	 * relative position, i.e. pos in (0,1). The actual position of that colour in the table is dependent on the number of entries set
	 * in the constructor.
	 */
	void setColor(GEOLIB::Color* val, double pos);

	/// Sets the first colour in the lookup table.
	void setStartColor(GEOLIB::Color* begin);

	/// Sets the last colour in the lookup table.
	void setEndColor(GEOLIB::Color* end);

	/// Gets the minimum value for the considered range of values.
	double getMinRange() const { return _range.first;  };

	/// Gets the maximum value for the considered range of values.
	double getMaxRange() const { return _range.second; };

	/// Sets the smallest value for which colours are calculated. All values smaller than min will get the colour of the first entry.
	void setMinRange(double min);

	/// Sets the largest value for which colours are calculated. All values larger than max will get the colour of the last entry.
	void setMaxRange(double max);

	/// Sets the minimum and maximum range of values. This is just a convenience function.
	void setRange(double start, double end);

	/// Returns the type of interpolation used.
	ColorLookupTable::LUTType getInterpolationType() const { return _type; };

	/// Sets the type of interpolation.
	void setInterpolationType(ColorLookupTable::LUTType type) { _type = type; };


private:
	/// Interpolates values linearly.
	unsigned char linInterpolation(unsigned char a, unsigned char b, double p) const;

	/// Interpolates values exponentially. gamma should roughly be in [0,4), for gamma=1 interpolation is linear.
	unsigned char expInterpolation(unsigned char a, unsigned char b, double gamma, double p) const;

	std::vector<GEOLIB::Color*> _lookuptable;
	std::map<size_t, GEOLIB::Color*> _dict;
	std::pair<double, double> _range;
	LUTType _type;
};

#endif // COLORLOOKUPTABLE_H
