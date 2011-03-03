void visDQMVerifyLoose(void)
{
  for (int i = 0; i < 15; ++i)
    gSystem->ResetSignal(i, kTRUE);

  const char *required[] = { "Run summary" };

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

  for (int i = 0, e = sizeof(required)/sizeof(required[0]); i != e; ++i)
  {
    TObject *obj = _file0->FindObjectAny(required[i]);
    if (! obj)
    {
      std::cerr << "VERIFY: Required object '" << required[i] << "' missing\n";
      exit(1);
    }
  }

  std::cerr << "VERIFY: Good to go\n";
  exit(0);
}
