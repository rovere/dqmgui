#include "VisMonitoring/DQMServer/interface/DQMRenderPlugin.h"
#include "TCanvas.h"
#include "TText.h"
#include "TColor.h"
#include <cstring>

class TestDQMRenderPlugin : public DQMRenderPlugin
{
public:
  virtual bool applies(const VisDQMObject &o, const VisDQMImgInfo &)
  {
    return strncmp(o.name.c_str(), "YourSubsystemName/", 18) == 0;
  }

  virtual void preDraw(TCanvas *c, const VisDQMObject &,
		       const VisDQMImgInfo &, VisDQMRenderInfo &)
  {
    c->SetFrameFillColor(TColor::GetColor(255, 255, 224));
  }

  virtual void postDraw(TCanvas *, const VisDQMObject &, const VisDQMImgInfo &)
  {
    TText *t = new TText(.5, .25, "This is a test");
    t->SetNDC(kTRUE);
    t->SetTextFont(63);
    t->SetTextSize(16);
    t->SetTextAlign(22);
    t->SetTextColor(TColor::GetColor(255, 124, 124));
    t->Draw();
  }
};

static TestDQMRenderPlugin instance;
