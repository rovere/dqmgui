def l1tlayout(i, p, *rows): i["L1TMonitor/Layouts/" + p] = DQMItem(layout=rows)

l1tlayout(dqmitems, "Summary/GT Decision bit correlations",
  ["L1TMonitor/L1TGT/GT decision bit correlation"])

l1tlayout(dqmitems, "Summary/GT FE Bx",
  ["L1TMonitor/L1TGT/GT FE Bx"])

l1tlayout(dqmitems, "Summary/GT decision bits",
  ["L1TMonitor/L1TGT/GT decision bits"])

l1tlayout(dqmitems, "Summary/DTTF_quality",
          ["L1TMonitor/L1TDTTF/DTTF_quality"])
l1tlayout(dqmitems,"Summary/DTTF_eta_value",
          ["L1TMonitor/L1TDTTF/DTTF_eta_value"])
l1tlayout(dqmitems, "Summary/DTTF_phi_value",
          ["L1TMonitor/L1TDTTF/DTTF_phi_value"])

l1tlayout(dqmitems, "Summary/DTTF_ntrack",
          ["L1TMonitor/L1TDTTF/DTTF_ntrack"])
