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

#ifndef __OVITO_HISTOGRAM_MODIFIER_EDITOR_H
#define __OVITO_HISTOGRAM_MODIFIER_EDITOR_H

#include <plugins/particles/gui/ParticlesGui.h>
#include <plugins/particles/gui/modifier/ParticleModifierEditor.h>

#ifndef signals
#define signals Q_SIGNALS
#endif
#include <qcustomplot.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * A properties editor for the HistogramModifier class.
 */
class HistogramModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE HistogramModifierEditor() : _rangeUpdate(true) {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

protected Q_SLOTS:

	/// Replots the histogram computed by the modifier.
	void plotHistogram();

	/// Keep x-axis range updated
	void updateXAxisRange(const QCPRange &newRange);

	/// This is called when the user has clicked the "Save Data" button.
	void onSaveData();

private:

	/// The graph widget to display the histogram.
	QCustomPlot* _histogramPlot;

	/// Marks the selection interval in the histogram plot.
	QCPItemStraightLine* _selectionRangeStartMarker;

	/// Marks the selection interval in the histogram plot.
	QCPItemStraightLine* _selectionRangeEndMarker;

	/// Update range when plot ranges change?
	bool _rangeUpdate;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_HISTOGRAM_MODIFIER_EDITOR_H
