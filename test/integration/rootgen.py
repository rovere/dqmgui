import datetime

import math


def root_mkdir_p(folder):
    import ROOT as r

    r.gDirectory.cd('/')
    fullpath = ''
    for level in folder.split('/'):
        fullpath += '/%s' % level
        if not level == '':
            if not r.gDirectory.GetDirectory(level):
                r.gDirectory.mkdir(level)
            r.gDirectory.cd(fullpath)


def gen_filename(directory=".", dataset="IntegTest"):
    version = 'V0001'
    run = 1
    runstr = 'R00000' + str(run)
    era = 'era' + datetime.datetime.utcnow().isoformat().replace(':', '').replace('.', '')
    datatier = 'DQM'
    filename = 'DQM_%s_%s__%s__%s__%s.root' % (version, runstr, dataset, era, datatier)
    return (
        filename,
        run,
        '/%s/%s/%s' % (dataset, era, datatier),
        '%s/%s' % (directory, filename)
    )


def create_file(content, directory='.', dataset='IntegTest'):
    import ROOT as r
    (filename, run, dataset, path) = gen_filename(directory, dataset=dataset)
    f = r.TFile(path, 'RECREATE')
    generated = 0
    for (folder, histos) in content.items():
        root_mkdir_p(folder)
        for h in histos:
            # make param dependant histoh['type']
            generated += 1
            h['gen'](h['name'])
    f.Write()
    f.Close()
    print 'Created %s with %d histograms in %d folders' % (path, generated, len(content))
    return filename, run, dataset, path


def TH1F(name, func=math.sin):
    import ROOT as r
    bins = 100
    xmin = 0
    xmax = 10
    step = float(xmax - xmin) / bins + xmin

    histo = r.TH1F(name, name, bins, xmin, xmax)
    histo.SetFillColor(632)
    for i in range(0, bins):
        x = step * i
        weight = func(x)
        histo.Fill(x, weight)
    histo.Write()


def TH1F_alt(name):
    TH1F(name, func=math.cos)


def TH2F(name):
    import ROOT as r
    bins = 10
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 10
    xstep = float(xmax - xmin) / bins
    ystep = float(ymax - ymin) / bins
    histo = r.TH2F(name, name, bins, xmin, xmax, bins, ymin, ymax)
    histo.SetFillColor(45)
    for i in range(bins):
        for j in range(bins):
            x = xmin + xstep * i
            y = ymin + ystep * j
            weight = math.sin(x) * math.cos(y) + 1
            histo.Fill(x, y, weight)
    histo.Write()


def TProfile(name):
    import ROOT as r
    bins = 100
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    xstep = float(xmax - xmin) / bins
    histo = r.TProfile(name, name, bins, xmin, xmax, ymin, ymax)
    histo.SetFillColor(45)
    for i in range(bins):
        x = xmin + xstep * i
        y = math.sin(x)
        histo.Fill(x, y)
    histo.Write()


def read_contents(filename):
    import ROOT as r

    f = r.TFile.Open(filename, 'read')
    scan_directory('')


def scan_directory(path):
    import ROOT as r
    directory = r.gDirectory.GetDirectory(path)
    keys = directory.GetListOfKeys()
    if len(keys) > 0:
        for key in keys:
            if key.IsFolder():
                step_into = '%s/%s' % (path, key.GetName())
                scan_directory(step_into)
            else:
                print '%s/%s %s' % (path, key.GetName(), key.GetClassName())
    else:
        print '%s empty' % path


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description="Root file manipulation utils")
    parser.add_argument('filename', metavar='FILENAME', type=str,
                        help='Filename to read')
    args = parser.parse_args()
    read_contents(args.filename)
