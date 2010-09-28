/**
 * \file VtkTrackedCamera.h
 * 25/08/2010 LB Initial implementation
 */

#ifndef VTKTRACKEDCAMERA_H
#define VTKTRACKEDCAMERA_H

#include <QObject>
#include <vtkOpenGLCamera.h>

/// @brief This camera implements view shearing for using with a head tracking
/// system.
class VtkTrackedCamera : public QObject, public vtkOpenGLCamera
{
	Q_OBJECT
	
public:
	/// @brief Constructor.
	VtkTrackedCamera(QObject* parent);
	
	/// @brief Destructor
	virtual ~VtkTrackedCamera();
	
	/// Sets the scaling from real meters to virtual meters.
	void setRealToVirtualScale(double scale) { _realToVirtualScale = scale; }

public slots:
	//void setTrackinData(double position[3], double dir[3]);
	/// @brief Sets the tracked position. The coordinate origin must be the center
	/// of the projection wall.
	// TODO Add tracking center translation vector
	void setTrackingData(double x, double y, double z);
	
	/// Sets the focal point in world coordinates. This point corresponds to the
	/// center of the projection wall.
	void setFocalPoint(double x, double y, double z);
	
	/// Move the camera by the given vector.
	void translate(double x, double y, double z);
	
	/// Rotate the camera by the given angles.
	void rotate(double yaw, double elevation, double roll);
	
	/// Updates the view.
	void updateView();
	
	/// Must be called to update the view after the camera was modified from the
	/// outside, e.g. from the vtkRenderWindowInteractor.
	void updatedFromOutside();
	
	/// @brief Sets the screen properties.
	/// @param aspectRatio width / height
	/// @param height The screen height in meter.
	void setScreenProperties(double aspectRatio, double height) {
		_screenAspectRatio = aspectRatio;
		_screenHeight = height; }

private:
	double _focalPoint[3];
	double _trackedPosition[3];
	double _realToVirtualScale;
	double _screenAspectRatio;
	double _screenHeight;
	
signals:
	/// Is emitted from updateView().
	void viewUpdated();
};

#endif // VTKTRACKEDCAMERA_H
