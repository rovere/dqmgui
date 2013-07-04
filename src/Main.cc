#include <TApplication.h>
#include <TCanvas.h>
#include <TPaveLabel.h>
#include <TF1.h>


void test() {
	TApplication theApp("App", 0, 0);
	TCanvas *canvas = new TCanvas("c", "The Hello Canvas", 400, 400);
	TPaveLabel *hello = new TPaveLabel(0.2,0.4,0.8,0.6,"Hello World");

	TF1 *plot = new TF1("tmp", "sin(x)", 0, 6.3);
	plot->Draw();
	canvas->Update();


	getchar();
}

#ifndef __CINT__
int main() {
	test();
	return(0);
}
# endif
