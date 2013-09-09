/*
 * StackedHistogramBuilder.h
 *
 *  Created on: 3 Sep 2013
 *      Author: Colin - CERN
 */
#ifndef STACKEDHISTOGRAMBUILDER_H_
#define STACKEDHISTOGRAMBUILDER_H_
#define DNDEBUG

#include <Rtypes.h>
#include <TCollection.h>
#include <TH1.h>
#include <THStack.h>
#include <TObjArray.h>
#include <TObject.h>
#include <cassert>
#include <vector>

#include "../../models/AbstractHistogramStackData.h"
#include "Builder.h"

namespace render {
	/// Abstract superclass of all <code>THStack</code> stacked histogram builders.
	template <class T> class StackedHistogramBuilder : public Builder<THStack*> {
		private:
			/// Pointer to the data that is used by this builder to build the <code>THStack</code>
			///	instance.
			AbstractHistogramStackData<T> *stackData;

		public:
			/// Default constructor.
			/// @param stackData pointer to the data that is used by this builder to build the
			///					 <code>THStack</code> instance.
			StackedHistogramBuilder<T>(AbstractHistogramStackData<T> *stackData)
					: stackData(stackData) {
				// Nothing to do here
			}

			/// Destructor.
			~StackedHistogramBuilder<T>() {
				delete stackData;
			}

			/// Builds the histogram stack.
			/// @return the build <code>THStack</code> instance
			THStack* build() {
				THStack *histogramStack = new THStack();
				std::vector<TH1*> histograms = this->stackData->getAllHistograms();
				StackedHistogramBuilder<T>::addAllToTHStack(histograms, histogramStack);
				return(histogramStack);
			}

			/// Adds data about the histogram that this builder is to build.
			/// @param histogramData data about the histogram that this build is to build
			void addHistogramData(T histogramData) {
				this->stackData->add(histogramData);
			}

		protected:
			/// Adds a given histogram to the given histograms stack.
			/// @param histogram pointer to the histogram to add to the histogram stack
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
