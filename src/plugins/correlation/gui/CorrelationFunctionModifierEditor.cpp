///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2017) Alexander Stukowski
//  Copyright (2017) Lars Pastewka
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

#include <plugins/particles/gui/ParticlesGui.h>
#include <plugins/correlation/CorrelationFunctionModifier.h>
#include <gui/mainwin/MainWindow.h>
#include <gui/properties/BooleanParameterUI.h>
#include <gui/properties/IntegerParameterUI.h>
#include <gui/properties/IntegerRadioButtonParameterUI.h>
#include <gui/properties/FloatParameterUI.h>
#include <gui/properties/VariantComboBoxParameterUI.h>
#include <plugins/stdobj/gui/widgets/PropertyReferenceParameterUI.h>
#include "CorrelationFunctionModifierEditor.h"

#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_scale_engine.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

IMPLEMENT_OVITO_CLASS(CorrelationFunctionModifierEditor);
SET_OVITO_OBJECT_EDITOR(CorrelationFunctionModifier, CorrelationFunctionModifierEditor);

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void CorrelationFunctionModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Correlation function"), rolloutParams, "particles.modifiers.correlation_function.html");

	// Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	PropertyReferenceParameterUI* sourceProperty1UI = new PropertyReferenceParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::sourceProperty1), &ParticleProperty::OOClass());
	layout->addWidget(new QLabel(tr("First property:"), rollout));
	layout->addWidget(sourceProperty1UI->comboBox());

	PropertyReferenceParameterUI* sourceProperty2UI = new PropertyReferenceParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::sourceProperty2), &ParticleProperty::OOClass());
	layout->addWidget(new QLabel(tr("Second property:"), rollout));
	layout->addWidget(sourceProperty2UI->comboBox());

	QGridLayout* gridlayout = new QGridLayout();
	gridlayout->setContentsMargins(4,4,4,4);
	gridlayout->setColumnStretch(1, 1);

	// FFT grid spacing parameter.
	FloatParameterUI* fftGridSpacingRadiusPUI = new FloatParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::fftGridSpacing));
	gridlayout->addWidget(fftGridSpacingRadiusPUI->label(), 0, 0);
	gridlayout->addLayout(fftGridSpacingRadiusPUI->createFieldLayout(), 0, 1);

	layout->addLayout(gridlayout);

	BooleanParameterUI* applyWindowUI = new BooleanParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::applyWindow));
	layout->addWidget(applyWindowUI->checkBox());

#if 0
	gridlayout = new QGridLayout();
	gridlayout->addWidget(new QLabel(tr("Average:"), rollout), 0, 0);
	VariantComboBoxParameterUI* averagingDirectionPUI = new VariantComboBoxParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::averagingDirection));
    averagingDirectionPUI->comboBox()->addItem("radial", QVariant::fromValue(CorrelationFunctionModifier::RADIAL));
    averagingDirectionPUI->comboBox()->addItem("cell vector 1", QVariant::fromValue(CorrelationFunctionModifier::CELL_VECTOR_1));
    averagingDirectionPUI->comboBox()->addItem("cell vector 2", QVariant::fromValue(CorrelationFunctionModifier::CELL_VECTOR_2));
    averagingDirectionPUI->comboBox()->addItem("cell vector 3", QVariant::fromValue(CorrelationFunctionModifier::CELL_VECTOR_3));
    gridlayout->addWidget(averagingDirectionPUI->comboBox(), 0, 1);
    layout->addLayout(gridlayout);
#endif

	QGroupBox* realSpaceGroupBox = new QGroupBox(tr("Real-space correlation function"));
	layout->addWidget(realSpaceGroupBox);

	BooleanParameterUI* doComputeNeighCorrelationUI = new BooleanParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::doComputeNeighCorrelation));

	QGridLayout* realSpaceGridLayout = new QGridLayout();
	realSpaceGridLayout->setContentsMargins(4,4,4,4);
	realSpaceGridLayout->setColumnStretch(1, 1);

	// Neighbor cutoff parameter.
	FloatParameterUI *neighCutoffRadiusPUI = new FloatParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::neighCutoff));
	neighCutoffRadiusPUI->setEnabled(false);
	realSpaceGridLayout->addWidget(neighCutoffRadiusPUI->label(), 1, 0);
	realSpaceGridLayout->addLayout(neighCutoffRadiusPUI->createFieldLayout(), 1, 1);

	// Number of bins parameter.
	IntegerParameterUI* numberOfNeighBinsPUI = new IntegerParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::numberOfNeighBins));
	numberOfNeighBinsPUI->setEnabled(false);
	realSpaceGridLayout->addWidget(numberOfNeighBinsPUI->label(), 2, 0);
	realSpaceGridLayout->addLayout(numberOfNeighBinsPUI->createFieldLayout(), 2, 1);

	connect(doComputeNeighCorrelationUI->checkBox(), &QCheckBox::toggled, neighCutoffRadiusPUI, &FloatParameterUI::setEnabled);
	connect(doComputeNeighCorrelationUI->checkBox(), &QCheckBox::toggled, numberOfNeighBinsPUI, &IntegerParameterUI::setEnabled);

	QGridLayout *normalizeRealSpaceLayout = new QGridLayout();
	normalizeRealSpaceLayout->addWidget(new QLabel(tr("Normalization:"), rollout), 0, 0);
	VariantComboBoxParameterUI* normalizeRealSpacePUI = new VariantComboBoxParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::normalizeRealSpace));
    normalizeRealSpacePUI->comboBox()->addItem("Do not normalize", QVariant::fromValue(CorrelationFunctionModifier::DO_NOT_NORMALIZE));
    normalizeRealSpacePUI->comboBox()->addItem("by covariance", QVariant::fromValue(CorrelationFunctionModifier::NORMALIZE_BY_COVARIANCE));
    normalizeRealSpacePUI->comboBox()->addItem("by RDF", QVariant::fromValue(CorrelationFunctionModifier::NORMALIZE_BY_RDF));
    normalizeRealSpaceLayout->addWidget(normalizeRealSpacePUI->comboBox(), 0, 1);

	QGridLayout* typeOfRealSpacePlotLayout = new QGridLayout();
	IntegerRadioButtonParameterUI *typeOfRealSpacePlotPUI = new IntegerRadioButtonParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::typeOfRealSpacePlot));
	typeOfRealSpacePlotLayout->addWidget(new QLabel(tr("Display as:")), 0, 0);
	typeOfRealSpacePlotLayout->addWidget(typeOfRealSpacePlotPUI->addRadioButton(0, tr("lin-lin")), 0, 1);
	typeOfRealSpacePlotLayout->addWidget(typeOfRealSpacePlotPUI->addRadioButton(1, tr("log-lin")), 0, 2);
	typeOfRealSpacePlotLayout->addWidget(typeOfRealSpacePlotPUI->addRadioButton(3, tr("log-log")), 0, 3);

	_realSpacePlot = new QwtPlot();
	_realSpacePlot->setMinimumHeight(200);
	_realSpacePlot->setMaximumHeight(200);
	_realSpacePlot->setCanvasBackground(Qt::white);
	_realSpacePlot->setAxisTitle(QwtPlot::xBottom, tr("Distance r"));
	_realSpacePlot->setAxisTitle(QwtPlot::yLeft, tr("C(r)"));

	// Axes.
	QGroupBox* axesBox = new QGroupBox(tr("Plot axes"), rollout);
	QVBoxLayout* axesSublayout = new QVBoxLayout(axesBox);
	axesSublayout->setContentsMargins(4,4,4,4);
	layout->addWidget(axesBox);
	// x-axis.
	{
		BooleanParameterUI* rangeUI = new BooleanParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::fixRealSpaceXAxisRange));
		axesSublayout->addWidget(rangeUI->checkBox());

		QHBoxLayout* hlayout = new QHBoxLayout();
		axesSublayout->addLayout(hlayout);
		FloatParameterUI* startPUI = new FloatParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::realSpaceXAxisRangeStart));
		FloatParameterUI* endPUI = new FloatParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::realSpaceXAxisRangeEnd));
		hlayout->addWidget(new QLabel(tr("From:")));
		hlayout->addLayout(startPUI->createFieldLayout());
		hlayout->addSpacing(12);
		hlayout->addWidget(new QLabel(tr("To:")));
		hlayout->addLayout(endPUI->createFieldLayout());
		startPUI->setEnabled(false);
		endPUI->setEnabled(false);
		connect(rangeUI->checkBox(), &QCheckBox::toggled, startPUI, &FloatParameterUI::setEnabled);
		connect(rangeUI->checkBox(), &QCheckBox::toggled, endPUI, &FloatParameterUI::setEnabled);
	}
	// y-axis.
	{
		BooleanParameterUI* rangeUI = new BooleanParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::fixRealSpaceYAxisRange));
		axesSublayout->addWidget(rangeUI->checkBox());

		QHBoxLayout* hlayout = new QHBoxLayout();
		axesSublayout->addLayout(hlayout);
		FloatParameterUI* startPUI = new FloatParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::realSpaceYAxisRangeStart));
		FloatParameterUI* endPUI = new FloatParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::realSpaceYAxisRangeEnd));
		hlayout->addWidget(new QLabel(tr("From:")));
		hlayout->addLayout(startPUI->createFieldLayout());
		hlayout->addSpacing(12);
		hlayout->addWidget(new QLabel(tr("To:")));
		hlayout->addLayout(endPUI->createFieldLayout());
		startPUI->setEnabled(false);
		endPUI->setEnabled(false);
		connect(rangeUI->checkBox(), &QCheckBox::toggled, startPUI, &FloatParameterUI::setEnabled);
		connect(rangeUI->checkBox(), &QCheckBox::toggled, endPUI, &FloatParameterUI::setEnabled);
	}

	QVBoxLayout* realSpaceLayout = new QVBoxLayout(realSpaceGroupBox);
	realSpaceLayout->addWidget(doComputeNeighCorrelationUI->checkBox());
	realSpaceLayout->addLayout(realSpaceGridLayout);
	realSpaceLayout->addLayout(normalizeRealSpaceLayout);
	realSpaceLayout->addLayout(typeOfRealSpacePlotLayout);
	realSpaceLayout->addWidget(_realSpacePlot);
	realSpaceLayout->addWidget(axesBox);

	QGroupBox* reciprocalSpaceGroupBox = new QGroupBox(tr("Reciprocal-space correlation function"));
	layout->addWidget(reciprocalSpaceGroupBox);

	BooleanParameterUI* normalizeReciprocalSpaceUI = new BooleanParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::normalizeReciprocalSpace));

	QGridLayout* typeOfReciprocalSpacePlotLayout = new QGridLayout();
	IntegerRadioButtonParameterUI *typeOfReciprocalSpacePlotPUI = new IntegerRadioButtonParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::typeOfReciprocalSpacePlot));
	typeOfReciprocalSpacePlotLayout->addWidget(new QLabel(tr("Display as:")), 0, 0);
	typeOfReciprocalSpacePlotLayout->addWidget(typeOfReciprocalSpacePlotPUI->addRadioButton(0, tr("lin-lin")), 0, 1);
	typeOfReciprocalSpacePlotLayout->addWidget(typeOfReciprocalSpacePlotPUI->addRadioButton(1, tr("log-lin")), 0, 2);
	typeOfReciprocalSpacePlotLayout->addWidget(typeOfReciprocalSpacePlotPUI->addRadioButton(3, tr("log-log")), 0, 3);

	_reciprocalSpacePlot = new QwtPlot();
	_reciprocalSpacePlot->setMinimumHeight(200);
	_reciprocalSpacePlot->setMaximumHeight(200);
	_reciprocalSpacePlot->setCanvasBackground(Qt::white);
	_reciprocalSpacePlot->setAxisTitle(QwtPlot::xBottom, tr("Wavevector q"));
	_reciprocalSpacePlot->setAxisTitle(QwtPlot::yLeft, tr("C(q)"));

	// Axes.
	axesBox = new QGroupBox(tr("Plot axes"), rollout);
	axesSublayout = new QVBoxLayout(axesBox);
	axesSublayout->setContentsMargins(4,4,4,4);
	layout->addWidget(axesBox);
	// x-axis.
	{
		BooleanParameterUI* rangeUI = new BooleanParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::fixReciprocalSpaceXAxisRange));
		axesSublayout->addWidget(rangeUI->checkBox());

		QHBoxLayout* hlayout = new QHBoxLayout();
		axesSublayout->addLayout(hlayout);
		FloatParameterUI* startPUI = new FloatParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::reciprocalSpaceXAxisRangeStart));
		FloatParameterUI* endPUI = new FloatParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::reciprocalSpaceXAxisRangeEnd));
		hlayout->addWidget(new QLabel(tr("From:")));
		hlayout->addLayout(startPUI->createFieldLayout());
		hlayout->addSpacing(12);
		hlayout->addWidget(new QLabel(tr("To:")));
		hlayout->addLayout(endPUI->createFieldLayout());
		startPUI->setEnabled(false);
		endPUI->setEnabled(false);
		connect(rangeUI->checkBox(), &QCheckBox::toggled, startPUI, &FloatParameterUI::setEnabled);
		connect(rangeUI->checkBox(), &QCheckBox::toggled, endPUI, &FloatParameterUI::setEnabled);
	}
	// y-axis.
	{
		BooleanParameterUI* rangeUI = new BooleanParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::fixReciprocalSpaceYAxisRange));
		axesSublayout->addWidget(rangeUI->checkBox());

		QHBoxLayout* hlayout = new QHBoxLayout();
		axesSublayout->addLayout(hlayout);
		FloatParameterUI* startPUI = new FloatParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::reciprocalSpaceYAxisRangeStart));
		FloatParameterUI* endPUI = new FloatParameterUI(this, PROPERTY_FIELD(CorrelationFunctionModifier::reciprocalSpaceYAxisRangeEnd));
		hlayout->addWidget(new QLabel(tr("From:")));
		hlayout->addLayout(startPUI->createFieldLayout());
		hlayout->addSpacing(12);
		hlayout->addWidget(new QLabel(tr("To:")));
		hlayout->addLayout(endPUI->createFieldLayout());
		startPUI->setEnabled(false);
		endPUI->setEnabled(false);
		connect(rangeUI->checkBox(), &QCheckBox::toggled, startPUI, &FloatParameterUI::setEnabled);
		connect(rangeUI->checkBox(), &QCheckBox::toggled, endPUI, &FloatParameterUI::setEnabled);
	}

	QVBoxLayout* reciprocalSpaceLayout = new QVBoxLayout(reciprocalSpaceGroupBox);
	reciprocalSpaceLayout->addWidget(normalizeReciprocalSpaceUI->checkBox());
	reciprocalSpaceLayout->addLayout(typeOfReciprocalSpacePlotLayout);
	reciprocalSpaceLayout->addWidget(_reciprocalSpacePlot);
	reciprocalSpaceLayout->addWidget(axesBox);

	connect(this, &CorrelationFunctionModifierEditor::contentsReplaced, this, &CorrelationFunctionModifierEditor::plotAllData);

	layout->addSpacing(12);
	QPushButton* saveDataButton = new QPushButton(tr("Export data to text file"));
	layout->addWidget(saveDataButton);
	connect(saveDataButton, &QPushButton::clicked, this, &CorrelationFunctionModifierEditor::onSaveData);

	// Status label.
	layout->addSpacing(6);
	layout->addWidget(statusLabel());
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool CorrelationFunctionModifierEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if (event->sender() == editObject() &&
		(event->type() == ReferenceEvent::TargetChanged ||
		 event->type() == ReferenceEvent::ObjectStatusChanged)) {
		plotAllDataLater(this);
	}
	return ModifierPropertiesEditor::referenceEvent(source, event);
}

/******************************************************************************
* Plot correlation function.
******************************************************************************/
void CorrelationFunctionModifierEditor::plotData(const QVector<FloatType> &xData,
												 const QVector<FloatType> &yData,
												 QwtPlot *plot,
												 QwtPlotCurve *&curve,
												 FloatType offset, FloatType fac)
{
	OVITO_ASSERT(xData.size() == yData.size());

	if(!curve) {
		curve = new QwtPlotCurve();
		curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
		curve->setBrush(Qt::lightGray);
		curve->attach(plot);
		QwtPlotGrid* plotGrid = new QwtPlotGrid();
		plotGrid->setPen(Qt::gray, 0, Qt::DotLine);
		plotGrid->attach(plot);
	}

	// Set data to plot.
	size_t numberOfDataPoints = yData.size();
	int startAt = 0;
	QVector<QPointF> plotData(numberOfDataPoints-startAt);
	for (int i = startAt; i < numberOfDataPoints; i++) {
		FloatType xValue = xData[i];
		FloatType yValue = fac*(yData[i]-offset);
		plotData[i-startAt].rx() = xValue;
		plotData[i-startAt].ry() = yValue;
	}
	curve->setSamples(plotData);
}

/******************************************************************************
* Updates the plot of the RDF computed by the modifier.
******************************************************************************/
void CorrelationFunctionModifierEditor::plotAllData()
{
	CorrelationFunctionModifier* modifier = static_object_cast<CorrelationFunctionModifier>(editObject());
	CorrelationFunctionModifierApplication* modApp = dynamic_object_cast<CorrelationFunctionModifierApplication>(someModifierApplication());
	if(!modifier || !modApp)
		return;

	FloatType offset = 0.0;
	FloatType fac = 1.0;
	if (modifier->normalizeRealSpace() == CorrelationFunctionModifier::NORMALIZE_BY_COVARIANCE) {
		offset = modApp->mean1()*modApp->mean2();
		fac = 1.0/(modApp->covariance()-offset);
	}
	FloatType rfac = 1.0;
	if (modifier->normalizeReciprocalSpace()) {
		rfac = 1.0/(modApp->covariance()-modApp->mean1()*modApp->mean2());
	}

	modifier->updateRanges(offset, fac, rfac, modApp);

	// Plot real-space correlation function
	if(!modApp->realSpaceCorrelationX().empty() &&
	   !modApp->realSpaceCorrelation().empty()) {
	    QVector<FloatType> y = modApp->realSpaceCorrelation();
		if (modifier->normalizeRealSpace() == CorrelationFunctionModifier::NORMALIZE_BY_RDF)
		    std::transform(y.begin(), y.end(), modApp->realSpaceRDF().constBegin(), y.begin(), std::divides<FloatType>());
		plotData(modApp->realSpaceCorrelationX(), y,
				 _realSpacePlot, _realSpaceCurve,
				 offset, fac);
	}

	if(!modApp->neighCorrelationX().empty() &&
	   !modApp->neighCorrelation().empty() &&
	   modifier->doComputeNeighCorrelation()) {
		if(!_neighCurve) {
			_neighCurve = new QwtPlotCurve();
			_neighCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
			_neighCurve->setPen(Qt::red);
			_neighCurve->attach(_realSpacePlot);
		}

		// Set data to plot.
		const auto &xData = modApp->neighCorrelationX();
		const auto &yData = modApp->neighCorrelation();
		const auto &rdfData = modApp->neighRDF();
		size_t numberOfDataPoints = yData.size();
		QVector<QPointF> plotData(numberOfDataPoints);
		bool normByRDF = modifier->normalizeRealSpace() == CorrelationFunctionModifier::NORMALIZE_BY_RDF;
		for (int i = 0; i < numberOfDataPoints; i++) {
			FloatType xValue = xData[i];
			FloatType yValue = fac*(yData[i]-offset);
			if (normByRDF)
				yValue /= rdfData[i];
			plotData[i].rx() = xValue;
			plotData[i].ry() = yValue;
		}
		_neighCurve->setSamples(plotData);
	}
	else {
		// Remove curve from plot if it exists.
		if (_neighCurve) {
			_neighCurve->detach();
			delete _neighCurve;
			_neighCurve = nullptr;
		}
	}

	// Plot reciprocal-space correlation function
	if(!modApp->reciprocalSpaceCorrelationX().empty() &&
	   !modApp->reciprocalSpaceCorrelation().empty()) {
		plotData(modApp->reciprocalSpaceCorrelationX(),
				 modApp->reciprocalSpaceCorrelation(),
				 _reciprocalSpacePlot,
				 _reciprocalSpaceCurve,
				 0.0, rfac);
	}

	// Set type of plot.
	if (modifier->typeOfRealSpacePlot() & 1)
		_realSpacePlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine());
	else
		_realSpacePlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine());
	if (modifier->typeOfRealSpacePlot() & 2)
		_realSpacePlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine());
	else
		_realSpacePlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine());

	if (modifier->typeOfReciprocalSpacePlot() & 1)
		_reciprocalSpacePlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine());
	else
		_reciprocalSpacePlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine());
	if (modifier->typeOfReciprocalSpacePlot() & 2)
		_reciprocalSpacePlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine());
	else
		_reciprocalSpacePlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine());

	_realSpacePlot->setAxisScale(QwtPlot::xBottom, modifier->realSpaceXAxisRangeStart(), modifier->realSpaceXAxisRangeEnd());
	_realSpacePlot->setAxisScale(QwtPlot::yLeft, modifier->realSpaceYAxisRangeStart(), modifier->realSpaceYAxisRangeEnd());
	_reciprocalSpacePlot->setAxisScale(QwtPlot::xBottom, modifier->reciprocalSpaceXAxisRangeStart(), modifier->reciprocalSpaceXAxisRangeEnd());
	_reciprocalSpacePlot->setAxisScale(QwtPlot::yLeft, modifier->reciprocalSpaceYAxisRangeStart(), modifier->reciprocalSpaceYAxisRangeEnd());

	_realSpacePlot->replot();
	_reciprocalSpacePlot->replot();
}

/******************************************************************************
* This is called when the user has clicked the "Save Data" button.
******************************************************************************/
void CorrelationFunctionModifierEditor::onSaveData()
{
	CorrelationFunctionModifier* modifier = static_object_cast<CorrelationFunctionModifier>(editObject());
	CorrelationFunctionModifierApplication* modApp = dynamic_object_cast<CorrelationFunctionModifierApplication>(someModifierApplication());
	if(!modifier || !modApp)
		return;

	if(modApp->realSpaceCorrelation().empty() && modApp->neighCorrelation().empty() && modApp->reciprocalSpaceCorrelation().empty())
		return;

	QString fileName = QFileDialog::getSaveFileName(mainWindow(),
		tr("Save Correlation Data"), QString(), tr("Text files (*.txt);;All files (*)"));
	if(fileName.isEmpty())
		return;

	try {
		QFile file(fileName);
		if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
			modifier->throwException(tr("Could not open file for writing: %1").arg(file.errorString()));

		QTextStream stream(&file);

		stream << "# This file contains the correlation between the following property:" << endl;
		stream << "# " << modifier->sourceProperty1().name() << " with mean value " << modApp->mean1() << endl;
		stream << "# " << modifier->sourceProperty2().name() << " with mean value " << modApp->mean2() << endl;
		stream << "# Covariance is " << modApp->covariance() << endl << endl;

		if (!modApp->realSpaceCorrelation().empty()) {
			stream << "# Real-space correlation function from FFT follows." << endl;
			stream << "# 1: Bin number" << endl;
			stream << "# 2: Distance r" << endl;
			stream << "# 3: Correlation function C(r)" << endl;
			stream << "# 4: Radial distribution function g(r)" << endl;
			for(int i = 0; i < modApp->realSpaceCorrelation().size(); i++) {
				stream << i << "\t" << modApp->realSpaceCorrelationX()[i]
				            << "\t" << modApp->realSpaceCorrelation()[i]
										<< "\t" << modApp->realSpaceRDF()[i] << endl;
			}
			stream << endl;
		}

		if (!modApp->neighCorrelation().empty()) {
			stream << "# Real-space correlation function from direct sum over neighbors follows." << endl;
			stream << "# 1: Bin number" << endl;
			stream << "# 2: Distance r" << endl;
			stream << "# 3: Correlation function C(r)" << endl;
			stream << "# 4: Radial distribution function g(r)" << endl;
			for(int i = 0; i < modApp->neighCorrelation().size(); i++) {
				stream << i << "\t" << modApp->neighCorrelationX()[i]
				            << "\t" << modApp->neighCorrelation()[i]
										<< "\t" << modApp->neighRDF()[i] << endl;
			}
			stream << endl;
		}

		if (!modApp->reciprocalSpaceCorrelation().empty()) {
			stream << "# Reciprocal-space correlation function from FFT follows." << endl;
			stream << "# 1: Bin number" << endl;
			stream << "# 2: Wavevector q (includes a factor of 2*pi)" << endl;
			stream << "# 3: Correlation function C(q)" << endl;
			for(int i = 0; i < modApp->reciprocalSpaceCorrelation().size(); i++) {
				stream << i << "\t" << modApp->reciprocalSpaceCorrelationX()[i]
				            << "\t" << modApp->reciprocalSpaceCorrelation()[i] << endl;
			}
		}
	}
	catch(const Exception& ex) {
		ex.reportError();
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
