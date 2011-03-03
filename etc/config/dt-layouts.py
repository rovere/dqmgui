def dtlayout(i, p, *rows): i["DT/Layouts/" + p] = DQMItem(layout=rows)

dtlayout(dqmitems, "DTIntegrityCheck_Summary",
  ["DT/DataIntegrity/FED770/FED770_ROSSummary",
   "DT/DataIntegrity/FED770/FED770_ROSStatus",
   "DT/DataIntegrity/FED770/SC10/FED770_SC10_SCTriggerBX"],
  ["DT/DataIntegrity/FED770/ROS10/FED770_ROS10_ROSError",
   "DT/DataIntegrity/FED770/FED770_FIFOStatus",
   "DT/DataIntegrity/FED770/SC10/FED770_SC10_SCTriggerQuality"])

dtlayout(dqmitems, "W1_St1_Signal",
  ["DT/DTDigiTask/Wheel0/Station1/$1/Occupancies/$1" ],
  ["DT/DTDigiTask/Wheel0/Station1/$1/TimeBoxes/$1",
   "DT/DTDigiTask/Wheel0/Station1/$1/TimeBoxes/$2",
   "DT/DTDigiTask/Wheel0/Station1/$1/TimeBoxes/$3"])

dtlayout(dqmitems, "W1_St2_Signal",
  ["DT/DTDigiTask/Wheel0/Station2/$1/Occupancies/$1"],
  ["DT/DTDigiTask/Wheel0/Station2/$1/TimeBoxes/$1",
   "DT/DTDigiTask/Wheel0/Station2/$1/TimeBoxes/$2",
   "DT/DTDigiTask/Wheel0/Station2/$1/TimeBoxes/$3"])

dtlayout(dqmitems, "W1_St3_Signal",
  ["DT/DTDigiTask/Wheel0/Station3/$1/Occupancies/$1"],
  ["DT/DTDigiTask/Wheel0/Station3/$1/TimeBoxes/$1",
   "DT/DTDigiTask/Wheel0/Station3/$1/TimeBoxes/$2",
   "DT/DTDigiTask/Wheel0/Station3/$1/TimeBoxes/$3"])

dtlayout(dqmitems, "W1_St4_Signal",
  ["DT/DTDigiTask/Wheel0/Station4/$1/Occupancies/$1"],
  ["DT/DTDigiTask/Wheel0/Station4/$1/TimeBoxes/$1",
   "DT/DTDigiTask/Wheel0/Station4/$1/TimeBoxes/$2"])

dtlayout(dqmitems, "DTLocalTrigger_Station_1",
  ["DT/DTLocalTriggerTask/Wheel0/$1/Station1/LocalTriggerPhi/$1",
   "DT/DTLocalTriggerTask/Wheel0/$1/Station1/LocalTriggerPhi/$2"],
  ["DT/DTLocalTriggerTask/Wheel0/$1/Station1/LocalTriggerPhi/$3",
   "DT/DTLocalTriggerTask/Wheel0/$1/Station1/LocalTriggerTheta/$1"])

dtlayout(dqmitems, "DTLocalTrigger_Station_2",
  ["DT/DTLocalTriggerTask/Wheel0/$1/Station2/LocalTriggerPhi/$1",
   "DT/DTLocalTriggerTask/Wheel0/$1/Station2/LocalTriggerPhi/$2"],
  ["DT/DTLocalTriggerTask/Wheel0/$1/Station2/LocalTriggerPhi/$3",
   "DT/DTLocalTriggerTask/Wheel0/$1/Station2/LocalTriggerTheta/$1"])

dtlayout(dqmitems, "DTLocalTrigger_Station_3",
  ["DT/DTLocalTriggerTask/Wheel0/$1/Station3/LocalTriggerPhi/$1",
   "DT/DTLocalTriggerTask/Wheel0/$1/Station3/LocalTriggerPhi/$2"],
  ["DT/DTLocalTriggerTask/Wheel0/$1/Station3/LocalTriggerPhi/$3",
   "DT/DTLocalTriggerTask/Wheel0/$1/Station3/LocalTriggerTheta/$1"])

dtlayout(dqmitems, "DTLocalTrigger_Station_4",
  ["DT/DTLocalTriggerTask/Wheel0/$1/Station4/LocalTriggerPhi/$1",
   "DT/DTLocalTriggerTask/Wheel0/$1/Station4/LocalTriggerPhi/$2"],
  ["DT/DTLocalTriggerTask/Wheel0/$1/Station4/LocalTriggerPhi/$3",
   "DT/DTLocalTriggerTask/Wheel0/$1/Station4/LocalTriggerTheta/$1"])


dtlayout(dqmitems, "W1_St1_OccupancyPerLayer",
  ["DT/DTDigiTask/Wheel0/Station1/$1/Occupancies/$2",
   "DT/DTDigiTask/Wheel0/Station1/$1/Occupancies/$6",
   "DT/DTDigiTask/Wheel0/Station1/$1/Occupancies/$10"],
  ["DT/DTDigiTask/Wheel0/Station1/$1/Occupancies/$3",
   "DT/DTDigiTask/Wheel0/Station1/$1/Occupancies/$7",
   "DT/DTDigiTask/Wheel0/Station1/$1/Occupancies/$11"],
  ["DT/DTDigiTask/Wheel0/Station1/$1/Occupancies/$4",
   "DT/DTDigiTask/Wheel0/Station1/$1/Occupancies/$8",
   "DT/DTDigiTask/Wheel0/Station1/$1/Occupancies/$12"],
  ["DT/DTDigiTask/Wheel0/Station1/$1/Occupancies/$5",
   "DT/DTDigiTask/Wheel0/Station1/$1/Occupancies/$9",
   "DT/DTDigiTask/Wheel0/Station1/$1/Occupancies/$13"])

dtlayout(dqmitems, "W1_St2_OccupancyPerLayer",
  ["DT/DTDigiTask/Wheel0/Station2/$1/Occupancies/$2",
   "DT/DTDigiTask/Wheel0/Station2/$1/Occupancies/$6",
   "DT/DTDigiTask/Wheel0/Station2/$1/Occupancies/$10"],
  ["DT/DTDigiTask/Wheel0/Station2/$1/Occupancies/$3",
   "DT/DTDigiTask/Wheel0/Station2/$1/Occupancies/$7",
   "DT/DTDigiTask/Wheel0/Station2/$1/Occupancies/$11"],
  ["DT/DTDigiTask/Wheel0/Station2/$1/Occupancies/$4",
   "DT/DTDigiTask/Wheel0/Station2/$1/Occupancies/$8",
   "DT/DTDigiTask/Wheel0/Station2/$1/Occupancies/$12"],
  ["DT/DTDigiTask/Wheel0/Station2/$1/Occupancies/$5",
   "DT/DTDigiTask/Wheel0/Station2/$1/Occupancies/$9",
   "DT/DTDigiTask/Wheel0/Station2/$1/Occupancies/$13"])

dtlayout(dqmitems, "W1_St3_OccupancyPerLayer",
  ["DT/DTDigiTask/Wheel0/Station3/$1/Occupancies/$2",
   "DT/DTDigiTask/Wheel0/Station3/$1/Occupancies/$6",
   "DT/DTDigiTask/Wheel0/Station3/$1/Occupancies/$10"],
  ["DT/DTDigiTask/Wheel0/Station3/$1/Occupancies/$3",
   "DT/DTDigiTask/Wheel0/Station3/$1/Occupancies/$7",
   "DT/DTDigiTask/Wheel0/Station3/$1/Occupancies/$11"],
  ["DT/DTDigiTask/Wheel0/Station3/$1/Occupancies/$4",
   "DT/DTDigiTask/Wheel0/Station3/$1/Occupancies/$8",
   "DT/DTDigiTask/Wheel0/Station3/$1/Occupancies/$12"],
  ["DT/DTDigiTask/Wheel0/Station3/$1/Occupancies/$5",
   "DT/DTDigiTask/Wheel0/Station3/$1/Occupancies/$9",
   "DT/DTDigiTask/Wheel0/Station3/$1/Occupancies/$13"])

dtlayout(dqmitems, "W1_St4_OccupancyPerLayer",
  ["DT/DTDigiTask/Wheel0/Station4/$1/Occupancies/$2",
   "DT/DTDigiTask/Wheel0/Station4/$1/Occupancies/$6"],
  ["DT/DTDigiTask/Wheel0/Station4/$1/Occupancies/$3",
   "DT/DTDigiTask/Wheel0/Station4/$1/Occupancies/$7"],
  ["DT/DTDigiTask/Wheel0/Station4/$1/Occupancies/$4",
   "DT/DTDigiTask/Wheel0/Station4/$1/Occupancies/$8"],
  ["DT/DTDigiTask/Wheel0/Station4/$1/Occupancies/$5",
   "DT/DTDigiTask/Wheel0/Station4/$1/Occupancies/$9"])

dtlayout(dqmitems, "DTIntegrityCheck_station1_first",
  ["DT/DataIntegrity/FED770/ROS10/ROB0/FED770_ROS10_ROB0_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB1/FED770_ROS10_ROB1_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB2/FED770_ROS10_ROB2_Occupancy"],
  ["DT/DataIntegrity/FED770/ROS10/ROB0/FED770_ROS10_ROB0_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB1/FED770_ROS10_ROB1_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB2/FED770_ROS10_ROB2_TimeBox"])

dtlayout(dqmitems, "DTIntegrityCheck_station1_second",
  ["DT/DataIntegrity/FED770/ROS10/ROB3/FED770_ROS10_ROB3_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB4/FED770_ROS10_ROB4_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB5/FED770_ROS10_ROB5_Occupancy"],
  ["DT/DataIntegrity/FED770/ROS10/ROB3/FED770_ROS10_ROB3_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB4/FED770_ROS10_ROB4_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB5/FED770_ROS10_ROB5_TimeBox"])

dtlayout(dqmitems, "DTIntegrityCheck_station2_first",
  ["DT/DataIntegrity/FED770/ROS10/ROB6/FED770_ROS10_ROB6_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB7/FED770_ROS10_ROB7_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB8/FED770_ROS10_ROB8_Occupancy"],
  ["DT/DataIntegrity/FED770/ROS10/ROB6/FED770_ROS10_ROB6_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB7/FED770_ROS10_ROB7_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB8/FED770_ROS10_ROB8_TimeBox"])

dtlayout(dqmitems, "DTIntegrityCheck_station2_second",
  ["DT/DataIntegrity/FED770/ROS10/ROB9/FED770_ROS10_ROB9_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB10/FED770_ROS10_ROB10_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB11/FED770_ROS10_ROB11_Occupancy"],
  ["DT/DataIntegrity/FED770/ROS10/ROB9/FED770_ROS10_ROB9_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB10/FED770_ROS10_ROB10_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB11/FED770_ROS10_ROB11_TimeBox"])

dtlayout(dqmitems, "DTIntegrityCheck_station3_first",
  ["DT/DataIntegrity/FED770/ROS10/ROB12/FED770_ROS10_ROB12_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB13/FED770_ROS10_ROB13_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB14/FED770_ROS10_ROB14_Occupancy"],
  ["DT/DataIntegrity/FED770/ROS10/ROB12/FED770_ROS10_ROB12_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB13/FED770_ROS10_ROB13_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB14/FED770_ROS10_ROB14_TimeBox"])

dtlayout(dqmitems, "DTIntegrityCheck_station3_second",
  ["DT/DataIntegrity/FED770/ROS10/ROB15/FED770_ROS10_ROB15_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB16/FED770_ROS10_ROB16_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB17/FED770_ROS10_ROB17_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB24/FED770_ROS10_ROB24_Occupancy"],
  ["DT/DataIntegrity/FED770/ROS10/ROB15/FED770_ROS10_ROB15_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB16/FED770_ROS10_ROB16_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB17/FED770_ROS10_ROB17_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB24/FED770_ROS10_ROB24_TimeBox"])

dtlayout(dqmitems, "DTIntegrityCheck_station4_first",
  ["DT/DataIntegrity/FED770/ROS10/ROB18/FED770_ROS10_ROB18_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB19/FED770_ROS10_ROB19_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB20/FED770_ROS10_ROB20_Occupancy"],
  ["DT/DataIntegrity/FED770/ROS10/ROB18/FED770_ROS10_ROB18_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB19/FED770_ROS10_ROB19_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB20/FED770_ROS10_ROB20_TimeBox"])

dtlayout(dqmitems, "DTIntegrityCheck_station4_second",
  ["DT/DataIntegrity/FED770/ROS10/ROB21/FED770_ROS10_ROB21_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB22/FED770_ROS10_ROB22_Occupancy",
   "DT/DataIntegrity/FED770/ROS10/ROB23/FED770_ROS10_ROB23_Occupancy"],
  ["DT/DataIntegrity/FED770/ROS10/ROB21/FED770_ROS10_ROB21_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB22/FED770_ROS10_ROB22_TimeBox",
   "DT/DataIntegrity/FED770/ROS10/ROB23/FED770_ROS10_ROB23_TimeBox"])
