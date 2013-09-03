/*
 * StackedHistogramBuilder.h
 *
 *  Created on: 3 Sep 2013
 *      Author: Colin - CERN
 */
#ifndef STACKEDHISTOGRAMBUILDER_H_
#define STACKEDHISTOGRAMBUILDER_H_

#include <cassert>

#include "../../models/AbstractHistogramStackData.h"

#define DNDEBUG

#include <Rtypes.h>
#include <TCollection.h>
#include <TH1.h>
#include <THStack.h>
#include <TObjArray.h>
#include <TObject.h>
#include <cassert>
#include <vector>
#include <cassert>

#include "../../models/HistogramData.h"
#include "../../models/HistogramStackData.h"

#include "Builder.h"

class THStack;

namespace render {
	/// TODO: Comment
	template <class T> class StackedHistogramBuilder : public Builder<THStack*> {
		private:
			/// TODO: Comment
		AbstractHistogramStackData<T> *stackData;

		public:
			/// TODO: Comment
			StackedHistogramBuilder<T>(AbstractHistogramStackData<T> *stackData)
					: stackData(stackData) {
				// Nothing to do here
			}

			~StackedHistogramBuilder<T>() {
				delete stackData;
			}

			/// TODO: Comment
			THStack* build() {
				THStack *histogramStack = new THStack();
				std::vector<TH1*> histograms = this->stackData->getAllHistograms();
				StackedHistogramBuilder<T>::addAllToTHStack(histograms, histogramStack);
				return(histogramStack);
			}

			/// TODO: Comment
			void addHistogramData(T histogramData) {
				this->stackData->add(histogramData);
			}

		protected:
			/// Adds a given histogram to the given histograms stack.
			/// @param histogram pointer to the histogram to put on the histogram stack
			/// @param histogramStack the histogram stack that the histogram is to be added to
			static void addToTHStack(TH1 *histogram, THStack *histogramStack) {
				#ifdef DNDEBUG
				TObjArray *histogramsInStack = histogramStack->GetStack();
				Int_t stackSize = (histogramsInStack == nullptr)
										? 0
										: histogramsInStack->GetSize();
				#endif

				histogramStack->Add(histogram);

				#ifdef DNDEBUG
				Int_t modifiedStackSize = histogramStack->GetStack()->GetSize();
				assert(modifiedStackSize == (stackSize + 1));
				#endif
			}

			/// Adds all of the histograms given to the given histogram stack.
			/// @param histograms the list of histograms to add to the stack
			/// @param histogramStack the histogram stack that the histogram is to be added to
			static void addAllToTHStack(std::vector<TH1*> histograms, THStack *histogramStack) {
				#ifdef DNDEBUG
				TObjArray *histogramsInStack = histogramStack->GetStack();
				Int_t stackSize = (histogramsInStack == nullptr)
										? 0
										: histogramsInStack->GetSize();
				#endif

				std::vector<TH1*>::iterator it = histograms.begin();

				while(it != histograms.end()) {
					TH1 *histogram = *it;
					StackedHistogramBuilder<T>::addToTHStack(histogram, histogramStack);
					it++;
				}

				#ifdef DNDEBUG
				Int_t modifiedStackSize = histogramStack->GetStack()->GetSize();
				assert(modifiedStackSize == (stackSize + histograms.size()));
				#endif
			}
	};
}
#endif
