import os
import shutil
import tempfile
from urllib2 import urlopen

import time
from pyvirtualdisplay import Display
from selenium import webdriver
from selenium.common.exceptions import TimeoutException
from selenium.webdriver import ActionChains
from selenium.webdriver.common.by import By
from selenium.webdriver.support.wait import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC

from test.integration.base import BaseIntegrationTest


class SeleniumTest(BaseIntegrationTest):
    """
    Run this as a standalone python script:
    export PYTHONPATH=${PWD}:${PYTHONPATH}
    python test_selenium.py
    python test_selenium.py --skip-upload --show-browser

    SLC6 that is used in Continuous integration has only firefox-45.8.0-2.el6_8.x86_64 available,
    so to run these tests older version of firefox and selenium are used, together with xvfb (virtual X frame buffer)
    for a headless selenium test experience.

    Here's an example that works for Ubuntu 16.04:
    sudo apt-get remove firefox
    sudo apt-get install firefox=45.0.2+build1-0ubuntu1 xvfb
    sudo apt-mark hold firefox
    pip install selenium==2.53.6 pyvirtualdisplay
    """

    SKIP_UPLOAD = False
    SHOW_BROWSER = False

    @classmethod
    def setUpClass(cls):
        """
        Run once before any tests are executed.
        RelVal ROOT file is uploaded for use within tests.
        It is hosted on Marco Rovere's web server
        """

        super(SeleniumTest, cls).setUpClass()

        cls.temp_root_file_dir = tempfile.mkdtemp()
        root_file_url = 'https://rovere.web.cern.ch/rovere/DQM_V0001_R000000001__RelValZpTT_1500_13__CMSSW_9_1_0_pre2-90X_upgrade2017_realistic_v20-v1__DQMIO.root'
        cls.root_file_dataset = '/RelValZpTT_1500_13/CMSSW_9_1_0_pre2-90X_upgrade2017_realistic_v20-v1/DQMIO'

        if cls.SKIP_UPLOAD:
            print 'Skipping ROOT file upload during test execution.'
        else:
            f = urlopen(root_file_url)
            print "Downloading " + root_file_url

            root_filename = os.path.basename(root_file_url)
            root_file_path = cls.temp_root_file_dir + '/' + root_filename
            with open(root_file_path, "wb") as local_file:
                local_file.write(f.read())

            cls.upload(root_filename, root_file_path)
            cls.uploadWatch(cls.root_file_dataset)

    @classmethod
    def tearDownClass(cls):
        """
        Run once after all tests finished execution.
        """
        super(SeleniumTest, cls).tearDownClass()
        shutil.rmtree(cls.temp_root_file_dir)

    def setUp(self):
        """
        During setUp (run before every test) uploaded ROOT file is selected, so all tests start when
        """

        super(SeleniumTest, self).setUp()
        if self.SHOW_BROWSER:
            print 'Showing browser window'
        else:
            display = Display(visible=0, size=(1680, 1050))
            display.start()
        self.driver = webdriver.Firefox()
        self.driver.set_window_size(1680, 1050)  # optional
        self.driver.get(self.base_url)
        print self.driver.current_url
        # Clicks the Run #:
        self.waitAndFindElementContains("Run #").click()
        # Finds the dataset uploaded
        dataset_div_xpath = "//div[contains(text(), '%s')]" % self.root_file_dataset
        self.waitAndFindElement(dataset_div_xpath).click()
        # Finds the link that takes selects this dataset
        self.waitAndFindElement(dataset_div_xpath + '/following-sibling::div/a').click()
        # Check default Summary workspace is loaded
        self.waitAndFindElementContains('CSC - 100.0% - (Never)')

    def tearDown(self):
        """
        During tearDown (run after every test) browser logs are checked for JavaScript errors encountered and will fail if that's the case.
        """

        browser_log = self.driver.get_log('browser')
        errors_encountered = False
        for message in browser_log:
            if message['level'] == 'SEVERE':
                print 'JavaScript Error: %s' % message['message']
                errors_encountered = True
        self.driver.quit()
        if errors_encountered:
            self.fail('JavaScript errors encountered. See test execution output for details.')

    def waitAndFindElementContains(self, text):
        # Read here for further insight on the changes
        return self.waitAndFindElement(selector="//*[text()[contains(., '%s')]]" % text)

    def waitAndFindElement(self, selector, by=By.XPATH, presence_function=EC.visibility_of_element_located):
        """
        If exepected element is not found, the test will fail.
        """

        delay = 15  # seconds
        try:
            element_present = presence_function((by, selector))
            WebDriverWait(self.driver, delay).until(element_present)
            return self.driver.find_element(by, selector)
        except TimeoutException:
            self.fail('Element was not found by "%s" with selector "%s"' % (by, selector))

    def doubleClick(self, elem):
        action_chains = ActionChains(self.driver)
        action_chains.double_click(elem).perform()

    def test_workspaces_reports(self):
        self.waitAndFindElementContains('Workspace').click()
        self.waitAndFindElementContains('Reports').click()
        self.waitAndFindElementContains('Subsystem')

    def test_workspaces_shift(self):
        self.waitAndFindElementContains('Workspace').click()
        self.waitAndFindElementContains('Shift').click()
        self.waitAndFindElement("//div[@id = 'canvas']//a[contains(text(), '%s')]" % 'CSC').click()
        self.waitAndFindElementContains('00 Chamber Status')

    def test_workspaces_PrimaryVertexV(self):
        self.waitAndFindElementContains('Workspace').click()
        self.waitAndFindElementContains('Everything').click()
        self.waitAndFindElementContains('Vertexing').click()
        self.waitAndFindElementContains('PrimaryVertexV').click()
        self.waitAndFindElementContains('GenAllV_ClosestDistanceZ')

    def test_openHistogram(self):
        csc_elem = self.waitAndFindElementContains('CSC - 100.0% - (Never)')
        self.doubleClick(csc_elem)
        self.waitAndFindElement("//div[@id = 'canvas']//a[contains(text(), '%s')]" % 'CSC').click()
        self.waitAndFindElementContains('CSCOfflineMonitor').click()
        self.waitAndFindElementContains('BXMonitor').click()
        histo = self.waitAndFindElement(
            selector="//h3[contains(text(), '%s')]/following-sibling::div/img" % 'hALCTMatch')
        self.doubleClick(histo)
        self.waitAndFindElementContains('CSC/CSCOfflineMonitor/BXMonitor/hALCTMatch')

    def test_openHistogramJsRoot(self):
        self.test_openHistogram()
        self.waitAndFindElementContains('JSROOT mode').click()
        self.waitAndFindElementContains('ALCT position in ALCT-CLCT match window [BX]')

    def test_logoAndViewFilter(self):
        self.waitAndFindElement("//*[@id = 'cmslogo']").click()
        self.waitAndFindElement("//*[@id = 'subhead-search']").send_keys('CSC')
        self.waitAndFindElementContains('(1  match)')

    def test_linkMeButton(self):
        self.test_openHistogram()
        self.waitAndFindElementContains('Link-Me').click()
        link_me_expected = '/start?runnr=1;dataset=/RelValZpTT_1500_13/CMSSW_9_1_0_pre2-90X_upgrade2017_realistic_v20-v1/DQMIO;sampletype=offline_relval;filter=all;referencepos=overlay;referenceshow=customise;referencenorm=True;referenceobj1=refobj;referenceobj2=none;referenceobj3=none;referenceobj4=none;search=;striptype=object;stripruns=;stripaxis=run;stripomit=none;workspace=CSC;size=M;root=CSC/CSCOfflineMonitor/BXMonitor;focus=CSC/CSCOfflineMonitor/BXMonitor/hALCTMatch;zoom=yes;'
        link_me_actual = self.waitAndFindElement("//*[@class = 'ext-mb-input']").get_attribute('value')
        link_me_actual.index(link_me_expected)

    def test_rootFileLink(self):
        self.waitAndFindElementContains('Workspace').click()
        self.waitAndFindElementContains('CSC').click()
        root_file_href = self.waitAndFindElementContains('ROOT File').get_attribute('href')
        self.assertTrue(root_file_href.endswith('.root'), 'Root file link must point to a ROOT file.')

    def test_searchWorkspace(self):
        self.waitAndFindElementContains("Run #").click()
        search_field = self.waitAndFindElement("//*[@id = 'sample-search']")
        datasets = self.waitAndFindElement("//*[@id = 'datasets']")

        search_field.send_keys('CMS')
        time.sleep(1)
        self.assertGreater(len(datasets.find_elements_by_xpath(".//*")), 0, 'Search must provide results when for CMS')

        search_field.send_keys('42')
        time.sleep(1)
        self.assertEqual(len(datasets.find_elements_by_xpath(".//*")), 0,
                         'Search should not provide any results for CMS42.')


if __name__ == '__main__':
    import unittest
    import argparse

    parser = argparse.ArgumentParser(prog="GUI Tests", description="Will run GUI tests")
    parser.add_argument("--skip-upload", action="store_const", const=True, default=False)
    parser.add_argument("--show-browser", action="store_const", const=True, default=False)
    args = parser.parse_args()
    SeleniumTest.SKIP_UPLOAD = args.skip_upload
    SeleniumTest.SHOW_BROWSER = args.show_browser
    suite = unittest.TestLoader().loadTestsFromTestCase(SeleniumTest)
    unittest.TextTestRunner(verbosity=2).run(suite)
