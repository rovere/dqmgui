#ifndef DQM_VISDQMRENDERTOOLS_H
#define DQM_VISDQMRENDERTOOLS_H

#include "classlib/utils/StringFormat.h"
#include "boost/algorithm/string.hpp"
#include "TProfile2D.h"
#include "TBufferJSON.h"

using lat::StringFormat;
using std::string;

static void replacePseudoNumericValues(string& json)
{
  boost::replace_all(json, "':nan,", "':'nan',");
  boost::replace_all(json, "':nan}", "':'nan'}");
  boost::replace_all(json, "':inf,", "':'inf',");
  boost::replace_all(json, "':inf}", "':'inf'}");
  boost::replace_all(json, "':-inf,", "':'-inf',");
  boost::replace_all(json, "':-inf}", "':'-inf'}");
  boost::replace_all(json, ",inf", ",'inf'");
  boost::replace_all(json, ",-inf", ",'-inf'");
}

static string arrayToJson(const string arrayString, const char* const name = NULL)
{
  string result;
  // if there is useless comma at the end then remove it
  if (arrayString.at(arrayString.size() - 1) == ',')
    result = arrayString.substr(0, arrayString.size() - 1);
  else
    result = arrayString;
  if (name)
    return StringFormat("'%1':[%2]")
        .arg(name)
        .arg(result);
  else
    return "[" + result + "]";
}


static string integralToJson(Double_t integral[][3] )
{
  return StringFormat("'integral':["
                      "[%7,%8,%9],"
                      "[%4,%5,%6],"
                      "[%1,%2,%3]"
                      "]")
      .arg(integral[0][0], 0, 'g')
      .arg(integral[0][1], 0, 'g')
      .arg(integral[0][2], 0, 'g')
      .arg(integral[1][0], 0, 'g')
      .arg(integral[1][1], 0, 'g')
      .arg(integral[1][2], 0, 'g')
      .arg(integral[2][0], 0, 'g')
      .arg(integral[2][1], 0, 'g')
      .arg(integral[2][2], 0, 'g');
}

/* The following methods return a json containing all bins data and due
   to code optimization also part of statistics: the integral. */

static string binsToArray(const TH1* const h)
{
  string contentList = "";
  string widthList = "";
  string errorList = "";
  Double_t sum = 0;
  const Double_t defWidth = (h->GetXaxis()->GetXmax()
                             - h->GetXaxis()->GetXmin())/h->GetNbinsX();
  bool isWidthDef = true;
  for (int i = h->GetXaxis()->GetFirst();
       i != h->GetXaxis()->GetLast() + 1; ++i)
  {
    if(isWidthDef && (h->GetXaxis()->GetBinWidth(i) != defWidth))
      isWidthDef = false;
    sum += h->GetBinContent(i);
    contentList += StringFormat("%1,")
        .arg((double)h->GetBinContent(i), 14, 'g', 11);
    widthList += StringFormat("%1,")
        .arg(h->GetXaxis()->GetBinWidth(i));
    errorList += StringFormat("%1,")
        .arg(h->GetBinError(i));
  }
  string integral = StringFormat("'integral':%1").arg(sum);
  contentList = arrayToJson(contentList, "content");
  errorList = arrayToJson(errorList, "error");
  if (isWidthDef)
    return integral + "," + contentList + ","+ errorList;
  else
    return integral + "," + contentList + ","
        + arrayToJson(widthList, "width")+ ","+ errorList;
}

static string binsToArray(const TH2* const h)
{
  string content = "";
  string widthX  = "";
  string widthY  = "";
  Double_t sum[3][3];
  bzero((void *)sum, 9*sizeof(Double_t));
  const Int_t Xlast = h->GetXaxis()->GetLast(), Ylast = h->GetYaxis()->GetLast();
  /** Naming conventions:
      ux - underflow X, uy = underflow Y
      ox - overflow X,  oy = oversflow Y */
  sum[0][0] = h->GetBinContent(0,0); // ux uy
  sum[0][1] = 0;                     // sum(1..Ylast) ux
  sum[0][2] = h->GetBinContent(0, Ylast + 1); // ux,oy
  sum[1][0] = 0; // sum(1..Xlast) uy
  sum[1][1] = 0; // integral
  sum[1][2] = 0; // sum(1..Xlast) oy
  sum[2][0] = h->GetBinContent(Xlast + 1,0); // ox,uy
  sum[2][1] = 0; // sum(1..Ylast) ox
  sum[2][2] = h->GetBinContent(Xlast + 1, Ylast + 1);

  const Double_t defWidthX = (h->GetXaxis()->GetXmax() - h->GetXaxis()->GetXmin())
                             / h->GetNbinsX();
  const Double_t defWidthY = (h->GetYaxis()->GetXmax() - h->GetYaxis()->GetXmin())
                             / h->GetNbinsY();
  bool isWidthXDef = true;
  bool isWidthYDef = true;

  for (int i = h->GetXaxis()->GetFirst(); i != Xlast+1; ++i)
  {
    sum[1][0] += h->GetBinContent(i, 0);  // sum(1..Xlast) uy
    sum[1][2] += h->GetBinContent(i, Ylast + 1);  // sum(1..Xlast) oy
    widthX += StringFormat("%1,").arg(h->GetXaxis()->GetBinWidth(i));
    if(isWidthXDef && (h->GetXaxis()->GetBinWidth(i) != defWidthX))
      isWidthXDef = false;
  }

  for (int j = h->GetYaxis()->GetFirst(); j != Ylast+1; ++j)
  {
    string subcontent = "";
    sum[0][1] += h->GetBinContent(0, j);        // sum(1..Ylast) ux
    sum[2][1] += h->GetBinContent(Xlast + 1, j); // sum(1..Ylast) ox
    widthY += StringFormat("%1,").arg(h->GetYaxis()->GetBinWidth(j));
    if(isWidthYDef && (h->GetYaxis()->GetBinWidth(j) != defWidthY))
      isWidthYDef = false;
    for (int i = h->GetXaxis()->GetFirst(); i != Xlast + 1; ++i)
    {
      const Double_t binValue = h->GetBinContent(i, j);
      sum[1][1]  += binValue;  //integral
      subcontent += StringFormat("%1,").arg(binValue);
    }
    content += arrayToJson(subcontent) + ",";
  }
  content = arrayToJson(content, "content");
  string widthString = "";
  if(!isWidthXDef)
    widthString += "," + arrayToJson(widthX, "widthX");
  if(!isWidthYDef)
    widthString += "," + arrayToJson(widthY, "widthY");
  return integralToJson(sum) + "," + content + widthString;
}

template <typename T>
static string errorCodeToJson(const T* const tprof)
{
  const char* const value = tprof->GetErrorOption();
  switch (value[0])
  {
    case 's':
    case 'i':
    case 'g':
      return StringFormat(",'errorStyle':'%1'").arg(value[0]);
  }
  return StringFormat(",'errorStyle':'unrecognized error code: %1'")
      .arg(value);
}

template <typename T>
static string optionalNumericValueToJson(const char* const name,
                                         T value,
                                         T defaultValue = 0,
                                         bool avoidComma = false)
{
  if(value == defaultValue)
    return "";
  return StringFormat("%1'%2':%3")
      .arg(avoidComma ? "" : ",")
      .arg(name)
      .arg(value);
}

template <typename T>
static string optionalTextValueToJson(const char* const name,
                                      T value,
                                      T defaultValue = "",
                                      bool avoidComma = false)
{
  if (!value || value[0] == '\0' || value == defaultValue)
    return "";
  return StringFormat("%1'%2':'%3'")
      .arg(avoidComma ? "" : ",")
      .arg(name)
      .arg(value);
}

static string axisDataToJson(const TAxis* const h)
{
  string result = "";
  result += optionalTextValueToJson<const char* const>("title",
                                                       h->GetTitle(),
                                                       NULL,
                                                       true);
  if (h->GetNbins() > 1)
  {
    if (result != "")
      result +=",";
    result += StringFormat(
        "'first':{'id':%1,'value':%2}"
        ",'last': {'id':%3,'value':%4}"
        )
        .arg(h->GetFirst())
        .arg(h->GetXmin())
        .arg(h->GetLast())
        .arg(h->GetXmax());
  }
  string labels = "";
  for (int i = h->GetFirst(); i != h->GetLast() + 1; ++i)
  {
    const char* const label = h->GetBinLabel(i);
    if (!label || label[0] == '\0')
      continue;
    labels += StringFormat("{'center': %1,'value': '%2'},")
        .arg(h->GetBinCenter(i))
        .arg(label);
  }
  if (labels.size() > 0)
  {
    if (result != "")
      result += ",";
    result += arrayToJson(labels, "labels");
  }
  if (result != "")
    return StringFormat(",'%1':{%2}")
        .arg(h->GetName())
        .arg(result);
  else
    return "";
}

static string statWithErrorToJson(const TH1* const hist, const char* const name)
{
  const Int_t X = 1/*, Y = 2, Z = 3*/;
  const Int_t X_ERROR = 101/*, Y_ERROR = 102, Z_ERROR = 103*/;
  const char* const format = ""
      "'X':{"
      "'value':%1"
      ",'error':%2"
      "}";
  if (strcmp(name,"mean") == 0) {
    if (std::isfinite(hist->GetMean(X)) && std::isfinite(hist->GetMeanError(X)))
      return StringFormat(format)
          .arg(hist->GetMean(X))
          .arg(hist->GetMeanError(X_ERROR));
    if (!std::isfinite(hist->GetMean(X))) {
      if (!std::isfinite(hist->GetMeanError(X_ERROR))) {
        return StringFormat(format)
            .arg("'NaN'")
            .arg("'NaN");
      } else {
        return StringFormat(format)
            .arg("'NaN'")
            .arg(hist->GetMeanError(X_ERROR));
      }
    }
    return StringFormat(format)
        .arg(hist->GetMean(X))
        .arg("'NaN");
  }
  if (strcmp(name,"rms") == 0) {
    if (std::isfinite(hist->GetRMS(X)) && std::isfinite(hist->GetRMSError(X_ERROR)))
      return StringFormat(format)
          .arg(hist->GetRMS(X))
          .arg(hist->GetRMSError(X_ERROR));
    if (!std::isfinite(hist->GetRMS(X))) {
      if (!std::isfinite(hist->GetRMSError(X_ERROR))) {
        return StringFormat(format)
            .arg("'NaN'")
            .arg("'NaN");
      } else {
        return StringFormat(format)
            .arg("'NaN'")
            .arg(hist->GetRMSError(X_ERROR));
      }
    }
    return StringFormat(format)
        .arg(hist->GetRMS(X))
        .arg("'NaN");
  }
  if (strcmp(name,"kurtosis") == 0) {
    if (std::isfinite(hist->GetKurtosis(X)) && std::isfinite(hist->GetKurtosis(X_ERROR)))
      return StringFormat(format)
          .arg(hist->GetKurtosis(X))
          .arg(hist->GetKurtosis(X_ERROR));
    if (!std::isfinite(hist->GetKurtosis(X))) {
      if (!std::isfinite(hist->GetKurtosis(X_ERROR))) {
        return StringFormat(format)
            .arg("'NaN'")
            .arg("'NaN");
      } else {
        return StringFormat(format)
            .arg("'NaN'")
            .arg(hist->GetKurtosis(X_ERROR));
      }
    }
    return StringFormat(format)
        .arg(hist->GetKurtosis(X))
        .arg("'NaN");
  }
  if (strcmp(name,"skewness") == 0) {
    if (std::isfinite(hist->GetSkewness(X)) && std::isfinite(hist->GetSkewness(X_ERROR)))
      return StringFormat(format)
          .arg(hist->GetSkewness(X))
          .arg(hist->GetSkewness(X_ERROR));
    if (!std::isfinite(hist->GetSkewness(X))) {
      if (!std::isfinite(hist->GetSkewness(X_ERROR))) {
        return StringFormat(format)
            .arg("'NaN'")
            .arg("'NaN");
      } else {
        return StringFormat(format)
            .arg("'NaN'")
            .arg(hist->GetSkewness(X_ERROR));
      }
    }
    return StringFormat(format)
        .arg(hist->GetSkewness(X))
        .arg("'NaN");
  }

  // name not recognized
  return StringFormat("'%1':'has not been recognised'")
      .arg(name);
}

static string statWithErrorToJson(const TH2* const hist, const char* const name)
{
  const Int_t X = 1, Y = 2 /*, Z = 3*/;
  const Int_t X_ERROR = 101, Y_ERROR = 102 /*, Z_ERROR = 103*/;
  const char* const format = ""
      "'X':{"
      "'value':%1"
      ",'error':%2"
      "},"
      "'Y':{"
      "'value':%3"
      ",'error':%4"
      "}";
  if (strcmp(name,"mean") == 0)
    return StringFormat(format)
        .arg(hist->GetMean(X))
        .arg(hist->GetMeanError(X))
        .arg(hist->GetMean(Y))
        .arg(hist->GetMeanError(Y));
  if (strcmp(name,"rms") == 0)
    return StringFormat(format)
        .arg(hist->GetRMS(X))
        .arg(hist->GetRMSError(X))
        .arg(hist->GetRMS(Y))
        .arg(hist->GetRMSError(Y));
  if (strcmp(name,"kurtosis") == 0)
    return StringFormat(format)
        .arg(hist->GetKurtosis(X))
        .arg(hist->GetKurtosis(X_ERROR))
        .arg(hist->GetKurtosis(Y))
        .arg(hist->GetKurtosis(Y_ERROR));
  if (strcmp(name,"skewness") == 0)
    return StringFormat(format)
        .arg(hist->GetSkewness(X))
        .arg(hist->GetSkewness(X_ERROR))
        .arg(hist->GetSkewness(Y))
        .arg(hist->GetSkewness(Y_ERROR));
  return StringFormat("'%1':'has not been recognised'")
      .arg(name);
}

static string rootObjectToJson(const TObject* const h)
{
  return TBufferJSON::ConvertToJSON(h).Data();
}

template<typename T>
static string statsToJson(const T* const hist)
{
  string result ="";
  result += StringFormat(""
                         "'name':'%1'"
                         ",'entries':%2"
                         ",'mean':{%3}"
                         ",'rms':{%4}"
                         ",'underflow':%5"
                         ",'overflow':%6")
      .arg(hist->GetName())
      .arg(hist->GetEntries(), 0, 'f')
      .arg(statWithErrorToJson(hist, "mean"))
      .arg(statWithErrorToJson(hist, "rms"))
      .arg(hist->GetBinContent(0))
      .arg(hist->GetBinContent(hist->GetXaxis()->GetLast() + 1));
  return result;
}

#endif //DQM_VISDQMRENDERTOOLS_H
