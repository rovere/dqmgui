import json
import os
import urllib
import urllib2
import base
import rootgen


class PlotFairyTest(base.BaseIntegrationTest):
    """
    Plot fairy tests are asserting that PNGs responded from DQMGUI's /plotfairy API are
    as the expected ones in this directory.

    If any of these tests fail, that means that response from plotfairy has diverged.
    There can be several root causes for this. Easiest way to identify would be to
    compare the file used in the test (e.g. IntegrationTestTH1.png) with DQMGUI's response.
    The URL is printed in stdout when you run this test, but it will look like the following:
    http://dqmgui-integration-1:8060/dqm/dev/jsonfairy/archive/1/IntegTest/era2017-04-06T133142550321/DQM/Pixel/IntegrationTest/IntegrationTestTH2?w=1042;h=512

    If you already know that the image processing has changed, you must also
    take care of updating the static png images against which the integration
    tests are run, otherwise a failure will appear.

    To update the images you need to:

    * generate the test ROOT file with:

    import rootgen

    rootgen.create_file({
      '/DQMData/Run 1/Pixel/Run summary/IntegrationTest': [
      {'name': 'IntegrationTestTH1', 'gen': rootgen.TH1F},
      {'name': 'IntegrationTestTH1_alt', 'gen': rootgen.TH1F_alt},
      {'name': 'IntegrationTestTH2', 'gen': rootgen.TH2F}
      ]},
      dataset='PlotFairyTest'
    )

    This will create, in the very same folder, a ROOT file named DQM_V0001_R000001__PlotFairyTest__era2017-08-18T124528612428__DQM.root

    * Upload the generated ROOT file to a test (up-to-date codewise) webserver
      and wait for it to be indexed.

    * Produce the following images:

       * Pixel/IntegrationTest/IntegrationTestTH1?w=1042;h=512
       * Pixel/IntegrationTest/IntegrationTestTH2?w=1042;h=512
       * plotfairy/overlay?withref=yes;obj=archive/1/PlotFairyTest/era2017-08-18T124528612428/DQM/Pixel/IntegrationTest/IntegrationTestTH1;obj=archive/1/PlotFairyTest/era2017-08-18T124528612428/DQM/Pixel/IntegrationTest/IntegrationTestTH1;obj=archive/1/PlotFairyTest/era2017-08-18T124528612428/DQM/Pixel/IntegrationTest/IntegrationTestTH1_alt;w=1042;h=512

    * Save the images with the appropriate name.
    """

    @classmethod
    def setUpClass(cls):
        super(PlotFairyTest, cls).setUpClass()
        (cls.filename, cls.run_number, cls.dataset) = cls.prepareIndex({
            '/DQMData/Run 1/Pixel/Run summary/IntegrationTest': [
                {'name': 'IntegrationTestTH1', 'gen': rootgen.TH1F},
                {'name': 'IntegrationTestTH1_alt', 'gen': rootgen.TH1F_alt},
                {'name': 'IntegrationTestTH2', 'gen': rootgen.TH2F}
            ]
        }, dataset='PlotFairyTest')

    def test_RenderPlugin_TH1(self):
        (expected_content, file_stat) = self.read_file('IntegrationTestTH1.png')
        (content, headers) = self.fetch_histogram('Pixel/IntegrationTest/IntegrationTestTH1')

        self.assertEquals('image/png', headers['content-type'], 'Plotfairy should return PNG files.')
        self.assertEquals(file_stat.st_size, int(headers['content-length']),
                          'Plotfairy must respond with the same file size always.')
        self.assertEquals(content, expected_content, 'Plotfairy should have same PNG file content.')

    def test_RenderPlugin_TH2(self):
        (expected_content, file_stat) = self.read_file('IntegrationTestTH2.png')
        (content, headers) = self.fetch_histogram('Pixel/IntegrationTest/IntegrationTestTH2')

        self.assertEquals('image/png', headers['content-type'], 'Plotfairy should return PNG files.')
        self.assertEquals(file_stat.st_size, int(headers['content-length']),
                          'Plotfairy must respond with the same file size always.')
        self.assertEquals(content, expected_content, 'Plotfairy should have same PNG file content.')

    def test_overlay_TH1(self):
        (expected_content, file_stat) = self.read_file('overlay.png')

        obj1 = 'archive/%d%s/%s' % (self.run_number, self.dataset, 'Pixel/IntegrationTest/IntegrationTestTH1')
        obj2 = 'archive/%d%s/%s' % (self.run_number, self.dataset, 'Pixel/IntegrationTest/IntegrationTestTH1_alt')
        obj1_quoted = urllib.quote_plus(obj1)
        obj2_quoted = urllib.quote_plus(obj2)
        histogram_url = '%splotfairy/overlay?withref=yes;obj=%s;obj=%s;obj=%s;w=1042;h=512' \
                        % (self.base_url, obj1_quoted, obj1_quoted, obj2_quoted)
        (content, headers) = self.fetch_histogram_by_url(histogram_url)

        self.assertEquals('image/png', headers['content-type'], 'Plotfairy should return PNG files.')
        self.assertEquals(file_stat.st_size, int(headers['content-length']),
                          'Plotfairy must respond with the same file size always.')
        self.assertEquals(content, expected_content, 'Plotfairy should have same PNG file content.')

    def test_overlay_TH1_reflabel(self):
        (expected_content, file_stat) = self.read_file('overlay_reflabel.png')

        obj1 = 'archive/%d%s/%s' % (self.run_number, self.dataset, 'Pixel/IntegrationTest/IntegrationTestTH1')
        obj2 = 'archive/%d%s/%s' % (self.run_number, self.dataset, 'Pixel/IntegrationTest/IntegrationTestTH1_alt')
        obj1_quoted = urllib.quote_plus(obj1)
        obj2_quoted = urllib.quote_plus(obj2)
        histogram_url = '%splotfairy/overlay?withref=yes;obj=%s;obj=%s;obj=%s;w=1042;h=512;reflabel=FirstSample;reflabel=SecondSample' \
                        % (self.base_url, obj1_quoted, obj1_quoted, obj2_quoted)
        (content, headers) = self.fetch_histogram_by_url(histogram_url)

        self.assertEquals('image/png', headers['content-type'], 'Plotfairy should return PNG files.')
        self.assertEquals(file_stat.st_size, int(headers['content-length']),
                          'Plotfairy must respond with the same file size always.')
        self.assertEquals(content, expected_content, 'Plotfairy should have same PNG file content.')

    def read_file(self, filename):
        script_dir = os.path.dirname(os.path.realpath(__file__))
        expected_output_path = script_dir + '/' + filename
        file_stat = os.stat(expected_output_path)
        with open(expected_output_path) as file:
            expected_content = file.read()
        return expected_content, file_stat

    def fetch_histogram(self, name):
        histogram_url = '%splotfairy/archive/%d%s/%s?w=1042;h=512' \
                        % (self.base_url, self.run_number, self.dataset, name)
        return self.fetch_histogram_by_url(histogram_url)

    def fetch_histogram_by_url(self, url):
        histogram_response = urllib2.urlopen(url)
        histogram_content = histogram_response.read()
        print 'Histogram fetched from %s with status %d' % (url, histogram_response.getcode())
        self.assertEquals(histogram_response.getcode(), 200, 'Request failed. Status code received not 200')
        return histogram_content, histogram_response.headers.dict
