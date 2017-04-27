import json
import urllib2

import base
import rootgen


class JsonFairyTest(base.BaseIntegrationTest):
    @classmethod
    def setUpClass(cls):
        super(JsonFairyTest, cls).setUpClass()
        (cls.filename, cls.run_number, cls.dataset) = cls.prepareIndex({
            '/DQMData/Run 1/Pixel/Run summary/AdditionalPixelErrors': [
                {'name': 'FedChNErr', 'gen': rootgen.TH1F},
                {'name': 'FedChLErr', 'gen': rootgen.TH2F},
                {'name': 'FedETypeNErr', 'gen': rootgen.TH2F},
            ],
            '/DQMData/Run 1/Pixel/Run summary/AdditionalPixelErrors/FED_0': [
                {'name': 'FedETypeNErr_siPixelDigis_0', 'gen': rootgen.TH1F},
            ],
            '/DQMData/Run 1/Pixel/Run summary': [
                {'name': 'averageDigiOccupancy', 'gen': rootgen.TProfile}],
            '/DQMData/Run 1/Pixel/Run summary/Barrel': [
                {'name': 'SUMOFF_charge_OnTrack_Barrel', 'gen': rootgen.TH1F},
                {'name': 'SUMOFF_nclusters_OnTrack_Barrel', 'gen': rootgen.TH1F},
                {'name': 'SUMOFF_size_OnTrack_Barrel', 'gen': rootgen.TH1F}],
            '/DQMData/Run 1/Pixel/Run summary/Endcap': [
                {'name': 'SUMOFF_charge_OnTrack_Endcap', 'gen': rootgen.TH1F},
                {'name': 'SUMOFF_nclusters_OnTrack_Endcap', 'gen': rootgen.TH1F},
                {'name': 'SUMOFF_size_OnTrack_Endcap', 'gen': rootgen.TH1F}]
        }, dataset='JsonFairyTest')

    def test_TH1F(self):
        response = self.fetch_histogram_json('Pixel/AdditionalPixelErrors/FedChNErr')

        self.assertEquals('TH1', response['hist']['type'])
        self.assertEquals('FedChNErr', response['hist']['title'])
        self.assertEquals(100, len(response['hist']['bins']['content']))
        self.assertEquals(0.099833413959, response['hist']['bins']['content'][1])
        self.assertEquals(0.19866932929, response['hist']['bins']['content'][2])
        self.assertEquals(0.29552021623, response['hist']['bins']['content'][3])
        self.assertEquals(0.999574, response['hist']['values']['max'])
        self.assertEquals(-0.999923, response['hist']['values']['min'])
        self.assertEquals(4.34981, response['hist']['stats']['mean']['X']['value'])
        self.assertEquals(4.43677, response['hist']['stats']['rms']['X']['value'])
        self.assertEquals(1, response['hist']['xaxis']['first']['id'])
        self.assertEquals(0, response['hist']['xaxis']['first']['value'])
        self.assertEquals(100, response['hist']['xaxis']['last']['id'])
        self.assertEquals(10, response['hist']['xaxis']['last']['value'])

    def test_TH2F(self):
        response = self.fetch_histogram_json('Pixel/AdditionalPixelErrors/FedChLErr')

        self.assertEquals('TH2', response['hist']['type'])
        self.assertEquals('FedChLErr', response['hist']['title'])
        self.assertEquals(10, len(response['hist']['bins']['content']))
        self.assertEquals(1.84147, response['hist']['bins']['content'][0][1])
        self.assertEquals(1.4913, response['hist']['bins']['content'][1][2])
        self.assertEquals(0.624506, response['hist']['bins']['content'][9][9])
        self.assertEquals(1.98936, response['hist']['values']['max'])
        self.assertEquals(0.0205428, response['hist']['values']['min'])
        self.assertEquals(4.50422, response['hist']['stats']['mean']['X']['value'])
        self.assertEquals(4.40922, response['hist']['stats']['mean']['Y']['value'])
        self.assertEquals(2.88947, response['hist']['stats']['rms']['X']['value'])
        self.assertEquals(2.88805, response['hist']['stats']['rms']['Y']['value'])
        self.assertEquals(1, response['hist']['xaxis']['first']['id'])
        self.assertEquals(0, response['hist']['xaxis']['first']['value'])
        self.assertEquals(10, response['hist']['xaxis']['last']['id'])
        self.assertEquals(10, response['hist']['xaxis']['last']['value'])
        self.assertEquals(1, response['hist']['yaxis']['first']['id'])
        self.assertEquals(0, response['hist']['yaxis']['first']['value'])
        self.assertEquals(10, response['hist']['yaxis']['last']['id'])
        self.assertEquals(10, response['hist']['yaxis']['last']['value'])

    def test_TProfile(self):
        response = self.fetch_histogram_json('Pixel/averageDigiOccupancy')

        self.assertEquals('TProfile', response['hist']['type'])
        self.assertEquals('averageDigiOccupancy', response['hist']['title'])
        self.assertEquals(100, len(response['hist']['bins']['content']))
        self.assertEquals(0.099833416647, response['hist']['bins']['content'][1])
        self.assertEquals(0.1986693308, response['hist']['bins']['content'][2])
        self.assertEquals(-0.45753589378, response['hist']['bins']['content'][99])
        self.assertEquals(0.999574, response['hist']['values']['max'])
        self.assertEquals(-0.999923, response['hist']['values']['min'])
        self.assertEquals(4.95, response['hist']['stats']['mean']['X']['value'])
        self.assertEquals(2.88661, response['hist']['stats']['rms']['X']['value'])
        self.assertEquals(1, response['hist']['xaxis']['first']['id'])
        self.assertEquals(0, response['hist']['xaxis']['first']['value'])
        self.assertEquals(100, response['hist']['xaxis']['last']['id'])
        self.assertEquals(10, response['hist']['xaxis']['last']['value'])
        self.assertEquals(1, response['hist']['yMax'])
        self.assertEquals(-1, response['hist']['yMin'])

    def fetch_histogram_json(self, name):
        histogram_url = '%sjsonfairy/archive/%d%s/%s?formatted=false' \
                        % (self.base_url, self.run_number, self.dataset, name)
        histogram_response = urllib2.urlopen(histogram_url)
        histogram_content = histogram_response.read()
        print 'Histogram fetched from %s with status %d' % (histogram_url, histogram_response.getcode())
        self.assertEquals(histogram_response.getcode(), 200, 'Request failed. Status code received not 200')
        histogram_json = json.loads(histogram_content)
        return histogram_json
