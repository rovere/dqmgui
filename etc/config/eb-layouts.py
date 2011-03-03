def eblayout(i, p, *rows): i["EcalBarrel/Layouts/" + p] = DQMItem(layout=rows)

eblayout(dqmitems, "Summary/0-Global-Summary",
  ["EcalBarrel/EBSummaryClient/EB global summary"])

eblayout(dqmitems, "Summary/1-Integrity-Summary",
  ["EcalBarrel/EBSummaryClient/EBIT integrity quality summary"])

eblayout(dqmitems, "Summary/2-Occupancy-Summary",
  ["EcalBarrel/EBSummaryClient/EBOT occupancy summary"])

eblayout(dqmitems, "Summary/3-Cosmic-Summary",
  ["EcalBarrel/EBSummaryClient/EBCT cosmic quality summary"])

eblayout(dqmitems, "Summary/4-PedestalOnline-Summary",
  ["EcalBarrel/EBSummaryClient/EBPOT pedestal quality summary G12"])

eblayout(dqmitems, "Summary/5-LaserL1-Summary",
  ["EcalBarrel/EBSummaryClient/EBLT laser quality summary L1"])

eblayout(dqmitems, "Integrity/0-Integrity",
  ["EcalBarrel/EBIntegrityTask/EBIT DCC size error"])

eblayout(dqmitems, "Integrity/EB-01/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-01"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-01"])

eblayout(dqmitems, "Integrity/EB-01/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-01"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-01"])

eblayout(dqmitems, "Integrity/EB-01/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-01"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-01"])

eblayout(dqmitems, "Integrity/EB-01/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-01"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-01"])

eblayout(dqmitems, "Integrity/EB-01/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-01"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-01"])

eblayout(dqmitems, "Integrity/EB-01/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-01"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-01"])

eblayout(dqmitems, "Integrity/EB-01/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-01"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-01"])

eblayout(dqmitems, "PedestalOnline/EB-01/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-01"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-01",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-01"])

eblayout(dqmitems, "Pedestal/EB-01/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-01"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-01",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-01"]),

eblayout(dqmitems, "Pedestal/EB-01/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-01"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-01",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-01"]),

eblayout(dqmitems, "Pedestal/EB-01/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-01"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-01",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-01"]),

eblayout(dqmitems, "Pedestal/EB-01/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-01"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-01"])

eblayout(dqmitems, "Pedestal/EB-01/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-01 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-01 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-01 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-01 G16"])

eblayout(dqmitems, "TestPulse/EB-01/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-01"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-01",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-01 G01"])

eblayout(dqmitems, "TestPulse/EB-01/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-01"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-01",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-01 G06"])

eblayout(dqmitems, "TestPulse/EB-01/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-01"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-01",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-01 G12"])

eblayout(dqmitems, "TestPulse/EB-01/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-01"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-01"])

eblayout(dqmitems, "TestPulse/EB-01/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-01 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-01 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-01 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-01/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-01 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-01 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-01 G16",
   None])

eblayout(dqmitems, "Laser/EB-01/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-01"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-01"])

eblayout(dqmitems, "Laser/EB-01/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-01 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-01 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-01 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-01 G16"])

eblayout(dqmitems, "Laser/EB-01/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-01",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-01"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-01",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-01"])

eblayout(dqmitems, "Laser/EB-01/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-01",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-01 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-01",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-01 L1B"])

eblayout(dqmitems, "Laser/EB-01/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-01 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-01 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-01 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-01/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-01 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-01 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-01 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-01/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-01",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-01"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-01",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-01"])

eblayout(dqmitems, "Laser/EB-01/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-01",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-01 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-01",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-01 L4B"])

eblayout(dqmitems, "Laser/EB-01/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-01 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-01 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-01 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-01/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-01 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-01 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-01 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-01/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-01"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-01"])

eblayout(dqmitems, "Timing/EB-01/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-01"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-01",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-01"])

eblayout(dqmitems, "Integrity/EB-02/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-02"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-02"])

eblayout(dqmitems, "Integrity/EB-02/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-02"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-02"])

eblayout(dqmitems, "Integrity/EB-02/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-02"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-02"])

eblayout(dqmitems, "Integrity/EB-02/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-02"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-02"])

eblayout(dqmitems, "Integrity/EB-02/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-02"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-02"])

eblayout(dqmitems, "Integrity/EB-02/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-02"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-02"])

eblayout(dqmitems, "Integrity/EB-02/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-02"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-02"])

eblayout(dqmitems, "PedestalOnline/EB-02/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-02"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-02",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-02"])

eblayout(dqmitems, "Pedestal/EB-02/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-02"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-02",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-02"]),

eblayout(dqmitems, "Pedestal/EB-02/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-02"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-02",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-02"]),

eblayout(dqmitems, "Pedestal/EB-02/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-02"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-02",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-02"]),

eblayout(dqmitems, "Pedestal/EB-02/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-02"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-02"])

eblayout(dqmitems, "Pedestal/EB-02/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-02 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-02 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-02 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-02 G16"])

eblayout(dqmitems, "TestPulse/EB-02/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-02"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-02",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-02 G01"])

eblayout(dqmitems, "TestPulse/EB-02/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-02"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-02",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-02 G06"])

eblayout(dqmitems, "TestPulse/EB-02/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-02"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-02",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-02 G12"])

eblayout(dqmitems, "TestPulse/EB-02/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-02"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-02"])

eblayout(dqmitems, "TestPulse/EB-02/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-02 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-02 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-02 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-02/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-02 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-02 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-02 G16",
   None])

eblayout(dqmitems, "Laser/EB-02/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-02"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-02"])

eblayout(dqmitems, "Laser/EB-02/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-02 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-02 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-02 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-02 G16"])

eblayout(dqmitems, "Laser/EB-02/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-02",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-02"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-02",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-02"])

eblayout(dqmitems, "Laser/EB-02/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-02",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-02 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-02",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-02 L1B"])

eblayout(dqmitems, "Laser/EB-02/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-02 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-02 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-02 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-02/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-02 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-02 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-02 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-02/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-02",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-02"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-02",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-02"])

eblayout(dqmitems, "Laser/EB-02/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-02",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-02 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-02",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-02 L4B"])

eblayout(dqmitems, "Laser/EB-02/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-02 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-02 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-02 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-02/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-02 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-02 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-02 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-02/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-02"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-02"])

eblayout(dqmitems, "Timing/EB-02/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-02"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-02",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-02"])

eblayout(dqmitems, "Integrity/EB-03/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-03"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-03"])

eblayout(dqmitems, "Integrity/EB-03/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-03"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-03"])

eblayout(dqmitems, "Integrity/EB-03/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-03"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-03"])

eblayout(dqmitems, "Integrity/EB-03/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-03"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-03"])

eblayout(dqmitems, "Integrity/EB-03/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-03"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-03"])

eblayout(dqmitems, "Integrity/EB-03/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-03"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-03"])

eblayout(dqmitems, "Integrity/EB-03/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-03"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-03"])

eblayout(dqmitems, "PedestalOnline/EB-03/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-03"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-03",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-03"])

eblayout(dqmitems, "Pedestal/EB-03/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-03"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-03",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-03"]),

eblayout(dqmitems, "Pedestal/EB-03/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-03"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-03",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-03"]),

eblayout(dqmitems, "Pedestal/EB-03/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-03"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-03",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-03"]),

eblayout(dqmitems, "Pedestal/EB-03/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-03"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-03"])

eblayout(dqmitems, "Pedestal/EB-03/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-03 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-03 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-03 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-03 G16"])

eblayout(dqmitems, "TestPulse/EB-03/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-03"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-03",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-03 G01"])

eblayout(dqmitems, "TestPulse/EB-03/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-03"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-03",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-03 G06"])

eblayout(dqmitems, "TestPulse/EB-03/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-03"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-03",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-03 G12"])

eblayout(dqmitems, "TestPulse/EB-03/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-03"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-03"])

eblayout(dqmitems, "TestPulse/EB-03/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-03 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-03 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-03 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-03/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-03 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-03 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-03 G16",
   None])

eblayout(dqmitems, "Laser/EB-03/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-03"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-03"])

eblayout(dqmitems, "Laser/EB-03/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-03 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-03 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-03 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-03 G16"])

eblayout(dqmitems, "Laser/EB-03/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-03",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-03"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-03",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-03"])

eblayout(dqmitems, "Laser/EB-03/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-03",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-03 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-03",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-03 L1B"])

eblayout(dqmitems, "Laser/EB-03/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-03 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-03 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-03 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-03/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-03 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-03 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-03 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-03/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-03",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-03"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-03",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-03"])

eblayout(dqmitems, "Laser/EB-03/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-03",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-03 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-03",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-03 L4B"])

eblayout(dqmitems, "Laser/EB-03/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-03 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-03 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-03 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-03/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-03 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-03 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-03 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-03/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-03"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-03"])

eblayout(dqmitems, "Timing/EB-03/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-03"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-03",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-03"])

eblayout(dqmitems, "Integrity/EB-04/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-04"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-04"])

eblayout(dqmitems, "Integrity/EB-04/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-04"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-04"])

eblayout(dqmitems, "Integrity/EB-04/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-04"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-04"])

eblayout(dqmitems, "Integrity/EB-04/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-04"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-04"])

eblayout(dqmitems, "Integrity/EB-04/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-04"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-04"])

eblayout(dqmitems, "Integrity/EB-04/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-04"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-04"])

eblayout(dqmitems, "Integrity/EB-04/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-04"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-04"])

eblayout(dqmitems, "PedestalOnline/EB-04/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-04"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-04",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-04"])

eblayout(dqmitems, "Pedestal/EB-04/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-04"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-04",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-04"]),

eblayout(dqmitems, "Pedestal/EB-04/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-04"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-04",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-04"]),

eblayout(dqmitems, "Pedestal/EB-04/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-04"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-04",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-04"]),

eblayout(dqmitems, "Pedestal/EB-04/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-04"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-04"])

eblayout(dqmitems, "Pedestal/EB-04/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-04 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-04 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-04 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-04 G16"])

eblayout(dqmitems, "TestPulse/EB-04/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-04"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-04",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-04 G01"])

eblayout(dqmitems, "TestPulse/EB-04/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-04"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-04",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-04 G06"])

eblayout(dqmitems, "TestPulse/EB-04/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-04"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-04",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-04 G12"])

eblayout(dqmitems, "TestPulse/EB-04/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-04"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-04"])

eblayout(dqmitems, "TestPulse/EB-04/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-04 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-04 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-04 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-04/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-04 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-04 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-04 G16",
   None])

eblayout(dqmitems, "Laser/EB-04/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-04"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-04"])

eblayout(dqmitems, "Laser/EB-04/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-04 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-04 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-04 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-04 G16"])

eblayout(dqmitems, "Laser/EB-04/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-04",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-04"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-04",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-04"])

eblayout(dqmitems, "Laser/EB-04/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-04",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-04 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-04",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-04 L1B"])

eblayout(dqmitems, "Laser/EB-04/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-04 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-04 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-04 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-04/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-04 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-04 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-04 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-04/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-04",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-04"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-04",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-04"])

eblayout(dqmitems, "Laser/EB-04/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-04",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-04 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-04",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-04 L4B"])

eblayout(dqmitems, "Laser/EB-04/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-04 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-04 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-04 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-04/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-04 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-04 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-04 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-04/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-04"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-04"])

eblayout(dqmitems, "Timing/EB-04/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-04"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-04",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-04"])

eblayout(dqmitems, "Integrity/EB-05/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-05"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-05"])

eblayout(dqmitems, "Integrity/EB-05/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-05"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-05"])

eblayout(dqmitems, "Integrity/EB-05/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-05"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-05"])

eblayout(dqmitems, "Integrity/EB-05/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-05"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-05"])

eblayout(dqmitems, "Integrity/EB-05/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-05"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-05"])

eblayout(dqmitems, "Integrity/EB-05/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-05"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-05"])

eblayout(dqmitems, "Integrity/EB-05/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-05"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-05"])

eblayout(dqmitems, "PedestalOnline/EB-05/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-05"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-05",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-05"])

eblayout(dqmitems, "Pedestal/EB-05/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-05"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-05",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-05"]),

eblayout(dqmitems, "Pedestal/EB-05/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-05"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-05",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-05"]),

eblayout(dqmitems, "Pedestal/EB-05/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-05"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-05",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-05"]),

eblayout(dqmitems, "Pedestal/EB-05/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-05"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-05"])

eblayout(dqmitems, "Pedestal/EB-05/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-05 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-05 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-05 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-05 G16"])

eblayout(dqmitems, "TestPulse/EB-05/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-05"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-05",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-05 G01"])

eblayout(dqmitems, "TestPulse/EB-05/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-05"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-05",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-05 G06"])

eblayout(dqmitems, "TestPulse/EB-05/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-05"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-05",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-05 G12"])

eblayout(dqmitems, "TestPulse/EB-05/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-05"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-05"])

eblayout(dqmitems, "TestPulse/EB-05/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-05 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-05 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-05 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-05/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-05 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-05 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-05 G16",
   None])

eblayout(dqmitems, "Laser/EB-05/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-05"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-05"])

eblayout(dqmitems, "Laser/EB-05/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-05 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-05 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-05 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-05 G16"])

eblayout(dqmitems, "Laser/EB-05/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-05",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-05"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-05",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-05"])

eblayout(dqmitems, "Laser/EB-05/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-05",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-05 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-05",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-05 L1B"])

eblayout(dqmitems, "Laser/EB-05/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-05 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-05 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-05 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-05/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-05 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-05 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-05 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-05/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-05",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-05"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-05",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-05"])

eblayout(dqmitems, "Laser/EB-05/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-05",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-05 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-05",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-05 L4B"])

eblayout(dqmitems, "Laser/EB-05/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-05 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-05 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-05 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-05/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-05 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-05 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-05 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-05/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-05"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-05"])

eblayout(dqmitems, "Timing/EB-05/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-05"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-05",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-05"])

eblayout(dqmitems, "Integrity/EB-06/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-06"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-06"])

eblayout(dqmitems, "Integrity/EB-06/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-06"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-06"])

eblayout(dqmitems, "Integrity/EB-06/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-06"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-06"])

eblayout(dqmitems, "Integrity/EB-06/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-06"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-06"])

eblayout(dqmitems, "Integrity/EB-06/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-06"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-06"])

eblayout(dqmitems, "Integrity/EB-06/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-06"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-06"])

eblayout(dqmitems, "Integrity/EB-06/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-06"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-06"])

eblayout(dqmitems, "PedestalOnline/EB-06/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-06"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-06",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-06"])

eblayout(dqmitems, "Pedestal/EB-06/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-06"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-06",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-06"]),

eblayout(dqmitems, "Pedestal/EB-06/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-06"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-06",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-06"]),

eblayout(dqmitems, "Pedestal/EB-06/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-06"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-06",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-06"]),

eblayout(dqmitems, "Pedestal/EB-06/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-06"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-06"])

eblayout(dqmitems, "Pedestal/EB-06/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-06 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-06 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-06 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-06 G16"])

eblayout(dqmitems, "TestPulse/EB-06/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-06"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-06",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-06 G01"])

eblayout(dqmitems, "TestPulse/EB-06/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-06"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-06",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-06 G06"])

eblayout(dqmitems, "TestPulse/EB-06/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-06"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-06",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-06 G12"])

eblayout(dqmitems, "TestPulse/EB-06/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-06"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-06"])

eblayout(dqmitems, "TestPulse/EB-06/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-06 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-06 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-06 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-06/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-06 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-06 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-06 G16",
   None])

eblayout(dqmitems, "Laser/EB-06/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-06"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-06"])

eblayout(dqmitems, "Laser/EB-06/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-06 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-06 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-06 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-06 G16"])

eblayout(dqmitems, "Laser/EB-06/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-06",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-06"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-06",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-06"])

eblayout(dqmitems, "Laser/EB-06/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-06",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-06 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-06",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-06 L1B"])

eblayout(dqmitems, "Laser/EB-06/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-06 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-06 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-06 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-06/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-06 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-06 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-06 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-06/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-06",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-06"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-06",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-06"])

eblayout(dqmitems, "Laser/EB-06/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-06",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-06 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-06",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-06 L4B"])

eblayout(dqmitems, "Laser/EB-06/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-06 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-06 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-06 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-06/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-06 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-06 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-06 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-06/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-06"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-06"])

eblayout(dqmitems, "Timing/EB-06/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-06"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-06",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-06"])

eblayout(dqmitems, "Integrity/EB-07/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-07"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-07"])

eblayout(dqmitems, "Integrity/EB-07/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-07"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-07"])

eblayout(dqmitems, "Integrity/EB-07/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-07"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-07"])

eblayout(dqmitems, "Integrity/EB-07/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-07"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-07"])

eblayout(dqmitems, "Integrity/EB-07/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-07"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-07"])

eblayout(dqmitems, "Integrity/EB-07/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-07"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-07"])

eblayout(dqmitems, "Integrity/EB-07/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-07"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-07"])

eblayout(dqmitems, "PedestalOnline/EB-07/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-07"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-07",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-07"])

eblayout(dqmitems, "Pedestal/EB-07/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-07"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-07",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-07"]),

eblayout(dqmitems, "Pedestal/EB-07/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-07"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-07",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-07"]),

eblayout(dqmitems, "Pedestal/EB-07/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-07"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-07",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-07"]),

eblayout(dqmitems, "Pedestal/EB-07/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-07"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-07"])

eblayout(dqmitems, "Pedestal/EB-07/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-07 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-07 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-07 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-07 G16"])

eblayout(dqmitems, "TestPulse/EB-07/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-07"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-07",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-07 G01"])

eblayout(dqmitems, "TestPulse/EB-07/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-07"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-07",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-07 G06"])

eblayout(dqmitems, "TestPulse/EB-07/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-07"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-07",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-07 G12"])

eblayout(dqmitems, "TestPulse/EB-07/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-07"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-07"])

eblayout(dqmitems, "TestPulse/EB-07/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-07 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-07 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-07 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-07/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-07 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-07 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-07 G16",
   None])

eblayout(dqmitems, "Laser/EB-07/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-07"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-07"])

eblayout(dqmitems, "Laser/EB-07/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-07 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-07 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-07 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-07 G16"])

eblayout(dqmitems, "Laser/EB-07/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-07",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-07"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-07",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-07"])

eblayout(dqmitems, "Laser/EB-07/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-07",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-07 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-07",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-07 L1B"])

eblayout(dqmitems, "Laser/EB-07/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-07 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-07 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-07 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-07/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-07 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-07 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-07 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-07/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-07",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-07"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-07",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-07"])

eblayout(dqmitems, "Laser/EB-07/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-07",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-07 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-07",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-07 L4B"])

eblayout(dqmitems, "Laser/EB-07/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-07 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-07 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-07 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-07/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-07 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-07 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-07 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-07/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-07"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-07"])

eblayout(dqmitems, "Timing/EB-07/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-07"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-07",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-07"])

eblayout(dqmitems, "Integrity/EB-08/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-08"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-08"])

eblayout(dqmitems, "Integrity/EB-08/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-08"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-08"])

eblayout(dqmitems, "Integrity/EB-08/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-08"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-08"])

eblayout(dqmitems, "Integrity/EB-08/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-08"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-08"])

eblayout(dqmitems, "Integrity/EB-08/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-08"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-08"])

eblayout(dqmitems, "Integrity/EB-08/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-08"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-08"])

eblayout(dqmitems, "Integrity/EB-08/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-08"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-08"])

eblayout(dqmitems, "PedestalOnline/EB-08/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-08"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-08",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-08"])

eblayout(dqmitems, "Pedestal/EB-08/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-08"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-08",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-08"]),

eblayout(dqmitems, "Pedestal/EB-08/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-08"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-08",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-08"]),

eblayout(dqmitems, "Pedestal/EB-08/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-08"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-08",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-08"]),

eblayout(dqmitems, "Pedestal/EB-08/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-08"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-08"])

eblayout(dqmitems, "Pedestal/EB-08/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-08 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-08 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-08 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-08 G16"])

eblayout(dqmitems, "TestPulse/EB-08/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-08"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-08",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-08 G01"])

eblayout(dqmitems, "TestPulse/EB-08/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-08"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-08",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-08 G06"])

eblayout(dqmitems, "TestPulse/EB-08/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-08"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-08",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-08 G12"])

eblayout(dqmitems, "TestPulse/EB-08/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-08"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-08"])

eblayout(dqmitems, "TestPulse/EB-08/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-08 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-08 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-08 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-08/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-08 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-08 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-08 G16",
   None])

eblayout(dqmitems, "Laser/EB-08/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-08"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-08"])

eblayout(dqmitems, "Laser/EB-08/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-08 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-08 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-08 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-08 G16"])

eblayout(dqmitems, "Laser/EB-08/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-08",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-08"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-08",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-08"])

eblayout(dqmitems, "Laser/EB-08/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-08",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-08 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-08",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-08 L1B"])

eblayout(dqmitems, "Laser/EB-08/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-08 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-08 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-08 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-08/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-08 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-08 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-08 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-08/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-08",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-08"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-08",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-08"])

eblayout(dqmitems, "Laser/EB-08/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-08",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-08 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-08",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-08 L4B"])

eblayout(dqmitems, "Laser/EB-08/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-08 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-08 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-08 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-08/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-08 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-08 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-08 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-08/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-08"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-08"])

eblayout(dqmitems, "Timing/EB-08/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-08"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-08",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-08"])

eblayout(dqmitems, "Integrity/EB-09/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-09"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-09"])

eblayout(dqmitems, "Integrity/EB-09/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-09"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-09"])

eblayout(dqmitems, "Integrity/EB-09/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-09"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-09"])

eblayout(dqmitems, "Integrity/EB-09/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-09"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-09"])

eblayout(dqmitems, "Integrity/EB-09/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-09"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-09"])

eblayout(dqmitems, "Integrity/EB-09/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-09"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-09"])

eblayout(dqmitems, "Integrity/EB-09/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-09"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-09"])

eblayout(dqmitems, "PedestalOnline/EB-09/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-09"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-09",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-09"])

eblayout(dqmitems, "Pedestal/EB-09/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-09"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-09",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-09"]),

eblayout(dqmitems, "Pedestal/EB-09/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-09"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-09",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-09"]),

eblayout(dqmitems, "Pedestal/EB-09/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-09"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-09",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-09"]),

eblayout(dqmitems, "Pedestal/EB-09/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-09"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-09"])

eblayout(dqmitems, "Pedestal/EB-09/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-09 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-09 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-09 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-09 G16"])

eblayout(dqmitems, "TestPulse/EB-09/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-09"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-09",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-09 G01"])

eblayout(dqmitems, "TestPulse/EB-09/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-09"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-09",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-09 G06"])

eblayout(dqmitems, "TestPulse/EB-09/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-09"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-09",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-09 G12"])

eblayout(dqmitems, "TestPulse/EB-09/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-09"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-09"])

eblayout(dqmitems, "TestPulse/EB-09/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-09 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-09 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-09 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-09/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-09 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-09 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-09 G16",
   None])

eblayout(dqmitems, "Laser/EB-09/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-09"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-09"])

eblayout(dqmitems, "Laser/EB-09/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-09 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-09 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-09 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-09 G16"])

eblayout(dqmitems, "Laser/EB-09/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-09",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-09"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-09",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-09"])

eblayout(dqmitems, "Laser/EB-09/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-09",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-09 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-09",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-09 L1B"])

eblayout(dqmitems, "Laser/EB-09/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-09 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-09 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-09 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-09/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-09 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-09 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-09 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-09/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-09",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-09"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-09",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-09"])

eblayout(dqmitems, "Laser/EB-09/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-09",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-09 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-09",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-09 L4B"])

eblayout(dqmitems, "Laser/EB-09/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-09 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-09 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-09 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-09/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-09 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-09 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-09 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-09/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-09"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-09"])

eblayout(dqmitems, "Timing/EB-09/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-09"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-09",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-09"])

eblayout(dqmitems, "Integrity/EB-10/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-10"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-10"])

eblayout(dqmitems, "Integrity/EB-10/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-10"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-10"])

eblayout(dqmitems, "Integrity/EB-10/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-10"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-10"])

eblayout(dqmitems, "Integrity/EB-10/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-10"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-10"])

eblayout(dqmitems, "Integrity/EB-10/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-10"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-10"])

eblayout(dqmitems, "Integrity/EB-10/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-10"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-10"])

eblayout(dqmitems, "Integrity/EB-10/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-10"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-10"])

eblayout(dqmitems, "PedestalOnline/EB-10/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-10"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-10",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-10"])

eblayout(dqmitems, "Pedestal/EB-10/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-10"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-10",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-10"]),

eblayout(dqmitems, "Pedestal/EB-10/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-10"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-10",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-10"]),

eblayout(dqmitems, "Pedestal/EB-10/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-10"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-10",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-10"]),

eblayout(dqmitems, "Pedestal/EB-10/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-10"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-10"])

eblayout(dqmitems, "Pedestal/EB-10/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-10 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-10 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-10 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-10 G16"])

eblayout(dqmitems, "TestPulse/EB-10/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-10"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-10",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-10 G01"])

eblayout(dqmitems, "TestPulse/EB-10/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-10"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-10",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-10 G06"])

eblayout(dqmitems, "TestPulse/EB-10/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-10"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-10",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-10 G12"])

eblayout(dqmitems, "TestPulse/EB-10/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-10"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-10"])

eblayout(dqmitems, "TestPulse/EB-10/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-10 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-10 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-10 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-10/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-10 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-10 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-10 G16",
   None])

eblayout(dqmitems, "Laser/EB-10/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-10"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-10"])

eblayout(dqmitems, "Laser/EB-10/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-10 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-10 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-10 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-10 G16"])

eblayout(dqmitems, "Laser/EB-10/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-10",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-10"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-10",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-10"])

eblayout(dqmitems, "Laser/EB-10/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-10",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-10 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-10",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-10 L1B"])

eblayout(dqmitems, "Laser/EB-10/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-10 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-10 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-10 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-10/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-10 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-10 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-10 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-10/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-10",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-10"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-10",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-10"])

eblayout(dqmitems, "Laser/EB-10/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-10",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-10 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-10",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-10 L4B"])

eblayout(dqmitems, "Laser/EB-10/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-10 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-10 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-10 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-10/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-10 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-10 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-10 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-10/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-10"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-10"])

eblayout(dqmitems, "Timing/EB-10/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-10"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-10",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-10"])

eblayout(dqmitems, "Integrity/EB-11/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-11"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-11"])

eblayout(dqmitems, "Integrity/EB-11/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-11"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-11"])

eblayout(dqmitems, "Integrity/EB-11/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-11"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-11"])

eblayout(dqmitems, "Integrity/EB-11/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-11"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-11"])

eblayout(dqmitems, "Integrity/EB-11/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-11"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-11"])

eblayout(dqmitems, "Integrity/EB-11/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-11"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-11"])

eblayout(dqmitems, "Integrity/EB-11/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-11"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-11"])

eblayout(dqmitems, "PedestalOnline/EB-11/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-11"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-11",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-11"])

eblayout(dqmitems, "Pedestal/EB-11/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-11"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-11",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-11"]),

eblayout(dqmitems, "Pedestal/EB-11/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-11"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-11",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-11"]),

eblayout(dqmitems, "Pedestal/EB-11/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-11"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-11",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-11"]),

eblayout(dqmitems, "Pedestal/EB-11/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-11"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-11"])

eblayout(dqmitems, "Pedestal/EB-11/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-11 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-11 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-11 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-11 G16"])

eblayout(dqmitems, "TestPulse/EB-11/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-11"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-11",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-11 G01"])

eblayout(dqmitems, "TestPulse/EB-11/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-11"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-11",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-11 G06"])

eblayout(dqmitems, "TestPulse/EB-11/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-11"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-11",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-11 G12"])

eblayout(dqmitems, "TestPulse/EB-11/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-11"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-11"])

eblayout(dqmitems, "TestPulse/EB-11/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-11 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-11 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-11 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-11/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-11 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-11 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-11 G16",
   None])

eblayout(dqmitems, "Laser/EB-11/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-11"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-11"])

eblayout(dqmitems, "Laser/EB-11/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-11 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-11 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-11 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-11 G16"])

eblayout(dqmitems, "Laser/EB-11/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-11",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-11"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-11",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-11"])

eblayout(dqmitems, "Laser/EB-11/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-11",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-11 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-11",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-11 L1B"])

eblayout(dqmitems, "Laser/EB-11/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-11 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-11 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-11 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-11/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-11 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-11 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-11 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-11/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-11",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-11"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-11",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-11"])

eblayout(dqmitems, "Laser/EB-11/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-11",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-11 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-11",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-11 L4B"])

eblayout(dqmitems, "Laser/EB-11/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-11 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-11 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-11 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-11/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-11 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-11 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-11 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-11/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-11"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-11"])

eblayout(dqmitems, "Timing/EB-11/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-11"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-11",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-11"])

eblayout(dqmitems, "Integrity/EB-12/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-12"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-12"])

eblayout(dqmitems, "Integrity/EB-12/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-12"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-12"])

eblayout(dqmitems, "Integrity/EB-12/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-12"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-12"])

eblayout(dqmitems, "Integrity/EB-12/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-12"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-12"])

eblayout(dqmitems, "Integrity/EB-12/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-12"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-12"])

eblayout(dqmitems, "Integrity/EB-12/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-12"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-12"])

eblayout(dqmitems, "Integrity/EB-12/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-12"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-12"])

eblayout(dqmitems, "PedestalOnline/EB-12/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-12"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-12",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-12"])

eblayout(dqmitems, "Pedestal/EB-12/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-12"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-12",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-12"]),

eblayout(dqmitems, "Pedestal/EB-12/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-12"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-12",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-12"]),

eblayout(dqmitems, "Pedestal/EB-12/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-12"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-12",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-12"]),

eblayout(dqmitems, "Pedestal/EB-12/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-12"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-12"])

eblayout(dqmitems, "Pedestal/EB-12/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-12 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-12 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-12 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-12 G16"])

eblayout(dqmitems, "TestPulse/EB-12/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-12"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-12",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-12 G01"])

eblayout(dqmitems, "TestPulse/EB-12/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-12"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-12",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-12 G06"])

eblayout(dqmitems, "TestPulse/EB-12/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-12"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-12",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-12 G12"])

eblayout(dqmitems, "TestPulse/EB-12/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-12"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-12"])

eblayout(dqmitems, "TestPulse/EB-12/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-12 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-12 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-12 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-12/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-12 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-12 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-12 G16",
   None])

eblayout(dqmitems, "Laser/EB-12/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-12"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-12"])

eblayout(dqmitems, "Laser/EB-12/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-12 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-12 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-12 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-12 G16"])

eblayout(dqmitems, "Laser/EB-12/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-12",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-12"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-12",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-12"])

eblayout(dqmitems, "Laser/EB-12/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-12",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-12 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-12",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-12 L1B"])

eblayout(dqmitems, "Laser/EB-12/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-12 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-12 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-12 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-12/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-12 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-12 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-12 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-12/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-12",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-12"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-12",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-12"])

eblayout(dqmitems, "Laser/EB-12/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-12",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-12 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-12",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-12 L4B"])

eblayout(dqmitems, "Laser/EB-12/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-12 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-12 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-12 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-12/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-12 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-12 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-12 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-12/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-12"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-12"])

eblayout(dqmitems, "Timing/EB-12/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-12"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-12",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-12"])

eblayout(dqmitems, "Integrity/EB-13/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-13"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-13"])

eblayout(dqmitems, "Integrity/EB-13/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-13"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-13"])

eblayout(dqmitems, "Integrity/EB-13/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-13"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-13"])

eblayout(dqmitems, "Integrity/EB-13/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-13"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-13"])

eblayout(dqmitems, "Integrity/EB-13/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-13"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-13"])

eblayout(dqmitems, "Integrity/EB-13/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-13"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-13"])

eblayout(dqmitems, "Integrity/EB-13/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-13"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-13"])

eblayout(dqmitems, "PedestalOnline/EB-13/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-13"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-13",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-13"])

eblayout(dqmitems, "Pedestal/EB-13/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-13"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-13",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-13"]),

eblayout(dqmitems, "Pedestal/EB-13/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-13"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-13",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-13"]),

eblayout(dqmitems, "Pedestal/EB-13/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-13"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-13",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-13"]),

eblayout(dqmitems, "Pedestal/EB-13/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-13"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-13"])

eblayout(dqmitems, "Pedestal/EB-13/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-13 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-13 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-13 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-13 G16"])

eblayout(dqmitems, "TestPulse/EB-13/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-13"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-13",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-13 G01"])

eblayout(dqmitems, "TestPulse/EB-13/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-13"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-13",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-13 G06"])

eblayout(dqmitems, "TestPulse/EB-13/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-13"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-13",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-13 G12"])

eblayout(dqmitems, "TestPulse/EB-13/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-13"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-13"])

eblayout(dqmitems, "TestPulse/EB-13/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-13 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-13 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-13 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-13/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-13 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-13 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-13 G16",
   None])

eblayout(dqmitems, "Laser/EB-13/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-13"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-13"])

eblayout(dqmitems, "Laser/EB-13/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-13 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-13 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-13 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-13 G16"])

eblayout(dqmitems, "Laser/EB-13/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-13",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-13"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-13",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-13"])

eblayout(dqmitems, "Laser/EB-13/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-13",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-13 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-13",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-13 L1B"])

eblayout(dqmitems, "Laser/EB-13/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-13 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-13 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-13 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-13/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-13 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-13 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-13 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-13/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-13",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-13"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-13",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-13"])

eblayout(dqmitems, "Laser/EB-13/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-13",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-13 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-13",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-13 L4B"])

eblayout(dqmitems, "Laser/EB-13/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-13 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-13 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-13 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-13/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-13 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-13 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-13 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-13/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-13"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-13"])

eblayout(dqmitems, "Timing/EB-13/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-13"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-13",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-13"])

eblayout(dqmitems, "Integrity/EB-14/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-14"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-14"])

eblayout(dqmitems, "Integrity/EB-14/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-14"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-14"])

eblayout(dqmitems, "Integrity/EB-14/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-14"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-14"])

eblayout(dqmitems, "Integrity/EB-14/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-14"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-14"])

eblayout(dqmitems, "Integrity/EB-14/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-14"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-14"])

eblayout(dqmitems, "Integrity/EB-14/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-14"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-14"])

eblayout(dqmitems, "Integrity/EB-14/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-14"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-14"])

eblayout(dqmitems, "PedestalOnline/EB-14/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-14"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-14",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-14"])

eblayout(dqmitems, "Pedestal/EB-14/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-14"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-14",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-14"]),

eblayout(dqmitems, "Pedestal/EB-14/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-14"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-14",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-14"]),

eblayout(dqmitems, "Pedestal/EB-14/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-14"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-14",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-14"]),

eblayout(dqmitems, "Pedestal/EB-14/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-14"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-14"])

eblayout(dqmitems, "Pedestal/EB-14/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-14 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-14 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-14 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-14 G16"])

eblayout(dqmitems, "TestPulse/EB-14/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-14"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-14",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-14 G01"])

eblayout(dqmitems, "TestPulse/EB-14/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-14"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-14",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-14 G06"])

eblayout(dqmitems, "TestPulse/EB-14/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-14"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-14",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-14 G12"])

eblayout(dqmitems, "TestPulse/EB-14/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-14"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-14"])

eblayout(dqmitems, "TestPulse/EB-14/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-14 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-14 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-14 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-14/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-14 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-14 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-14 G16",
   None])

eblayout(dqmitems, "Laser/EB-14/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-14"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-14"])

eblayout(dqmitems, "Laser/EB-14/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-14 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-14 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-14 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-14 G16"])

eblayout(dqmitems, "Laser/EB-14/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-14",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-14"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-14",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-14"])

eblayout(dqmitems, "Laser/EB-14/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-14",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-14 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-14",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-14 L1B"])

eblayout(dqmitems, "Laser/EB-14/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-14 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-14 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-14 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-14/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-14 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-14 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-14 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-14/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-14",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-14"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-14",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-14"])

eblayout(dqmitems, "Laser/EB-14/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-14",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-14 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-14",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-14 L4B"])

eblayout(dqmitems, "Laser/EB-14/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-14 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-14 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-14 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-14/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-14 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-14 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-14 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-14/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-14"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-14"])

eblayout(dqmitems, "Timing/EB-14/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-14"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-14",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-14"])

eblayout(dqmitems, "Integrity/EB-15/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-15"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-15"])

eblayout(dqmitems, "Integrity/EB-15/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-15"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-15"])

eblayout(dqmitems, "Integrity/EB-15/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-15"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-15"])

eblayout(dqmitems, "Integrity/EB-15/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-15"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-15"])

eblayout(dqmitems, "Integrity/EB-15/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-15"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-15"])

eblayout(dqmitems, "Integrity/EB-15/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-15"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-15"])

eblayout(dqmitems, "Integrity/EB-15/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-15"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-15"])

eblayout(dqmitems, "PedestalOnline/EB-15/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-15"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-15",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-15"])

eblayout(dqmitems, "Pedestal/EB-15/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-15"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-15",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-15"]),

eblayout(dqmitems, "Pedestal/EB-15/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-15"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-15",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-15"]),

eblayout(dqmitems, "Pedestal/EB-15/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-15"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-15",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-15"]),

eblayout(dqmitems, "Pedestal/EB-15/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-15"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-15"])

eblayout(dqmitems, "Pedestal/EB-15/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-15 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-15 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-15 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-15 G16"])

eblayout(dqmitems, "TestPulse/EB-15/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-15"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-15",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-15 G01"])

eblayout(dqmitems, "TestPulse/EB-15/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-15"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-15",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-15 G06"])

eblayout(dqmitems, "TestPulse/EB-15/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-15"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-15",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-15 G12"])

eblayout(dqmitems, "TestPulse/EB-15/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-15"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-15"])

eblayout(dqmitems, "TestPulse/EB-15/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-15 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-15 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-15 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-15/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-15 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-15 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-15 G16",
   None])

eblayout(dqmitems, "Laser/EB-15/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-15"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-15"])

eblayout(dqmitems, "Laser/EB-15/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-15 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-15 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-15 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-15 G16"])

eblayout(dqmitems, "Laser/EB-15/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-15",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-15"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-15",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-15"])

eblayout(dqmitems, "Laser/EB-15/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-15",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-15 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-15",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-15 L1B"])

eblayout(dqmitems, "Laser/EB-15/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-15 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-15 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-15 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-15/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-15 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-15 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-15 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-15/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-15",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-15"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-15",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-15"])

eblayout(dqmitems, "Laser/EB-15/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-15",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-15 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-15",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-15 L4B"])

eblayout(dqmitems, "Laser/EB-15/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-15 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-15 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-15 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-15/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-15 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-15 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-15 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-15/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-15"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-15"])

eblayout(dqmitems, "Timing/EB-15/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-15"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-15",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-15"])

eblayout(dqmitems, "Integrity/EB-16/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-16"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-16"])

eblayout(dqmitems, "Integrity/EB-16/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-16"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-16"])

eblayout(dqmitems, "Integrity/EB-16/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-16"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-16"])

eblayout(dqmitems, "Integrity/EB-16/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-16"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-16"])

eblayout(dqmitems, "Integrity/EB-16/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-16"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-16"])

eblayout(dqmitems, "Integrity/EB-16/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-16"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-16"])

eblayout(dqmitems, "Integrity/EB-16/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-16"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-16"])

eblayout(dqmitems, "PedestalOnline/EB-16/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-16"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-16",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-16"])

eblayout(dqmitems, "Pedestal/EB-16/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-16"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-16",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-16"]),

eblayout(dqmitems, "Pedestal/EB-16/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-16"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-16",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-16"]),

eblayout(dqmitems, "Pedestal/EB-16/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-16"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-16",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-16"]),

eblayout(dqmitems, "Pedestal/EB-16/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-16"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-16"])

eblayout(dqmitems, "Pedestal/EB-16/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-16 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-16 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-16 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-16 G16"])

eblayout(dqmitems, "TestPulse/EB-16/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-16"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-16",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-16 G01"])

eblayout(dqmitems, "TestPulse/EB-16/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-16"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-16",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-16 G06"])

eblayout(dqmitems, "TestPulse/EB-16/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-16"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-16",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-16 G12"])

eblayout(dqmitems, "TestPulse/EB-16/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-16"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-16"])

eblayout(dqmitems, "TestPulse/EB-16/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-16 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-16 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-16 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-16/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-16 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-16 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-16 G16",
   None])

eblayout(dqmitems, "Laser/EB-16/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-16"])

eblayout(dqmitems, "Laser/EB-16/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-16 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-16 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-16 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-16 G16"])

eblayout(dqmitems, "Laser/EB-16/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-16",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-16"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-16",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-16"])

eblayout(dqmitems, "Laser/EB-16/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-16",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-16 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-16",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-16 L1B"])

eblayout(dqmitems, "Laser/EB-16/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-16 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-16 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-16 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-16/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-16 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-16 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-16 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-16/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-16",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-16"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-16",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-16"])

eblayout(dqmitems, "Laser/EB-16/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-16",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-16 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-16",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-16 L4B"])

eblayout(dqmitems, "Laser/EB-16/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-16 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-16 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-16 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-16/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-16 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-16 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-16 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-16/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-16"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-16"])

eblayout(dqmitems, "Timing/EB-16/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-16"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-16",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-16"])

eblayout(dqmitems, "Integrity/EB-17/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-17"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-17"])

eblayout(dqmitems, "Integrity/EB-17/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-17"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-17"])

eblayout(dqmitems, "Integrity/EB-17/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-17"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-17"])

eblayout(dqmitems, "Integrity/EB-17/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-17"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-17"])

eblayout(dqmitems, "Integrity/EB-17/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-17"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-17"])

eblayout(dqmitems, "Integrity/EB-17/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-17"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-17"])

eblayout(dqmitems, "Integrity/EB-17/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-17"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-17"])

eblayout(dqmitems, "PedestalOnline/EB-17/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-17"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-17",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-17"])

eblayout(dqmitems, "Pedestal/EB-17/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-17"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-17",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-17"]),

eblayout(dqmitems, "Pedestal/EB-17/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-17"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-17",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-17"]),

eblayout(dqmitems, "Pedestal/EB-17/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-17"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-17",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-17"]),

eblayout(dqmitems, "Pedestal/EB-17/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-17"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-17"])

eblayout(dqmitems, "Pedestal/EB-17/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-17 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-17 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-17 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-17 G16"])

eblayout(dqmitems, "TestPulse/EB-17/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-17"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-17",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-17 G01"])

eblayout(dqmitems, "TestPulse/EB-17/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-17"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-17",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-17 G06"])

eblayout(dqmitems, "TestPulse/EB-17/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-17"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-17",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-17 G12"])

eblayout(dqmitems, "TestPulse/EB-17/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-17"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-17"])

eblayout(dqmitems, "TestPulse/EB-17/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-17 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-17 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-17 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-17/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-17 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-17 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-17 G16",
   None])

eblayout(dqmitems, "Laser/EB-17/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-17"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-17"])

eblayout(dqmitems, "Laser/EB-17/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-17 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-17 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-17 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-17 G16"])

eblayout(dqmitems, "Laser/EB-17/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-17",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-17"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-17",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-17"])

eblayout(dqmitems, "Laser/EB-17/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-17",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-17 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-17",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-17 L1B"])

eblayout(dqmitems, "Laser/EB-17/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-17 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-17 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-17 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-17/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-17 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-17 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-17 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-17/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-17",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-17"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-17",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-17"])

eblayout(dqmitems, "Laser/EB-17/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-17",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-17 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-17",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-17 L4B"])

eblayout(dqmitems, "Laser/EB-17/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-17 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-17 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-17 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-17/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-17 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-17 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-17 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-17/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-17"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-17"])

eblayout(dqmitems, "Timing/EB-17/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-17"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-17",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-17"])

eblayout(dqmitems, "Integrity/EB-18/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB-18"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB-18"])

eblayout(dqmitems, "Integrity/EB-18/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB-18"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB-18"])

eblayout(dqmitems, "Integrity/EB-18/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB-18"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB-18"])

eblayout(dqmitems, "Integrity/EB-18/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB-18"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB-18"])

eblayout(dqmitems, "Integrity/EB-18/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB-18"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB-18"])

eblayout(dqmitems, "Integrity/EB-18/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB-18"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB-18"])

eblayout(dqmitems, "Integrity/EB-18/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB-18"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB-18"])

eblayout(dqmitems, "PedestalOnline/EB-18/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB-18"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB-18",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB-18"])

eblayout(dqmitems, "Pedestal/EB-18/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB-18"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB-18",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB-18"]),

eblayout(dqmitems, "Pedestal/EB-18/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB-18"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB-18",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB-18"]),

eblayout(dqmitems, "Pedestal/EB-18/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB-18"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB-18",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB-18"]),

eblayout(dqmitems, "Pedestal/EB-18/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB-18"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB-18"])

eblayout(dqmitems, "Pedestal/EB-18/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-18 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-18 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB-18 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB-18 G16"])

eblayout(dqmitems, "TestPulse/EB-18/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB-18"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB-18",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-18 G01"])

eblayout(dqmitems, "TestPulse/EB-18/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB-18"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB-18",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-18 G06"])

eblayout(dqmitems, "TestPulse/EB-18/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB-18"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB-18",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB-18 G12"])

eblayout(dqmitems, "TestPulse/EB-18/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB-18"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB-18"])

eblayout(dqmitems, "TestPulse/EB-18/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB-18 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-18 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB-18 G01",
   None])

eblayout(dqmitems, "TestPulse/EB-18/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB-18 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB-18 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB-18 G16",
   None])

eblayout(dqmitems, "Laser/EB-18/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB-18"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB-18"])

eblayout(dqmitems, "Laser/EB-18/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-18 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB-18 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-18 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB-18 G16"])

eblayout(dqmitems, "Laser/EB-18/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB-18",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB-18"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB-18",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB-18"])

eblayout(dqmitems, "Laser/EB-18/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB-18",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-18 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB-18",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB-18 L1B"])

eblayout(dqmitems, "Laser/EB-18/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB-18 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB-18 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB-18 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB-18/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB-18 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB-18 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB-18 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB-18/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB-18",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB-18"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB-18",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB-18"])

eblayout(dqmitems, "Laser/EB-18/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB-18",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-18 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB-18",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB-18 L4B"])

eblayout(dqmitems, "Laser/EB-18/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB-18 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB-18 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB-18 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB-18/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB-18 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB-18 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB-18 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB-18/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-18"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB-18"])

eblayout(dqmitems, "Timing/EB-18/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB-18"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB-18",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB-18"])

eblayout(dqmitems, "Integrity/EB+01/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+01"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+01"])

eblayout(dqmitems, "Integrity/EB+01/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+01"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+01"])

eblayout(dqmitems, "Integrity/EB+01/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+01"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+01"])

eblayout(dqmitems, "Integrity/EB+01/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+01"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+01"])

eblayout(dqmitems, "Integrity/EB+01/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+01"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+01"])

eblayout(dqmitems, "Integrity/EB+01/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+01"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+01"])

eblayout(dqmitems, "Integrity/EB+01/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+01"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+01"])

eblayout(dqmitems, "PedestalOnline/EB+01/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+01"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+01",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+01"])

eblayout(dqmitems, "Pedestal/EB+01/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+01"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+01",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+01"]),

eblayout(dqmitems, "Pedestal/EB+01/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+01"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+01",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+01"]),

eblayout(dqmitems, "Pedestal/EB+01/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+01"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+01",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+01"]),

eblayout(dqmitems, "Pedestal/EB+01/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+01"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+01"])

eblayout(dqmitems, "Pedestal/EB+01/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+01 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+01 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+01 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+01 G16"])

eblayout(dqmitems, "TestPulse/EB+01/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+01"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+01",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+01 G01"])

eblayout(dqmitems, "TestPulse/EB+01/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+01"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+01",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+01 G06"])

eblayout(dqmitems, "TestPulse/EB+01/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+01"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+01",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+01 G12"])

eblayout(dqmitems, "TestPulse/EB+01/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+01"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+01"])

eblayout(dqmitems, "TestPulse/EB+01/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+01 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+01 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+01 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+01/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+01 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+01 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+01 G16",
   None])

eblayout(dqmitems, "Laser/EB+01/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+01"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+01"])

eblayout(dqmitems, "Laser/EB+01/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+01 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+01 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+01 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+01 G16"])

eblayout(dqmitems, "Laser/EB+01/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+01",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+01"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+01",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+01"])

eblayout(dqmitems, "Laser/EB+01/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+01",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+01 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+01",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+01 L1B"])

eblayout(dqmitems, "Laser/EB+01/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+01 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+01 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+01 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+01/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+01 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+01 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+01 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+01/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+01",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+01"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+01",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+01"])

eblayout(dqmitems, "Laser/EB+01/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+01",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+01 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+01",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+01 L4B"])

eblayout(dqmitems, "Laser/EB+01/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+01 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+01 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+01 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+01/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+01 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+01 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+01 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+01/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+01"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+01"])

eblayout(dqmitems, "Timing/EB+01/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+01"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+01",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+01"])

eblayout(dqmitems, "Integrity/EB+02/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+02"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+02"])

eblayout(dqmitems, "Integrity/EB+02/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+02"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+02"])

eblayout(dqmitems, "Integrity/EB+02/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+02"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+02"])

eblayout(dqmitems, "Integrity/EB+02/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+02"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+02"])

eblayout(dqmitems, "Integrity/EB+02/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+02"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+02"])

eblayout(dqmitems, "Integrity/EB+02/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+02"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+02"])

eblayout(dqmitems, "Integrity/EB+02/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+02"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+02"])

eblayout(dqmitems, "PedestalOnline/EB+02/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+02"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+02",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+02"])

eblayout(dqmitems, "Pedestal/EB+02/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+02"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+02",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+02"]),

eblayout(dqmitems, "Pedestal/EB+02/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+02"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+02",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+02"]),

eblayout(dqmitems, "Pedestal/EB+02/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+02"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+02",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+02"]),

eblayout(dqmitems, "Pedestal/EB+02/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+02"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+02"])

eblayout(dqmitems, "Pedestal/EB+02/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+02 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+02 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+02 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+02 G16"])

eblayout(dqmitems, "TestPulse/EB+02/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+02"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+02",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+02 G01"])

eblayout(dqmitems, "TestPulse/EB+02/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+02"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+02",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+02 G06"])

eblayout(dqmitems, "TestPulse/EB+02/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+02"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+02",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+02 G12"])

eblayout(dqmitems, "TestPulse/EB+02/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+02"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+02"])

eblayout(dqmitems, "TestPulse/EB+02/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+02 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+02 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+02 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+02/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+02 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+02 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+02 G16",
   None])

eblayout(dqmitems, "Laser/EB+02/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+02"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+02"])

eblayout(dqmitems, "Laser/EB+02/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+02 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+02 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+02 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+02 G16"])

eblayout(dqmitems, "Laser/EB+02/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+02",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+02"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+02",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+02"])

eblayout(dqmitems, "Laser/EB+02/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+02",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+02 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+02",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+02 L1B"])

eblayout(dqmitems, "Laser/EB+02/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+02 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+02 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+02 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+02/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+02 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+02 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+02 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+02/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+02",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+02"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+02",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+02"])

eblayout(dqmitems, "Laser/EB+02/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+02",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+02 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+02",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+02 L4B"])

eblayout(dqmitems, "Laser/EB+02/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+02 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+02 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+02 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+02/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+02 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+02 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+02 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+02/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+02"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+02"])

eblayout(dqmitems, "Timing/EB+02/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+02"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+02",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+02"])

eblayout(dqmitems, "Integrity/EB+03/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+03"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+03"])

eblayout(dqmitems, "Integrity/EB+03/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+03"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+03"])

eblayout(dqmitems, "Integrity/EB+03/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+03"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+03"])

eblayout(dqmitems, "Integrity/EB+03/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+03"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+03"])

eblayout(dqmitems, "Integrity/EB+03/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+03"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+03"])

eblayout(dqmitems, "Integrity/EB+03/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+03"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+03"])

eblayout(dqmitems, "Integrity/EB+03/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+03"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+03"])

eblayout(dqmitems, "PedestalOnline/EB+03/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+03"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+03",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+03"])

eblayout(dqmitems, "Pedestal/EB+03/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+03"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+03",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+03"]),

eblayout(dqmitems, "Pedestal/EB+03/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+03"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+03",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+03"]),

eblayout(dqmitems, "Pedestal/EB+03/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+03"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+03",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+03"]),

eblayout(dqmitems, "Pedestal/EB+03/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+03"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+03"])

eblayout(dqmitems, "Pedestal/EB+03/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+03 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+03 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+03 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+03 G16"])

eblayout(dqmitems, "TestPulse/EB+03/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+03"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+03",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+03 G01"])

eblayout(dqmitems, "TestPulse/EB+03/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+03"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+03",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+03 G06"])

eblayout(dqmitems, "TestPulse/EB+03/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+03"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+03",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+03 G12"])

eblayout(dqmitems, "TestPulse/EB+03/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+03"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+03"])

eblayout(dqmitems, "TestPulse/EB+03/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+03 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+03 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+03 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+03/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+03 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+03 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+03 G16",
   None])

eblayout(dqmitems, "Laser/EB+03/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+03"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+03"])

eblayout(dqmitems, "Laser/EB+03/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+03 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+03 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+03 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+03 G16"])

eblayout(dqmitems, "Laser/EB+03/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+03",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+03"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+03",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+03"])

eblayout(dqmitems, "Laser/EB+03/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+03",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+03 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+03",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+03 L1B"])

eblayout(dqmitems, "Laser/EB+03/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+03 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+03 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+03 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+03/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+03 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+03 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+03 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+03/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+03",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+03"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+03",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+03"])

eblayout(dqmitems, "Laser/EB+03/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+03",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+03 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+03",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+03 L4B"])

eblayout(dqmitems, "Laser/EB+03/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+03 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+03 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+03 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+03/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+03 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+03 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+03 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+03/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+03"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+03"])

eblayout(dqmitems, "Timing/EB+03/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+03"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+03",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+03"])

eblayout(dqmitems, "Integrity/EB+04/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+04"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+04"])

eblayout(dqmitems, "Integrity/EB+04/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+04"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+04"])

eblayout(dqmitems, "Integrity/EB+04/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+04"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+04"])

eblayout(dqmitems, "Integrity/EB+04/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+04"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+04"])

eblayout(dqmitems, "Integrity/EB+04/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+04"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+04"])

eblayout(dqmitems, "Integrity/EB+04/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+04"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+04"])

eblayout(dqmitems, "Integrity/EB+04/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+04"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+04"])

eblayout(dqmitems, "PedestalOnline/EB+04/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+04"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+04",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+04"])

eblayout(dqmitems, "Pedestal/EB+04/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+04"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+04",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+04"]),

eblayout(dqmitems, "Pedestal/EB+04/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+04"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+04",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+04"]),

eblayout(dqmitems, "Pedestal/EB+04/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+04"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+04",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+04"]),

eblayout(dqmitems, "Pedestal/EB+04/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+04"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+04"])

eblayout(dqmitems, "Pedestal/EB+04/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+04 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+04 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+04 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+04 G16"])

eblayout(dqmitems, "TestPulse/EB+04/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+04"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+04",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+04 G01"])

eblayout(dqmitems, "TestPulse/EB+04/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+04"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+04",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+04 G06"])

eblayout(dqmitems, "TestPulse/EB+04/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+04"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+04",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+04 G12"])

eblayout(dqmitems, "TestPulse/EB+04/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+04"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+04"])

eblayout(dqmitems, "TestPulse/EB+04/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+04 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+04 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+04 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+04/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+04 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+04 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+04 G16",
   None])

eblayout(dqmitems, "Laser/EB+04/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+04"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+04"])

eblayout(dqmitems, "Laser/EB+04/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+04 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+04 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+04 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+04 G16"])

eblayout(dqmitems, "Laser/EB+04/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+04",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+04"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+04",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+04"])

eblayout(dqmitems, "Laser/EB+04/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+04",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+04 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+04",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+04 L1B"])

eblayout(dqmitems, "Laser/EB+04/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+04 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+04 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+04 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+04/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+04 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+04 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+04 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+04/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+04",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+04"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+04",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+04"])

eblayout(dqmitems, "Laser/EB+04/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+04",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+04 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+04",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+04 L4B"])

eblayout(dqmitems, "Laser/EB+04/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+04 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+04 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+04 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+04/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+04 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+04 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+04 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+04/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+04"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+04"])

eblayout(dqmitems, "Timing/EB+04/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+04"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+04",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+04"])

eblayout(dqmitems, "Integrity/EB+05/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+05"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+05"])

eblayout(dqmitems, "Integrity/EB+05/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+05"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+05"])

eblayout(dqmitems, "Integrity/EB+05/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+05"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+05"])

eblayout(dqmitems, "Integrity/EB+05/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+05"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+05"])

eblayout(dqmitems, "Integrity/EB+05/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+05"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+05"])

eblayout(dqmitems, "Integrity/EB+05/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+05"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+05"])

eblayout(dqmitems, "Integrity/EB+05/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+05"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+05"])

eblayout(dqmitems, "PedestalOnline/EB+05/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+05"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+05",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+05"])

eblayout(dqmitems, "Pedestal/EB+05/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+05"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+05",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+05"]),

eblayout(dqmitems, "Pedestal/EB+05/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+05"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+05",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+05"]),

eblayout(dqmitems, "Pedestal/EB+05/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+05"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+05",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+05"]),

eblayout(dqmitems, "Pedestal/EB+05/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+05"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+05"])

eblayout(dqmitems, "Pedestal/EB+05/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+05 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+05 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+05 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+05 G16"])

eblayout(dqmitems, "TestPulse/EB+05/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+05"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+05",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+05 G01"])

eblayout(dqmitems, "TestPulse/EB+05/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+05"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+05",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+05 G06"])

eblayout(dqmitems, "TestPulse/EB+05/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+05"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+05",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+05 G12"])

eblayout(dqmitems, "TestPulse/EB+05/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+05"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+05"])

eblayout(dqmitems, "TestPulse/EB+05/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+05 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+05 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+05 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+05/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+05 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+05 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+05 G16",
   None])

eblayout(dqmitems, "Laser/EB+05/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+05"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+05"])

eblayout(dqmitems, "Laser/EB+05/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+05 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+05 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+05 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+05 G16"])

eblayout(dqmitems, "Laser/EB+05/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+05",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+05"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+05",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+05"])

eblayout(dqmitems, "Laser/EB+05/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+05",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+05 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+05",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+05 L1B"])

eblayout(dqmitems, "Laser/EB+05/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+05 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+05 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+05 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+05/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+05 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+05 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+05 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+05/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+05",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+05"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+05",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+05"])

eblayout(dqmitems, "Laser/EB+05/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+05",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+05 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+05",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+05 L4B"])

eblayout(dqmitems, "Laser/EB+05/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+05 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+05 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+05 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+05/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+05 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+05 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+05 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+05/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+05"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+05"])

eblayout(dqmitems, "Timing/EB+05/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+05"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+05",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+05"])

eblayout(dqmitems, "Integrity/EB+06/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+06"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+06"])

eblayout(dqmitems, "Integrity/EB+06/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+06"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+06"])

eblayout(dqmitems, "Integrity/EB+06/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+06"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+06"])

eblayout(dqmitems, "Integrity/EB+06/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+06"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+06"])

eblayout(dqmitems, "Integrity/EB+06/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+06"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+06"])

eblayout(dqmitems, "Integrity/EB+06/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+06"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+06"])

eblayout(dqmitems, "Integrity/EB+06/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+06"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+06"])

eblayout(dqmitems, "PedestalOnline/EB+06/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+06"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+06",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+06"])

eblayout(dqmitems, "Pedestal/EB+06/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+06"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+06",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+06"]),

eblayout(dqmitems, "Pedestal/EB+06/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+06"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+06",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+06"]),

eblayout(dqmitems, "Pedestal/EB+06/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+06"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+06",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+06"]),

eblayout(dqmitems, "Pedestal/EB+06/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+06"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+06"])

eblayout(dqmitems, "Pedestal/EB+06/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+06 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+06 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+06 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+06 G16"])

eblayout(dqmitems, "TestPulse/EB+06/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+06"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+06",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+06 G01"])

eblayout(dqmitems, "TestPulse/EB+06/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+06"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+06",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+06 G06"])

eblayout(dqmitems, "TestPulse/EB+06/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+06"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+06",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+06 G12"])

eblayout(dqmitems, "TestPulse/EB+06/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+06"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+06"])

eblayout(dqmitems, "TestPulse/EB+06/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+06 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+06 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+06 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+06/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+06 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+06 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+06 G16",
   None])

eblayout(dqmitems, "Laser/EB+06/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+06"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+06"])

eblayout(dqmitems, "Laser/EB+06/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+06 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+06 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+06 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+06 G16"])

eblayout(dqmitems, "Laser/EB+06/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+06",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+06"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+06",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+06"])

eblayout(dqmitems, "Laser/EB+06/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+06",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+06 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+06",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+06 L1B"])

eblayout(dqmitems, "Laser/EB+06/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+06 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+06 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+06 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+06/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+06 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+06 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+06 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+06/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+06",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+06"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+06",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+06"])

eblayout(dqmitems, "Laser/EB+06/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+06",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+06 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+06",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+06 L4B"])

eblayout(dqmitems, "Laser/EB+06/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+06 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+06 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+06 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+06/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+06 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+06 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+06 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+06/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+06"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+06"])

eblayout(dqmitems, "Timing/EB+06/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+06"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+06",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+06"])

eblayout(dqmitems, "Integrity/EB+07/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+07"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+07"])

eblayout(dqmitems, "Integrity/EB+07/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+07"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+07"])

eblayout(dqmitems, "Integrity/EB+07/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+07"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+07"])

eblayout(dqmitems, "Integrity/EB+07/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+07"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+07"])

eblayout(dqmitems, "Integrity/EB+07/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+07"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+07"])

eblayout(dqmitems, "Integrity/EB+07/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+07"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+07"])

eblayout(dqmitems, "Integrity/EB+07/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+07"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+07"])

eblayout(dqmitems, "PedestalOnline/EB+07/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+07"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+07",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+07"])

eblayout(dqmitems, "Pedestal/EB+07/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+07"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+07",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+07"]),

eblayout(dqmitems, "Pedestal/EB+07/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+07"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+07",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+07"]),

eblayout(dqmitems, "Pedestal/EB+07/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+07"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+07",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+07"]),

eblayout(dqmitems, "Pedestal/EB+07/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+07"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+07"])

eblayout(dqmitems, "Pedestal/EB+07/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+07 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+07 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+07 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+07 G16"])

eblayout(dqmitems, "TestPulse/EB+07/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+07"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+07",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+07 G01"])

eblayout(dqmitems, "TestPulse/EB+07/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+07"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+07",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+07 G06"])

eblayout(dqmitems, "TestPulse/EB+07/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+07"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+07",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+07 G12"])

eblayout(dqmitems, "TestPulse/EB+07/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+07"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+07"])

eblayout(dqmitems, "TestPulse/EB+07/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+07 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+07 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+07 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+07/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+07 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+07 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+07 G16",
   None])

eblayout(dqmitems, "Laser/EB+07/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+07"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+07"])

eblayout(dqmitems, "Laser/EB+07/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+07 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+07 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+07 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+07 G16"])

eblayout(dqmitems, "Laser/EB+07/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+07",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+07"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+07",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+07"])

eblayout(dqmitems, "Laser/EB+07/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+07",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+07 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+07",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+07 L1B"])

eblayout(dqmitems, "Laser/EB+07/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+07 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+07 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+07 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+07/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+07 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+07 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+07 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+07/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+07",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+07"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+07",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+07"])

eblayout(dqmitems, "Laser/EB+07/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+07",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+07 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+07",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+07 L4B"])

eblayout(dqmitems, "Laser/EB+07/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+07 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+07 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+07 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+07/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+07 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+07 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+07 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+07/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+07"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+07"])

eblayout(dqmitems, "Timing/EB+07/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+07"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+07",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+07"])

eblayout(dqmitems, "Integrity/EB+08/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+08"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+08"])

eblayout(dqmitems, "Integrity/EB+08/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+08"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+08"])

eblayout(dqmitems, "Integrity/EB+08/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+08"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+08"])

eblayout(dqmitems, "Integrity/EB+08/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+08"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+08"])

eblayout(dqmitems, "Integrity/EB+08/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+08"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+08"])

eblayout(dqmitems, "Integrity/EB+08/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+08"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+08"])

eblayout(dqmitems, "Integrity/EB+08/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+08"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+08"])

eblayout(dqmitems, "PedestalOnline/EB+08/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+08"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+08",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+08"])

eblayout(dqmitems, "Pedestal/EB+08/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+08"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+08",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+08"]),

eblayout(dqmitems, "Pedestal/EB+08/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+08"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+08",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+08"]),

eblayout(dqmitems, "Pedestal/EB+08/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+08"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+08",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+08"]),

eblayout(dqmitems, "Pedestal/EB+08/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+08"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+08"])

eblayout(dqmitems, "Pedestal/EB+08/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+08 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+08 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+08 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+08 G16"])

eblayout(dqmitems, "TestPulse/EB+08/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+08"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+08",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+08 G01"])

eblayout(dqmitems, "TestPulse/EB+08/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+08"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+08",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+08 G06"])

eblayout(dqmitems, "TestPulse/EB+08/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+08"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+08",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+08 G12"])

eblayout(dqmitems, "TestPulse/EB+08/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+08"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+08"])

eblayout(dqmitems, "TestPulse/EB+08/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+08 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+08 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+08 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+08/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+08 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+08 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+08 G16",
   None])

eblayout(dqmitems, "Laser/EB+08/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+08"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+08"])

eblayout(dqmitems, "Laser/EB+08/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+08 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+08 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+08 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+08 G16"])

eblayout(dqmitems, "Laser/EB+08/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+08",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+08"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+08",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+08"])

eblayout(dqmitems, "Laser/EB+08/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+08",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+08 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+08",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+08 L1B"])

eblayout(dqmitems, "Laser/EB+08/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+08 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+08 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+08 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+08/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+08 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+08 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+08 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+08/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+08",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+08"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+08",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+08"])

eblayout(dqmitems, "Laser/EB+08/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+08",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+08 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+08",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+08 L4B"])

eblayout(dqmitems, "Laser/EB+08/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+08 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+08 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+08 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+08/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+08 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+08 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+08 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+08/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+08"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+08"])

eblayout(dqmitems, "Timing/EB+08/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+08"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+08",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+08"])

eblayout(dqmitems, "Integrity/EB+09/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+09"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+09"])

eblayout(dqmitems, "Integrity/EB+09/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+09"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+09"])

eblayout(dqmitems, "Integrity/EB+09/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+09"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+09"])

eblayout(dqmitems, "Integrity/EB+09/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+09"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+09"])

eblayout(dqmitems, "Integrity/EB+09/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+09"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+09"])

eblayout(dqmitems, "Integrity/EB+09/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+09"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+09"])

eblayout(dqmitems, "Integrity/EB+09/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+09"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+09"])

eblayout(dqmitems, "PedestalOnline/EB+09/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+09"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+09",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+09"])

eblayout(dqmitems, "Pedestal/EB+09/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+09"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+09",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+09"]),

eblayout(dqmitems, "Pedestal/EB+09/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+09"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+09",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+09"]),

eblayout(dqmitems, "Pedestal/EB+09/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+09"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+09",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+09"]),

eblayout(dqmitems, "Pedestal/EB+09/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+09"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+09"])

eblayout(dqmitems, "Pedestal/EB+09/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+09 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+09 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+09 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+09 G16"])

eblayout(dqmitems, "TestPulse/EB+09/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+09"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+09",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+09 G01"])

eblayout(dqmitems, "TestPulse/EB+09/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+09"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+09",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+09 G06"])

eblayout(dqmitems, "TestPulse/EB+09/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+09"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+09",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+09 G12"])

eblayout(dqmitems, "TestPulse/EB+09/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+09"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+09"])

eblayout(dqmitems, "TestPulse/EB+09/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+09 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+09 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+09 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+09/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+09 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+09 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+09 G16",
   None])

eblayout(dqmitems, "Laser/EB+09/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+09"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+09"])

eblayout(dqmitems, "Laser/EB+09/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+09 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+09 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+09 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+09 G16"])

eblayout(dqmitems, "Laser/EB+09/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+09",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+09"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+09",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+09"])

eblayout(dqmitems, "Laser/EB+09/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+09",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+09 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+09",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+09 L1B"])

eblayout(dqmitems, "Laser/EB+09/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+09 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+09 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+09 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+09/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+09 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+09 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+09 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+09/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+09",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+09"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+09",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+09"])

eblayout(dqmitems, "Laser/EB+09/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+09",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+09 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+09",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+09 L4B"])

eblayout(dqmitems, "Laser/EB+09/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+09 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+09 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+09 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+09/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+09 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+09 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+09 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+09/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+09"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+09"])

eblayout(dqmitems, "Timing/EB+09/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+09"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+09",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+09"])

eblayout(dqmitems, "Integrity/EB+10/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+10"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+10"])

eblayout(dqmitems, "Integrity/EB+10/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+10"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+10"])

eblayout(dqmitems, "Integrity/EB+10/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+10"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+10"])

eblayout(dqmitems, "Integrity/EB+10/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+10"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+10"])

eblayout(dqmitems, "Integrity/EB+10/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+10"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+10"])

eblayout(dqmitems, "Integrity/EB+10/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+10"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+10"])

eblayout(dqmitems, "Integrity/EB+10/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+10"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+10"])

eblayout(dqmitems, "PedestalOnline/EB+10/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+10"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+10",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+10"])

eblayout(dqmitems, "Pedestal/EB+10/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+10"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+10",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+10"]),

eblayout(dqmitems, "Pedestal/EB+10/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+10"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+10",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+10"]),

eblayout(dqmitems, "Pedestal/EB+10/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+10"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+10",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+10"]),

eblayout(dqmitems, "Pedestal/EB+10/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+10"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+10"])

eblayout(dqmitems, "Pedestal/EB+10/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+10 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+10 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+10 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+10 G16"])

eblayout(dqmitems, "TestPulse/EB+10/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+10"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+10",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+10 G01"])

eblayout(dqmitems, "TestPulse/EB+10/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+10"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+10",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+10 G06"])

eblayout(dqmitems, "TestPulse/EB+10/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+10"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+10",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+10 G12"])

eblayout(dqmitems, "TestPulse/EB+10/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+10"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+10"])

eblayout(dqmitems, "TestPulse/EB+10/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+10 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+10 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+10 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+10/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+10 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+10 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+10 G16",
   None])

eblayout(dqmitems, "Laser/EB+10/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+10"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+10"])

eblayout(dqmitems, "Laser/EB+10/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+10 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+10 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+10 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+10 G16"])

eblayout(dqmitems, "Laser/EB+10/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+10",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+10"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+10",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+10"])

eblayout(dqmitems, "Laser/EB+10/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+10",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+10 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+10",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+10 L1B"])

eblayout(dqmitems, "Laser/EB+10/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+10 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+10 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+10 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+10/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+10 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+10 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+10 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+10/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+10",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+10"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+10",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+10"])

eblayout(dqmitems, "Laser/EB+10/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+10",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+10 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+10",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+10 L4B"])

eblayout(dqmitems, "Laser/EB+10/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+10 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+10 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+10 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+10/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+10 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+10 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+10 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+10/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+10"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+10"])

eblayout(dqmitems, "Timing/EB+10/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+10"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+10",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+10"])

eblayout(dqmitems, "Integrity/EB+11/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+11"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+11"])

eblayout(dqmitems, "Integrity/EB+11/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+11"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+11"])

eblayout(dqmitems, "Integrity/EB+11/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+11"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+11"])

eblayout(dqmitems, "Integrity/EB+11/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+11"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+11"])

eblayout(dqmitems, "Integrity/EB+11/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+11"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+11"])

eblayout(dqmitems, "Integrity/EB+11/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+11"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+11"])

eblayout(dqmitems, "Integrity/EB+11/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+11"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+11"])

eblayout(dqmitems, "PedestalOnline/EB+11/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+11"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+11",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+11"])

eblayout(dqmitems, "Pedestal/EB+11/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+11"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+11",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+11"]),

eblayout(dqmitems, "Pedestal/EB+11/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+11"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+11",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+11"]),

eblayout(dqmitems, "Pedestal/EB+11/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+11"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+11",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+11"]),

eblayout(dqmitems, "Pedestal/EB+11/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+11"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+11"])

eblayout(dqmitems, "Pedestal/EB+11/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+11 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+11 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+11 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+11 G16"])

eblayout(dqmitems, "TestPulse/EB+11/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+11"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+11",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+11 G01"])

eblayout(dqmitems, "TestPulse/EB+11/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+11"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+11",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+11 G06"])

eblayout(dqmitems, "TestPulse/EB+11/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+11"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+11",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+11 G12"])

eblayout(dqmitems, "TestPulse/EB+11/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+11"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+11"])

eblayout(dqmitems, "TestPulse/EB+11/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+11 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+11 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+11 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+11/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+11 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+11 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+11 G16",
   None])

eblayout(dqmitems, "Laser/EB+11/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+11"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+11"])

eblayout(dqmitems, "Laser/EB+11/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+11 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+11 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+11 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+11 G16"])

eblayout(dqmitems, "Laser/EB+11/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+11",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+11"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+11",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+11"])

eblayout(dqmitems, "Laser/EB+11/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+11",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+11 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+11",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+11 L1B"])

eblayout(dqmitems, "Laser/EB+11/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+11 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+11 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+11 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+11/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+11 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+11 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+11 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+11/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+11",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+11"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+11",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+11"])

eblayout(dqmitems, "Laser/EB+11/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+11",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+11 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+11",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+11 L4B"])

eblayout(dqmitems, "Laser/EB+11/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+11 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+11 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+11 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+11/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+11 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+11 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+11 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+11/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+11"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+11"])

eblayout(dqmitems, "Timing/EB+11/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+11"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+11",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+11"])

eblayout(dqmitems, "Integrity/EB+12/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+12"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+12"])

eblayout(dqmitems, "Integrity/EB+12/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+12"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+12"])

eblayout(dqmitems, "Integrity/EB+12/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+12"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+12"])

eblayout(dqmitems, "Integrity/EB+12/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+12"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+12"])

eblayout(dqmitems, "Integrity/EB+12/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+12"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+12"])

eblayout(dqmitems, "Integrity/EB+12/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+12"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+12"])

eblayout(dqmitems, "Integrity/EB+12/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+12"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+12"])

eblayout(dqmitems, "PedestalOnline/EB+12/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+12"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+12",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+12"])

eblayout(dqmitems, "Pedestal/EB+12/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+12"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+12",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+12"]),

eblayout(dqmitems, "Pedestal/EB+12/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+12"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+12",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+12"]),

eblayout(dqmitems, "Pedestal/EB+12/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+12"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+12",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+12"]),

eblayout(dqmitems, "Pedestal/EB+12/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+12"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+12"])

eblayout(dqmitems, "Pedestal/EB+12/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+12 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+12 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+12 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+12 G16"])

eblayout(dqmitems, "TestPulse/EB+12/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+12"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+12",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+12 G01"])

eblayout(dqmitems, "TestPulse/EB+12/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+12"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+12",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+12 G06"])

eblayout(dqmitems, "TestPulse/EB+12/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+12"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+12",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+12 G12"])

eblayout(dqmitems, "TestPulse/EB+12/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+12"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+12"])

eblayout(dqmitems, "TestPulse/EB+12/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+12 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+12 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+12 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+12/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+12 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+12 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+12 G16",
   None])

eblayout(dqmitems, "Laser/EB+12/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+12"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+12"])

eblayout(dqmitems, "Laser/EB+12/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+12 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+12 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+12 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+12 G16"])

eblayout(dqmitems, "Laser/EB+12/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+12",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+12"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+12",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+12"])

eblayout(dqmitems, "Laser/EB+12/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+12",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+12 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+12",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+12 L1B"])

eblayout(dqmitems, "Laser/EB+12/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+12 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+12 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+12 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+12/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+12 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+12 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+12 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+12/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+12",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+12"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+12",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+12"])

eblayout(dqmitems, "Laser/EB+12/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+12",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+12 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+12",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+12 L4B"])

eblayout(dqmitems, "Laser/EB+12/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+12 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+12 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+12 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+12/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+12 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+12 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+12 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+12/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+12"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+12"])

eblayout(dqmitems, "Timing/EB+12/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+12"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+12",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+12"])

eblayout(dqmitems, "Integrity/EB+13/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+13"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+13"])

eblayout(dqmitems, "Integrity/EB+13/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+13"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+13"])

eblayout(dqmitems, "Integrity/EB+13/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+13"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+13"])

eblayout(dqmitems, "Integrity/EB+13/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+13"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+13"])

eblayout(dqmitems, "Integrity/EB+13/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+13"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+13"])

eblayout(dqmitems, "Integrity/EB+13/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+13"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+13"])

eblayout(dqmitems, "Integrity/EB+13/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+13"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+13"])

eblayout(dqmitems, "PedestalOnline/EB+13/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+13"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+13",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+13"])

eblayout(dqmitems, "Pedestal/EB+13/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+13"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+13",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+13"]),

eblayout(dqmitems, "Pedestal/EB+13/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+13"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+13",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+13"]),

eblayout(dqmitems, "Pedestal/EB+13/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+13"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+13",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+13"]),

eblayout(dqmitems, "Pedestal/EB+13/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+13"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+13"])

eblayout(dqmitems, "Pedestal/EB+13/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+13 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+13 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+13 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+13 G16"])

eblayout(dqmitems, "TestPulse/EB+13/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+13"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+13",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+13 G01"])

eblayout(dqmitems, "TestPulse/EB+13/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+13"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+13",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+13 G06"])

eblayout(dqmitems, "TestPulse/EB+13/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+13"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+13",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+13 G12"])

eblayout(dqmitems, "TestPulse/EB+13/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+13"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+13"])

eblayout(dqmitems, "TestPulse/EB+13/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+13 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+13 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+13 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+13/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+13 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+13 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+13 G16",
   None])

eblayout(dqmitems, "Laser/EB+13/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+13"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+13"])

eblayout(dqmitems, "Laser/EB+13/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+13 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+13 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+13 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+13 G16"])

eblayout(dqmitems, "Laser/EB+13/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+13",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+13"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+13",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+13"])

eblayout(dqmitems, "Laser/EB+13/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+13",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+13 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+13",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+13 L1B"])

eblayout(dqmitems, "Laser/EB+13/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+13 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+13 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+13 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+13/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+13 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+13 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+13 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+13/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+13",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+13"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+13",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+13"])

eblayout(dqmitems, "Laser/EB+13/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+13",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+13 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+13",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+13 L4B"])

eblayout(dqmitems, "Laser/EB+13/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+13 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+13 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+13 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+13/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+13 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+13 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+13 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+13/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+13"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+13"])

eblayout(dqmitems, "Timing/EB+13/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+13"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+13",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+13"])

eblayout(dqmitems, "Integrity/EB+14/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+14"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+14"])

eblayout(dqmitems, "Integrity/EB+14/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+14"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+14"])

eblayout(dqmitems, "Integrity/EB+14/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+14"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+14"])

eblayout(dqmitems, "Integrity/EB+14/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+14"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+14"])

eblayout(dqmitems, "Integrity/EB+14/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+14"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+14"])

eblayout(dqmitems, "Integrity/EB+14/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+14"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+14"])

eblayout(dqmitems, "Integrity/EB+14/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+14"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+14"])

eblayout(dqmitems, "PedestalOnline/EB+14/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+14"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+14",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+14"])

eblayout(dqmitems, "Pedestal/EB+14/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+14"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+14",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+14"]),

eblayout(dqmitems, "Pedestal/EB+14/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+14"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+14",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+14"]),

eblayout(dqmitems, "Pedestal/EB+14/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+14"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+14",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+14"]),

eblayout(dqmitems, "Pedestal/EB+14/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+14"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+14"])

eblayout(dqmitems, "Pedestal/EB+14/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+14 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+14 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+14 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+14 G16"])

eblayout(dqmitems, "TestPulse/EB+14/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+14"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+14",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+14 G01"])

eblayout(dqmitems, "TestPulse/EB+14/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+14"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+14",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+14 G06"])

eblayout(dqmitems, "TestPulse/EB+14/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+14"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+14",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+14 G12"])

eblayout(dqmitems, "TestPulse/EB+14/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+14"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+14"])

eblayout(dqmitems, "TestPulse/EB+14/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+14 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+14 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+14 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+14/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+14 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+14 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+14 G16",
   None])

eblayout(dqmitems, "Laser/EB+14/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+14"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+14"])

eblayout(dqmitems, "Laser/EB+14/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+14 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+14 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+14 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+14 G16"])

eblayout(dqmitems, "Laser/EB+14/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+14",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+14"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+14",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+14"])

eblayout(dqmitems, "Laser/EB+14/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+14",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+14 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+14",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+14 L1B"])

eblayout(dqmitems, "Laser/EB+14/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+14 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+14 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+14 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+14/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+14 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+14 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+14 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+14/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+14",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+14"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+14",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+14"])

eblayout(dqmitems, "Laser/EB+14/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+14",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+14 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+14",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+14 L4B"])

eblayout(dqmitems, "Laser/EB+14/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+14 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+14 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+14 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+14/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+14 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+14 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+14 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+14/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+14"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+14"])

eblayout(dqmitems, "Timing/EB+14/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+14"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+14",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+14"])

eblayout(dqmitems, "Integrity/EB+15/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+15"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+15"])

eblayout(dqmitems, "Integrity/EB+15/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+15"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+15"])

eblayout(dqmitems, "Integrity/EB+15/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+15"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+15"])

eblayout(dqmitems, "Integrity/EB+15/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+15"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+15"])

eblayout(dqmitems, "Integrity/EB+15/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+15"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+15"])

eblayout(dqmitems, "Integrity/EB+15/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+15"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+15"])

eblayout(dqmitems, "Integrity/EB+15/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+15"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+15"])

eblayout(dqmitems, "PedestalOnline/EB+15/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+15"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+15",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+15"])

eblayout(dqmitems, "Pedestal/EB+15/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+15"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+15",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+15"]),

eblayout(dqmitems, "Pedestal/EB+15/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+15"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+15",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+15"]),

eblayout(dqmitems, "Pedestal/EB+15/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+15"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+15",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+15"]),

eblayout(dqmitems, "Pedestal/EB+15/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+15"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+15"])

eblayout(dqmitems, "Pedestal/EB+15/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+15 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+15 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+15 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+15 G16"])

eblayout(dqmitems, "TestPulse/EB+15/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+15"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+15",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+15 G01"])

eblayout(dqmitems, "TestPulse/EB+15/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+15"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+15",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+15 G06"])

eblayout(dqmitems, "TestPulse/EB+15/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+15"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+15",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+15 G12"])

eblayout(dqmitems, "TestPulse/EB+15/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+15"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+15"])

eblayout(dqmitems, "TestPulse/EB+15/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+15 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+15 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+15 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+15/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+15 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+15 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+15 G16",
   None])

eblayout(dqmitems, "Laser/EB+15/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+15"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+15"])

eblayout(dqmitems, "Laser/EB+15/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+15 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+15 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+15 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+15 G16"])

eblayout(dqmitems, "Laser/EB+15/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+15",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+15"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+15",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+15"])

eblayout(dqmitems, "Laser/EB+15/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+15",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+15 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+15",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+15 L1B"])

eblayout(dqmitems, "Laser/EB+15/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+15 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+15 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+15 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+15/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+15 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+15 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+15 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+15/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+15",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+15"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+15",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+15"])

eblayout(dqmitems, "Laser/EB+15/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+15",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+15 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+15",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+15 L4B"])

eblayout(dqmitems, "Laser/EB+15/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+15 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+15 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+15 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+15/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+15 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+15 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+15 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+15/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+15"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+15"])

eblayout(dqmitems, "Timing/EB+15/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+15"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+15",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+15"])

eblayout(dqmitems, "Integrity/EB+16/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+16"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+16"])

eblayout(dqmitems, "Integrity/EB+16/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+16"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+16"])

eblayout(dqmitems, "Integrity/EB+16/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+16"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+16"])

eblayout(dqmitems, "Integrity/EB+16/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+16"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+16"])

eblayout(dqmitems, "Integrity/EB+16/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+16"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+16"])

eblayout(dqmitems, "Integrity/EB+16/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+16"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+16"])

eblayout(dqmitems, "Integrity/EB+16/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+16"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+16"])

eblayout(dqmitems, "PedestalOnline/EB+16/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+16"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+16",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+16"])

eblayout(dqmitems, "Pedestal/EB+16/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+16"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+16",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+16"]),

eblayout(dqmitems, "Pedestal/EB+16/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+16"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+16",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+16"]),

eblayout(dqmitems, "Pedestal/EB+16/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+16"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+16",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+16"]),

eblayout(dqmitems, "Pedestal/EB+16/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+16"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+16"])

eblayout(dqmitems, "Pedestal/EB+16/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+16 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+16 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+16 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+16 G16"])

eblayout(dqmitems, "TestPulse/EB+16/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+16"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+16",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+16 G01"])

eblayout(dqmitems, "TestPulse/EB+16/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+16"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+16",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+16 G06"])

eblayout(dqmitems, "TestPulse/EB+16/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+16"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+16",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+16 G12"])

eblayout(dqmitems, "TestPulse/EB+16/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+16"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+16"])

eblayout(dqmitems, "TestPulse/EB+16/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+16 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+16 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+16 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+16/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+16 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+16 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+16 G16",
   None])

eblayout(dqmitems, "Laser/EB+16/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+16"])

eblayout(dqmitems, "Laser/EB+16/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+16 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+16 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+16 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+16 G16"])

eblayout(dqmitems, "Laser/EB+16/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+16",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+16"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+16",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+16"])

eblayout(dqmitems, "Laser/EB+16/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+16",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+16 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+16",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+16 L1B"])

eblayout(dqmitems, "Laser/EB+16/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+16 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+16 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+16 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+16/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+16 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+16 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+16 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+16/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+16",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+16"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+16",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+16"])

eblayout(dqmitems, "Laser/EB+16/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+16",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+16 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+16",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+16 L4B"])

eblayout(dqmitems, "Laser/EB+16/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+16 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+16 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+16 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+16/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+16 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+16 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+16 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+16/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+16"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+16"])

eblayout(dqmitems, "Timing/EB+16/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+16"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+16",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+16"])

eblayout(dqmitems, "Integrity/EB+17/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+17"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+17"])

eblayout(dqmitems, "Integrity/EB+17/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+17"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+17"])

eblayout(dqmitems, "Integrity/EB+17/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+17"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+17"])

eblayout(dqmitems, "Integrity/EB+17/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+17"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+17"])

eblayout(dqmitems, "Integrity/EB+17/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+17"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+17"])

eblayout(dqmitems, "Integrity/EB+17/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+17"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+17"])

eblayout(dqmitems, "Integrity/EB+17/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+17"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+17"])

eblayout(dqmitems, "PedestalOnline/EB+17/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+17"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+17",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+17"])

eblayout(dqmitems, "Pedestal/EB+17/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+17"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+17",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+17"]),

eblayout(dqmitems, "Pedestal/EB+17/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+17"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+17",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+17"]),

eblayout(dqmitems, "Pedestal/EB+17/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+17"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+17",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+17"]),

eblayout(dqmitems, "Pedestal/EB+17/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+17"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+17"])

eblayout(dqmitems, "Pedestal/EB+17/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+17 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+17 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+17 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+17 G16"])

eblayout(dqmitems, "TestPulse/EB+17/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+17"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+17",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+17 G01"])

eblayout(dqmitems, "TestPulse/EB+17/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+17"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+17",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+17 G06"])

eblayout(dqmitems, "TestPulse/EB+17/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+17"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+17",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+17 G12"])

eblayout(dqmitems, "TestPulse/EB+17/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+17"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+17"])

eblayout(dqmitems, "TestPulse/EB+17/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+17 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+17 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+17 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+17/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+17 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+17 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+17 G16",
   None])

eblayout(dqmitems, "Laser/EB+17/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+17"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+17"])

eblayout(dqmitems, "Laser/EB+17/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+17 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+17 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+17 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+17 G16"])

eblayout(dqmitems, "Laser/EB+17/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+17",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+17"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+17",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+17"])

eblayout(dqmitems, "Laser/EB+17/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+17",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+17 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+17",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+17 L1B"])

eblayout(dqmitems, "Laser/EB+17/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+17 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+17 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+17 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+17/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+17 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+17 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+17 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+17/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+17",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+17"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+17",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+17"])

eblayout(dqmitems, "Laser/EB+17/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+17",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+17 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+17",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+17 L4B"])

eblayout(dqmitems, "Laser/EB+17/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+17 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+17 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+17 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+17/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+17 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+17 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+17 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+17/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+17"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+17"])

eblayout(dqmitems, "Timing/EB+17/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+17"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+17",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+17"])

eblayout(dqmitems, "Integrity/EB+18/0-Channel-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality EB+18"],
  ["EcalBarrel/EBOccupancyTask/EBOT occupancy EB+18"])

eblayout(dqmitems, "Integrity/EB+18/1-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/Gain/EBIT gain EB+18"],
  ["EcalBarrel/EBIntegrityTask/ChId/EBIT ChId EB+18"])

eblayout(dqmitems, "Integrity/EB+18/2-Gain-Integrity",
  ["EcalBarrel/EBIntegrityTask/GainSwitch/EBIT gain switch EB+18"],
  ["EcalBarrel/EBIntegrityTask/GainSwitchStay/EBIT gain switch stay EB+18"])

eblayout(dqmitems, "Integrity/EB+18/3-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/TTId/EBIT TTId EB+18"],
  ["EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize EB+18"])

eblayout(dqmitems, "Integrity/EB+18/4-MemBox-Global-Integrity",
  ["EcalBarrel/EBIntegrityClient/EBIT data integrity quality MEM EB+18"],
  ["EcalBarrel/EBOccupancyTask/EBOT MEM occupancy EB+18"])

eblayout(dqmitems, "Integrity/EB+18/5-MemBox-Channel-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId EB+18"],
  ["EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain EB+18"])

eblayout(dqmitems, "Integrity/EB+18/6-MemBox-TT-Integrity",
  ["EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId EB+18"],
  ["EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize EB+18"])

eblayout(dqmitems, "PedestalOnline/EB+18/Gain12/0-PedestalOnline",
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal quality G12 EB+18"],
  ["EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal mean G12 EB+18",
   "EcalBarrel/EBPedestalOnlineClient/EBPOT pedestal rms G12 EB+18"])

eblayout(dqmitems, "Pedestal/EB+18/Gain01/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G01 EB+18"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G01 EB+18",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G01 EB+18"]),

eblayout(dqmitems, "Pedestal/EB+18/Gain06/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G06 EB+18"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G06 EB+18",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G06 EB+18"]),

eblayout(dqmitems, "Pedestal/EB+18/Gain12/0-Pedestal",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality G12 EB+18"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal mean G12 EB+18",
   "EcalBarrel/EBPedestalClient/EBPT pedestal rms G12 EB+18"]),

eblayout(dqmitems, "Pedestal/EB+18/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G01 EB+18"],
  ["EcalBarrel/EBPedestalClient/EBPT pedestal quality PNs G16 EB+18"])

eblayout(dqmitems, "Pedestal/EB+18/PNs/0-Pedestal-PNs",
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+18 G01",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+18 G01"],
  ["EcalBarrel/EBPedestalTask/PN/Gain01/EBPDT PNs pedestal EB+18 G16",
   "EcalBarrel/EBPedestalClient/EBPDT PNs rms EB+18 G16"])

eblayout(dqmitems, "TestPulse/EB+18/Gain01/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G01 EB+18"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G01 EB+18",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+18 G01"])

eblayout(dqmitems, "TestPulse/EB+18/Gain06/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G06 EB+18"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G06 EB+18",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+18 G06"])

eblayout(dqmitems, "TestPulse/EB+18/Gain12/0-TestPulse",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality G12 EB+18"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse amplitude G12 EB+18",
   "EcalBarrel/EBTestPulseTask/EBTPT shape EB+18 G12"])

eblayout(dqmitems, "TestPulse/EB+18/PNs/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G01 EB+18"],
  ["EcalBarrel/EBTestPulseClient/EBTPT test pulse quality PNs G16 EB+18"])

eblayout(dqmitems, "TestPulse/EB+18/PNs/Gain01/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs pedestal EB+18 G01",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+18 G01"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain01/EBPDT PNs amplitude EB+18 G01",
   None])

eblayout(dqmitems, "TestPulse/EB+18/PNs/Gain16/0-TestPulse-PNs",
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs pedestal EB+18 G16",
   "EcalBarrel/EBTestPulseClient/EBPDT PNs pedestal rms EB+18 G16"],
  ["EcalBarrel/EBTestPulseTask/PN/Gain16/EBPDT PNs amplitude EB+18 G16",
   None])

eblayout(dqmitems, "Laser/EB+18/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 EB+18"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 EB+18"])

eblayout(dqmitems, "Laser/EB+18/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+18 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L1 PNs EB+18 G16"],
  ["EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+18 G01",
   "EcalBarrel/EBLaserClient/EBLT laser quality L4 PNs EB+18 G16"])

eblayout(dqmitems, "Laser/EB+18/L1/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1A EB+18",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1A EB+18"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L1B EB+18",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L1B EB+18"])

eblayout(dqmitems, "Laser/EB+18/L1/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L1A EB+18",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+18 L1A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L1B EB+18",
   "EcalBarrel/EBLaserTask/Laser1/EBLT shape EB+18 L1B"])

eblayout(dqmitems, "Laser/EB+18/L1/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs pedestal EB+18 G01 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain01/EBPDT PNs pedestal rms EB+18 G01 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain01/EBPDT PNs amplitude EB+18 G01 L1",
   None])

eblayout(dqmitems, "Laser/EB+18/L1/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs pedestal EB+18 G16 L1",
   "EcalBarrel/EBLaserClient/Laser1/PN/Gain16/EBPDT PNs pedestal rms EB+18 G16 L1"],
  ["EcalBarrel/EBLaserTask/Laser1/PN/Gain16/EBPDT PNs amplitude EB+18 G16 L1",
   None])

eblayout(dqmitems, "Laser/EB+18/L4/0-Laser",
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4A EB+18",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4A EB+18"],
  ["EcalBarrel/EBLaserClient/EBLT amplitude L4B EB+18",
   "EcalBarrel/EBLaserClient/EBLT amplitude over PN L4B EB+18"])

eblayout(dqmitems, "Laser/EB+18/L4/1-Laser",
  ["EcalBarrel/EBLaserClient/EBLT timing L4A EB+18",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+18 L4A"],
  ["EcalBarrel/EBLaserClient/EBLT timing L4B EB+18",
   "EcalBarrel/EBLaserTask/Laser4/EBLT shape EB+18 L4B"])

eblayout(dqmitems, "Laser/EB+18/L4/2-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs pedestal EB+18 G01 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain01/EBPDT PNs pedestal rms EB+18 G01 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain01/EBPDT PNs amplitude EB+18 G01 L4",
   None])

eblayout(dqmitems, "Laser/EB+18/L4/3-Laser",
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs pedestal EB+18 G16 L4",
   "EcalBarrel/EBLaserClient/Laser4/PN/Gain16/EBPDT PNs pedestal rms EB+18 G16 L4"],
  ["EcalBarrel/EBLaserTask/Laser4/PN/Gain16/EBPDT PNs amplitude EB+18 G16 L4",
   None])

eblayout(dqmitems, "Timing/EB+18/0-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+18"],
  ["EcalBarrel/EBTimingClient/EBTMT timing EB+18"])

eblayout(dqmitems, "Timing/EB+18/1-Timing",
  ["EcalBarrel/EBTimingClient/EBTMT timing quality EB+18"],
  ["EcalBarrel/EBTimingClient/EBTMT timing mean EB+18",
   "EcalBarrel/EBTimingClient/EBTMT timing rms EB+18"])

eblayout(dqmitems, "Cluster/0-Cluster",
  ["EcalBarrel/EBClusterTask/EBCLT island BC energy"],
  ["EcalBarrel/EBClusterTask/EBCLT island BC energy map"])

eblayout(dqmitems, "Cluster/1-Cluster",
  ["EcalBarrel/EBClusterTask/EBCLT island BC number"],
  ["EcalBarrel/EBClusterTask/EBCLT island BC number map"])

eblayout(dqmitems, "Cluster/2-Cluster",
  ["EcalBarrel/EBClusterTask/EBCLT island BC crystals"],
  ["EcalBarrel/EBClusterTask/EBCLT island BC size map"])

eblayout(dqmitems, "Cluster/3-Cluster",
  ["EcalBarrel/EBClusterTask/EBCLT island SC energy"],
  ["EcalBarrel/EBClusterTask/EBCLT island SC energy map"])

eblayout(dqmitems, "Cluster/4-Cluster",
  ["EcalBarrel/EBClusterTask/EBCLT island SC number"],
  ["EcalBarrel/EBClusterTask/EBCLT island SC number map"])

eblayout(dqmitems, "Cluster/5-Cluster",
  ["EcalBarrel/EBClusterTask/EBCLT island SC size"],
  ["EcalBarrel/EBClusterTask/EBCLT island SC size map"])

eblayout(dqmitems, "Cluster/6-Cluster",
  ["EcalBarrel/EBClusterTask/EBCLT island BC ET map"],
  ["EcalBarrel/EBClusterTask/EBCLT island SC ET map"])

