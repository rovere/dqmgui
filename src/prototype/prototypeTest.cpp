#define DNDEBUG

#include <Rtypes.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TH1.h>
#include <THStack.h>
#include <TObject.h>
#include <iostream>

#include "../cpp/DQM/render/builders/DataHistogramBuilder.h"
#include "../cpp/DQM/render/builders/HistogramBuilder.h"
#include "../cpp/DQM/render/builders/StackedHistogramBuilder.h"
#include "../cpp/DQM/render/models/display-data/HistogramDisplayData.h"
#include "../cpp/DQM/render/models/display-data/HistogramStackDisplayData.h"
#include "../cpp/DQM/render/utils/RandomHistogramGenerator.h"


namespace prototype {
	HistogramStackDisplayData generateMCStackDisplayData(Double_t weights[]) {
		HistogramStackDisplayData *histogramStackDisplayData = new HistogramStackDisplayData();

		for(Int_t i = 0; i < sizeof(weights); i++) {
			TH1D *histogram = RandomHistogramGenerator::createMCHistogram();
			Double_t weight = weights[i];

			HistogramDisplayData *histogramDisplayData = new HistogramDisplayData(histogram, weight);
			histogramStackDisplayData->add(*histogramDisplayData);
		}

		return(*histogramStackDisplayData);
	}

	/// Runs an example of the histogram display program.
	void runExample(Double_t weights[]) {
		TApplication *application = new TApplication("App", 0, 0);
		TCanvas *canvas = new TCanvas("canvas", "Test Application", 400, 400);

		TH1D dataHistogram = *RandomHistogramGenerator::createGausHistogram(-1);
		HistogramStackDisplayData histogramStackDisplayData = generateMCStackDisplayData(weights);

		DataHistogramBuilder *dataHistogramBuilder = new DataHistogramBuilder(&dataHistogram);
		TH1D formattedDataHistogram = dataHistogramBuilder->build();
		formattedDataHistogram.Draw();

		// (Director pattern)
		Double_t targetStackedHistogramArea = dataHistogram.Integral();
		StackedHistogramBuilder *stackedHistogramBuilder = new StackedHistogramBuilder(
				targetStackedHistogramArea);
		stackedHistogramBuilder->addHistogramStackDisplayData(histogramStackDisplayData);
		THStack histogramStack = stackedHistogramBuilder->build();
		histogramStack.Draw("SAME");

		canvas->Update();
		application->Run();
	}
}


#ifndef __CINT__
int main(int argc, const char* argv[]) {
	Double_t weights[] = {0.6, 0.1, 0.3};
	prototype::runExample(weights);

	std::cout << "Complete";
	return(0);
}
# endif
