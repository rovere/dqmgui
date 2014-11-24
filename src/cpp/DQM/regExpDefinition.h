#ifndef DQM_REGEXPDEFINITION_H
#define DQM_REGEXPDEFINITION_H

static Regexp RX_THOUSANDS("(\\d)(\\d{3}($|\\D))");
static Regexp RX_OUTER_WHITE("^\\s+|\\s+$");
static Regexp RX_CMSSW_VERSION("CMSSW(?:_[0-9])+(?:_pre[0-9]+)?");

#endif // DQM_REGEXPDEFINITION_H
