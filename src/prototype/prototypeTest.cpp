#include <iostream>
#include <new>

#include "utils/RandomHistogramGenerator.h"

#include <Rtypes.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TH1.h>
#include <THStack.h>
#include <TObject.h>
#include <cassert>
#include <iostream>
#include <list>
#include <string>

#include "builders/DataHistogramBuilder.h"
#include "builders/StackedHistogramBuilder.h"
#include "models/HistogramDisplayData.h"
#include "models/HistogramStackDisplayData.h"


namespace prototype {
	/// Runs an example of the histogram display program.
	void runExample(Int_t numberOfMCHistograms, Double_t weights[]) {
		TApplication *application = new TApplication("App", 0, 0);

		TCanvas *canvas = new TCanvas("c", "Test Application", 400, 400);
		TH1D dataHistogram = *RandomHistogramGenerator::createGausHistogram(-1);
		HistogramStackDisplayData *histogramStackDisplayData = new HistogramStackDisplayData();

		for(Int_t i = 0; i < numberOfMCHistograms; i++) {
			TH1D *histogram = RandomHistogramGenerator::createMCHistogram();
			Double_t weight = weights[i];

			HistogramDisplayData *histogramDisplayData = new HistogramDisplayData(histogram, weight);
			histogramStackDisplayData->add(*histogramDisplayData);
		}

		// (Director pattern)
		StackedHistogramBuilder *stackedHistogramBuilder = new StackedHistogramBuilder();
		stackedHistogramBuilder->addHistogramStackDisplayData(*histogramStackDisplayData);
		THStack histogramStack = stackedHistogramBuilder->build();
		histogramStack.Draw();

		DataHistogramBuilder *dataHistogramBuilder = new DataHistogramBuilder(&dataHistogram);
		TH1D formattedDataHistogram = dataHistogramBuilder->build();
		formattedDataHistogram.Draw("SAME");

		canvas->Update();
		application->Run();
	}
}


#ifndef __CINT__
int main(int argc, const char* argv[]) {
	Double_t weights[] = {0.6, 0.1, 0.3};

	Int_t numberOfWeights = sizeof(weights) / sizeof(Double_t);
	prototype::runExample(numberOfWeights, weights);

	std::cout << "Complete";
	return(0);
}
# endif
