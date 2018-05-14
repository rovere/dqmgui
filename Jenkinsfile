pipeline {
  agent none
  stages {
    stage('Run Tests') {
      failFast true
      parallel {
        stage('Start') {
          agent {
            label "dqmgui-ci-worker-start" 
          }
          steps {
            // start server in development mode, wait for it to start and print logs
            sh '''
              source /data/srv/current/apps/dqmgui/128/etc/profile.d/env.sh
              /data/srv/current/config/dqmgui/manage -f dev start "I did read documentation"
              # check if render plugins are there
              ls /data/srv/state/dqmgui/dev/render/*ext
              if [ $? != 0 ]; then
                exit 1
              fi
              pgrep -f dqmgui
              sleep 10
              pgrep -f dqmgui
              if [ $? != 0 ]; then
                exit 1
              fi
              for i in /data/srv/logs/dqmgui/dev/*; do echo $i:; cat $i; done
            '''
          }
        }
        stage('Integration') {
          agent {
            label "dqmgui-ci-worker-integration"
          }
          steps {
            checkout scm
            // start server in development mode, wait for it to start and print logs
            // integration tests use visDQMUpload script so we setup DQM env variables before running
            sh '''
                source /data/srv/current/apps/dqmgui/128/etc/profile.d/env.sh
                monDistPatch -s DQM
                if [ $? != 0 ]; then
                  exit 1
                fi
                /data/srv/current/config/dqmgui/manage -f dev start "I did read documentation"
                # check if render plugins are there
                ls /data/srv/state/dqmgui/dev/render/*ext
                if [ $? != 0 ]; then
                  exit 1
                fi
                pgrep -f dqmgui
                sleep 10
                pgrep -f dqmgui
                if [ $? != 0 ]; then
                  exit 1
                fi
                for i in /data/srv/logs/dqmgui/dev/*; do echo $i:; cat $i; done
                nosetests --verbosity=3 --where=test/integration/ --with-xunit
            '''
          }
        }
        stage('Index Regression') {
          agent {
            label "dqmgui-ci-worker-index"
          }
          steps {
            checkout scm
            // start server in development mode, wait for it to start and print logs
            sh '''
                source /data/srv/current/apps/dqmgui/128/etc/profile.d/env.sh
                monDistPatch -s DQM
                if [ $? != 0 ]; then
                  exit 1
                fi
                /data/srv/current/config/dqmgui/manage -f dev start "I did read documentation"
                ls /data/srv/state/dqmgui/dev/render/*ext
                if [ $? != 0 ]; then
                  exit 1
                fi
                pgrep -f dqmgui
                sleep 10
                pgrep -f dqmgui
                if [ $? != 0 ]; then
                  exit 1
                fi
                for i in /data/srv/logs/dqmgui/dev/*; do echo $i:; cat $i; done
                source /data/srv/current/apps/dqmgui/128/etc/profile.d/env.sh
                wget https://rovere.web.cern.ch/rovere/test_index.tar.bz2
                tar xf test_index.tar.bz2
                python test/index/regression.py test_index/
            '''
          }
        }
      }
    }
  }
}
