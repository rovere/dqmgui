import unittest
import ConfigParser
import hashlib
import json
import os
import subprocess
import re
import time
import shutil
import tempfile
import urllib2
from stat import ST_SIZE
from sys import stdout

import rootgen


class UploadWatchTimeout(Exception):
    pass


class BaseIntegrationTest(unittest.TestCase):
    
    @classmethod
    def setUpClass(cls):
        super(BaseIntegrationTest, cls).setUpClass()
        config = ConfigParser.ConfigParser()
        script_dir = os.path.dirname(os.path.realpath(__file__))
        config.read(script_dir + '/config.cfg')
        cls.base_url = config.get('dqm', 'base_url')
        cls.upload_watch_interval = float(config.get('dqm', 'upload_watch_interval'))
        cls.upload_watch_retries = int(config.get('dqm', 'upload_watch_retries'))
        cls.temp_dir = tempfile.mkdtemp()
        print 'setUp'

    @classmethod
    def tearDownClass(cls):
        super(BaseIntegrationTest, cls).tearDownClass()
        shutil.rmtree(cls.temp_dir)
        print 'tearDown'

    @classmethod
    def upload(cls, filename, path):

        try:
            subprocess.check_call(["visDQMUpload", cls.base_url, path])
        except (OSError, subprocess.CalledProcessError) as e:
            print 'visDQMUpload script invokation failed'
            print e
            try:
                cls.upload_requests(filename, path)
            except ImportError as e:
                print 'Either visDQMUpload must be available on path or requests library must be installed manually.'
                raise e

    @classmethod
    def upload_requests(cls, filename, path):
        import requests

        url = cls.base_url + 'data/put'
        files = {'file': (filename, open(path, 'rb'))}
        data = {'size': str(os.stat(path)[ST_SIZE]),
                'checksum': 'md5:%s' % hashlib.md5(file(path).read()).hexdigest()}
        headers = {'User-agent': 'Integration tests'}
        response = requests.post(url, data=data, files=files, headers=headers)

        print 'Uploaded. HTTP Status: %d, DQM-Status-Code: %s, DQM-Status-Message: %s, DQM-Status-Detail: %s, Body: %s' \
              % (response.status_code, response.headers['DQM-Status-Code'], response.headers['DQM-Status-Message'],
                 response.headers['DQM-Status-Detail'], response.content)

    @classmethod
    def prepareIndex(cls, content, dataset='IntegTest'):
        (filename, run, dataset, path) = rootgen.create_file(content, directory=cls.temp_dir, dataset=dataset)
        cls.upload(filename, path)
        cls.uploadWatch(dataset)

        return filename, run, dataset

    @classmethod
    def session(cls):
        create_session_response = urllib2.urlopen(cls.base_url)
        create_session_content = create_session_response.read()
        if create_session_response.getcode() != 200:
            raise RuntimeError('Request failed. Status code re1ceived not 200')

        session = re.search('/dqm/dev/session/([\w\d]+)', create_session_content)
        return session.group(1)

    @classmethod
    def chooseSample(cls, session):
        choose_sample_url = '%ssession/%s/chooseSample?vary=run;order=dataset' % (cls.base_url, session)
        response = urllib2.urlopen(choose_sample_url)
        content = response.read()
        if response.getcode() != 200:
            raise RuntimeError('Request failed. Status code re1ceived not 200')
        content = re.sub('\)$', '', re.sub('^\(', '', content)).replace('\'', '"')
        json_content = json.loads(content)
        return json_content

    @classmethod
    def uploadWatch(cls, dataset):
        stdout.write('Upload watch started for dataset %s. Checking every %s seconds' % (dataset, cls.upload_watch_interval))
        stdout.flush()
        session = cls.session()
        start_time = time.time()
        i = 0
        while i < cls.upload_watch_retries:
            time.sleep(cls.upload_watch_interval)
            stdout.write('.')
            stdout.flush()
            choose_sample_json = cls.chooseSample(session)
            for types in choose_sample_json[1]['items']:
                for item in types['items']:
                    if dataset == item['dataset']:
                        print ''
                        print 'Dataset %s found in %.3f with type %s, run %s, importversion %s, version %s' \
                              % (dataset, (time.time() - start_time), item['type'], item['run'], item['importversion'],
                                 item['version'])
                        return True
            i += 1

        raise UploadWatchTimeout('Either server was too slow to index the file, or there was an error parsing it. '
                                 'Check /data/srv/logs/dqmgui/dev/agent-import-128-*.log for details.')
