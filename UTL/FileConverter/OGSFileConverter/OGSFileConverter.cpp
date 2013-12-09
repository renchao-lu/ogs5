/**
 * \file OGSFileConverter.cpp
 * 2012/04/04 KR Initial implementation
 */

#include "OGSFileConverter.h"
#include "FileListDialog.h"
#include "ConversionTools.h"
#include "OGSError.h"

#include <QFileInfo>

// conversion includes
#include "ProjectData.h"
#include "StringTools.h"

// geometry
#include "GEOObjects.h"
#include "OGSIOVer4.h"
#include "XmlIO/XmlGmlInterface.h"

// mesh
#include "GridAdapter.h"
#include "VtkMeshSource.h"
#include "VtkMeshConverter.h"
#include <vtkDataObject.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>

// old condition objects
#include "XmlIO/XmlCndInterface.h"
#include "BoundaryCondition.h"
#include "InitialCondition.h"
#include "SourceTerm.h"
#include "rf_bc_new.h"
#include "rf_ic_new.h"
#include "rf_st_new.h"

OGSFileConverter::OGSFileConverter(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
}

OGSFileConverter::~OGSFileConverter()
{
}

void OGSFileConverter::convertGML2GLI(const QStringList &input, const QString &output)
{
	ProjectData project;
	GEOLIB::GEOObjects* geo_objects = new GEOLIB::GEOObjects;
	project.setGEOObjects(geo_objects);

	FileFinder fileFinder = createFileFinder();
	std::string schemaName(fileFinder.getPath("OpenGeoSysGLI.xsd"));
	FileIO::XmlGmlInterface xml(&project, schemaName);

	for (QStringList::const_iterator it=input.begin(); it!=input.end(); ++it)
	{
		const QFileInfo fi(*it);
		const std::string file_name = fi.baseName().toStdString();
		const std::string output_str = QString(output + "/" + fi.completeBaseName() + ".gli").toStdString();

		if (fileExists(output_str))
		{
			xml.readFile(*it);
			std::vector<std::string> geo_names;
			geo_objects->getGeometryNames(geo_names);
			FileIO::writeGLIFileV4(output_str, geo_names[0], *geo_objects);
			geo_objects->removeSurfaceVec(geo_names[0]);
			geo_objects->removePolylineVec(geo_names[0]);
			geo_objects->removePointVec(geo_names[0]);
		}
	}

	//FileIO::writeAllDataToGLIFileV4(output.toStdString(), *geo_objects);
	OGSError::box("File conversion finished");
}

void OGSFileConverter::convertGLI2GML(const QStringList &input, const QString &output)
{
	ProjectData project;
	GEOLIB::GEOObjects* geo_objects = new GEOLIB::GEOObjects;
	project.setGEOObjects(geo_objects);
	FileFinder fileFinder = createFileFinder();
	std::string schemaName(fileFinder.getPath("OpenGeoSysGLI.xsd"));
	FileIO::XmlGmlInterface xml(&project, schemaName);

	for (QStringList::const_iterator it=input.begin(); it!=input.end(); ++it)
	{
		const QFileInfo fi(*it);
		const std::string output_str = QString(output + "/" + fi.completeBaseName() + ".gml").toStdString();
		const std::string geo_name = BaseLib::getFileNameFromPath(it->toStdString(), true);

		if (fileExists(output_str))
		{
			std::string unique_name;
			std::vector<std::string> errors;

			if (FileIO::readGLIFileV4(it->toStdString(), geo_objects, unique_name, errors)) 
			{
				if (errors.empty())
				{
					xml.setNameForExport(geo_name);
					xml.writeToFile(output_str);
					geo_objects->removeSurfaceVec(geo_name);
					geo_objects->removePolylineVec(geo_name);
					geo_objects->removePointVec(geo_name);
				}
				else
					for (size_t k(0); k<errors.size(); k++)
						OGSError::box(QString::fromStdString(errors[k]));

			}
		}
	}
	
	OGSError::box("File conversion finished");
}

void OGSFileConverter::convertVTU2MSH(const QStringList &input, const QString &output)
{
	for (QStringList::const_iterator it=input.begin(); it!=input.end(); ++it)
	{
		const QFileInfo fi(*it);
		const std::string msh_name = fi.fileName().toStdString();
		const std::string output_str = QString(output + "/" + fi.completeBaseName() + ".msh").toStdString();

		if (fileExists(output_str))
		{
			vtkXMLUnstructuredGridReader* reader = vtkXMLUnstructuredGridReader::New();
			reader->SetFileName(it->toStdString().c_str());
			reader->Update();

			const GridAdapter* grid = VtkMeshConverter::convertUnstructuredGrid(reader->GetOutput());
			FileIO::OGSMeshIO meshIO;
			meshIO.setMesh(grid->getCFEMesh());
			meshIO.writeToFile(output_str.c_str());
		
			delete grid;
			reader->Delete();
		}
	}

	OGSError::box("File conversion finished");
}

void OGSFileConverter::convertMSH2VTU(const QStringList &input, const QString &output)
{
	for (QStringList::const_iterator it=input.begin(); it!=input.end(); ++it)
	{
		const QFileInfo fi(*it);
		const std::string output_str = QString(output + "/" + fi.completeBaseName() + ".vtu").toStdString();
		const std::string msh_name = BaseLib::getFileNameFromPath(it->toStdString(), true);

		if (fileExists(output_str))
		{
			const GridAdapter grid(it->toStdString());
			VtkMeshSource* source = VtkMeshSource::New();
			source->SetGrid(&grid);
		
			vtkUnstructuredGridAlgorithm* alg = dynamic_cast<vtkUnstructuredGridAlgorithm*>(source);
			vtkXMLUnstructuredGridWriter* writer = vtkXMLUnstructuredGridWriter::New();
			writer->SetInput(alg->GetOutputDataObject(0));
			writer->SetDataModeToAscii();
			writer->SetCompressorTypeToNone();
			writer->SetFileName(output_str.c_str());
			writer->Write();
			writer->Delete();
		}
	}
	
	OGSError::box("File conversion finished");
}

void OGSFileConverter::convertCND2BC(const QStringList &input, const QString &output)
{
	ProjectData project;
	GEOLIB::GEOObjects* geo_objects = new GEOLIB::GEOObjects;
	project.setGEOObjects(geo_objects);

	// HACK for enabling conversion of files without loading the associated geometry
	std::vector<GEOLIB::Point*> *fake_geo = new std::vector<GEOLIB::Point*>;
	fake_geo->push_back(new GEOLIB::Point(0,0,0));
	std::string fake_name("conversionTestRun#1");
	geo_objects->addPointVec(fake_geo, fake_name);

	FileFinder fileFinder = createFileFinder();
	std::string schemaName(fileFinder.getPath("OpenGeoSysCond.xsd"));
	FileIO::XmlCndInterface xml(&project, schemaName);

	std::vector<FEMCondition*> conditions;

	for (QStringList::const_iterator it=input.begin(); it!=input.end(); ++it)
		xml.readFile(conditions, *it);

	if (!conditions.empty())
	{
		project.addConditions(conditions);
		QFileInfo fi(output);
		FEMCondition::CondType type = FEMCondition::UNSPECIFIED;
		if (fi.suffix().compare("bc") == 0)      type = FEMCondition::BOUNDARY_CONDITION;
		else if (fi.suffix().compare("ic") == 0) type = FEMCondition::INITIAL_CONDITION;
		else if (fi.suffix().compare("st") == 0) type = FEMCondition::SOURCE_TERM;

		size_t count(0);
		size_t nConds(conditions.size());
		for (size_t i=0; i<nConds; i++)
		{
			if (conditions[i]->getCondType() == type)
			{
				if (type == FEMCondition::BOUNDARY_CONDITION)
					bc_list.push_back(new CBoundaryCondition(static_cast<BoundaryCondition*>(conditions[i])));
				else if (type == FEMCondition::INITIAL_CONDITION)
					ic_vector.push_back(new CInitialCondition(static_cast<InitialCondition*>(conditions[i])));
				else if (type == FEMCondition::SOURCE_TERM)
					st_vector.push_back(new CSourceTerm(static_cast<SourceTerm*>(conditions[i])));

				if (conditions[i]->getProcessDistributionType() == FiniteElement::DIRECT)
				{
					std::string count_str (QString::number(count++).toStdString());
					std::string direct_value_file = fi.absolutePath().toStdString() + "/direct_values" + count_str + ".txt";
					st_vector[st_vector.size()-1]->fname = direct_value_file;
					ConversionTools::writeDirectValues(*conditions[i], direct_value_file);
				}
			}
		}
		if (type == FEMCondition::BOUNDARY_CONDITION)
			BCWrite(output.toStdString());
		else if (type == FEMCondition::INITIAL_CONDITION)
			ICWrite(output.toStdString());
		else if (type == FEMCondition::SOURCE_TERM)
			STWrite(output.toStdString());
	}
	OGSError::box("File conversion finished");
}

void OGSFileConverter::convertBC2CND(const QStringList &input, const QString &output)
{
	ProjectData project;
	std::vector<FEMCondition*> conditions;
	for (QStringList::const_iterator it=input.begin(); it!=input.end(); ++it)
		ConversionTools::getFEMConditionsFromASCIIFile(*it, conditions);

	if (!conditions.empty())
	{
		project.addConditions(conditions);
		FileFinder fileFinder = createFileFinder();
		std::string schemaName(fileFinder.getPath("OpenGeoSysCond.xsd"));
		FileIO::XmlCndInterface xml(&project, schemaName);
		xml.writeToFile(output.toStdString());
	}
	OGSError::box("File conversion finished");
}

FileFinder OGSFileConverter::createFileFinder()
{
	FileFinder fileFinder;
	fileFinder.addDirectory(".");
	fileFinder.addDirectory(std::string(SOURCEPATH).append("/FileIO"));
	return fileFinder;
}

void OGSFileConverter::on_gml2gliButton_pressed()
{
	FileListDialog dlg(FileListDialog::GML, FileListDialog::GLI);
	if (dlg.exec())
		convertGML2GLI(dlg.getInputFileList(), dlg.getOutputDir());
}

void OGSFileConverter::on_gli2gmlButton_pressed()
{
	FileListDialog dlg(FileListDialog::GLI, FileListDialog::GML);
	if (dlg.exec())
		convertGLI2GML(dlg.getInputFileList(), dlg.getOutputDir());
}

void OGSFileConverter::on_vtu2mshButton_pressed()
{
	FileListDialog dlg(FileListDialog::VTU, FileListDialog::MSH);
	if (dlg.exec())
		convertVTU2MSH(dlg.getInputFileList(), dlg.getOutputDir());
}

void OGSFileConverter::on_msh2vtuButton_pressed()
{
	FileListDialog dlg(FileListDialog::MSH, FileListDialog::VTU);
	if (dlg.exec())
		convertMSH2VTU(dlg.getInputFileList(), dlg.getOutputDir());
}

void OGSFileConverter::on_bc2cndButton_pressed()
{
	FileListDialog dlg(FileListDialog::BC, FileListDialog::CND);
	if (dlg.exec())
		convertBC2CND(dlg.getInputFileList(), dlg.getOutputDir());
}

void OGSFileConverter::on_cnd2bcButton_pressed()
{
	FileListDialog dlg(FileListDialog::CND, FileListDialog::BC);
	if (dlg.exec())
		convertCND2BC(dlg.getInputFileList(), dlg.getOutputDir());
}

void OGSFileConverter::on_closeDialogButton_pressed()
{
	this->close();
}

bool OGSFileConverter::fileExists(const std::string &file_name) const
{
	std::ifstream file(file_name);
	if (file)
	{
		QString name = QString::fromStdString(BaseLib::getFileNameFromPath(file_name, true));
		return OGSError::question("The file \'" + name + "\' already exists.\n Do you want to overwrite it?", "Warning");
	}
	return false;
}


