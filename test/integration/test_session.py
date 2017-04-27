import unittest

import base


class SessionTest(base.BaseIntegrationTest):
    def test_session(self):
        session = self.session()
        choose_sample_json = self.chooseSample(session)
        self.assertEqual('AutoUpdate', choose_sample_json[0]['kind'], 'Choose sample json[0] is not AutoUpdate')
        self.assertEqual('DQMSample', choose_sample_json[1]['kind'], 'Choose sample json[0] is not AutoUpdate')


if __name__ == '__main__':
    unittest.main()
