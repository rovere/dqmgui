import subprocess

import sys


def regression_test(directory):
    import ROOT
    with open(directory + '/RootVersion.txt') as version_file:
        regression_root_version = version_file.readline().rstrip()
    current_root_version = ROOT.gROOT.GetVersion()
    command_raw = "visDQMIndex dump --sample %s %s data 2>&1 >/dev/null | wc -l"
    sample_sizes_path = directory + '/output.txt'

    print("Validating index files with ROOT version %s (current) against %s."
          % (current_root_version, regression_root_version))
    print("Calling the following command for every sample:")
    print(command_raw % ('<sample>', directory))
    print("Expected sample sizes taken from %s" % sample_sizes_path)

    fail = False
    with open(sample_sizes_path) as samples_file:
        for line in samples_file:
            pieces = line.split(' ')
            sample = pieces[0]
            expected_size = pieces[1].rstrip()
            command = command_raw % (sample, directory)
            out = subprocess.check_output([command], shell=True).rstrip()
            match = expected_size == out
            print("Sample %s. Expected line count: %s, actual line count: %s. %s"
                  % (sample, expected_size, out, 'PASS' if match else 'FAIL'))

            if not match:
                fail = True

    if fail:
        sys.exit('Samples size is different for current version of root.')


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description="Index regression test")
    parser.add_argument('directory', metavar='DIRECTORY', type=str,
                        help='Destination of extracted test_index.tar.bz2')
    args = parser.parse_args()

    regression_test(args.directory)
