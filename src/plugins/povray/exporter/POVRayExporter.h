///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2016) Alexander Stukowski
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  OVITO is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once


#include <core/Core.h>
#include <core/dataset/io/FileExporter.h>
#include "../renderer/POVRayRenderer.h"

namespace Ovito { namespace POVRay {

/**
 * \brief Export services that write the scene to a POV-Ray file.
 */
class OVITO_POVRAY_EXPORT POVRayExporter : public FileExporter
{
	/// Defines a metaclass specialization for this exporter type.
	class OOMetaClass : public FileExporter::OOMetaClass
	{
	public:
		
		/// Inherit standard constructor from base meta class.
		using FileExporter::OOMetaClass::OOMetaClass;

		/// Returns the file filter that specifies the files that can be exported by this service.
		virtual QString fileFilter() const override { 
	#ifndef Q_OS_WIN
			return QStringLiteral("*.pov");
	#else 
			// Workaround for bug in Windows file selection dialog (https://bugreports.qt.io/browse/QTBUG-45759)
			return QStringLiteral("*");
	#endif
		}

		/// Returns the filter description that is displayed in the drop-down box of the file dialog.
		virtual QString fileFilterDescription() const override { return tr("POV-Ray scene"); }
	};

	Q_OBJECT
	OVITO_CLASS_META(POVRayExporter, OOMetaClass)
	
public:

	/// \brief Constructs a new instance of this class.
	Q_INVOKABLE POVRayExporter(DataSet* dataset);

	/// \brief Selects the nodes from the scene to be exported by this exporter if no specific set of nodes was provided.
	virtual void selectStandardOutputData() override; 

	/// \brief This is called once for every output file to be written and before exportData() is called.
	virtual bool openOutputFile(const QString& filePath, int numberOfFrames) override;

	/// \brief This is called once for every output file written after exportData() has been called.
	virtual void closeOutputFile(bool exportCompleted) override;

	/// Returns the current file this exporter is writing to.
	QFile& outputFile() { return _outputFile; }

protected:

	/// \brief Exports a single animation frame to the current output file.
	virtual bool exportFrame(int frameNumber, TimePoint time, const QString& filePath, TaskManager& taskManager) override;

private:

	/// The output file stream.
	QFile _outputFile;

	/// The internal renderer, which is responsible for streaming the scene to a POVRay scene file.
	OORef<POVRayRenderer> _renderer;
};

}	// End of namespace
}	// End of namespace


