void visDQMVerifyStrict(void)
{
  for (int i = 0; i < 15; ++i)
    gSystem->ResetSignal(i, kTRUE);

  // NB: cannot use scalars like iRun, iEvent, iLumiSection.
  // We search for them manually out of an EventInfo.
  const char *required[] = { "Run summary",
			     "reportSummaryMap",
			     "reportSummaryContents",
			     "EventInfo" };

  if (! _file0)
  {
    std::cerr << "VERIFY: Invalid ROOT file\n";
    exit(1);
  }

  if (_file0->IsZombie())
  {
    std::cerr << "VERIFY: Zombie ROOT file\n";
    exit(1);
  }

  long long runnr = -1;
  long long eventnr = -1;
  long long luminr = -1;

  for (int i = 0, e = sizeof(required)/sizeof(required[0]); i != e; ++i)
  {
    TObject *obj = _file0->FindObjectAny(required[i]);
    if (! obj)
    {
      std::cerr << "VERIFY: Required object '" << required[i] << "' missing\n";
      exit(1);
    }

    if (! strcmp(required[i], "EventInfo"))
    {
      bool haverun = false;
      bool haveevent = false;
      bool havelumi = false;
      bool havesum = false;
      TDirectory *dir = (TDirectory *) obj;
      TIter next(dir->GetListOfKeys());
      TKey *key;

      while ((key = (TKey *) next()))
      {
	const char *name = key->ReadObj()->GetName();
	if (!strncmp(name, "<reportSummary>f=", 17))
	  havesum = true;
	if (!strncmp(name, "<iRun>i=", 8))
	  haverun = true, sscanf(name+8, "%jd", &runnr);
	if (!strncmp(name, "<iEvent>i=", 10))
	  haveevent = true, sscanf(name+10, "%jd", &eventnr);
	if (!strncmp(name, "<iLumiSection>i=", 16))
	  havelumi = true, sscanf(name+16, "%jd", &luminr);
      }

      if (! havesum)
      {
	std::cerr << "VERIFY: Required object 'reportSummary' missing\n";
	exit(1);
      }
      if (! haverun)
      {
	std::cerr << "VERIFY: Required object 'iRun' missing\n";
	exit(1);
      }
      if (! haveevent)
      {
	std::cerr << "VERIFY: Required object 'iEvent' missing\n";
	exit(1);
      }
      if (! havelumi)
      {
	std::cerr << "VERIFY: Required object 'iLumiSection' missing\n";
	exit(1);
      }
    }
  }

  std::cerr << "VERIFY: Good to go; R="
	    << runnr << ":L="
	    << luminr << ":E="
	    << eventnr << "\n";
  exit(0);
}
