node('dqmgui-ci-worker') {
    stage('Build') {
        checkout scm
        // environment debug
        sh '''
            id
            pwd
            python --version
            env
            ls -al
            ls -al /data/srv
        '''

        // init environment variables and patch the version in the image with latest from repository
        sh '''
            for s in /data/srv/current/*/*/*/*/*/etc/profile.d/init.sh; do . $s; done
            source /data/srv/current/apps/dqmgui/128/etc/profile.d/env.sh
            monDistPatch -s DQM
        '''
    }
    stage('Start') {
        // start server in development mode, wait for it to start and print logs
        sh '''
            source /data/srv/current/apps/dqmgui/128/etc/profile.d/env.sh
            /data/srv/current/config/dqmgui/manage -f dev start "I did read documentation"
            ps aux | grep dqm
            sleep 10
            ps aux | grep dqm
            for i in /data/srv/logs/dqmgui/dev/*; do echo $i:; cat $i; done
        '''
    }
    stage('Integration Test') {
        try {
            // integration tests use visDQMUpload script so we setup DQM env variables before running
            sh '''
                ps aux | grep dqm
                source /data/srv/current/apps/dqmgui/128/etc/profile.d/env.sh
                nosetests --where=test/integration/ --with-xunit
            '''
        } finally {
            // Application logs printed. Helpful in case of failures
           sh '''
           for i in /data/srv/logs/dqmgui/dev/*; do echo $i:; cat $i; done
           '''
           step([$class: 'JUnitResultArchiver', testResults: 'nosetests.xml'])
        }

    }
    stage('Index regression') {
        // Extracts and validates new code works can index an older snapshot of index
        sh '''
            source /data/srv/current/apps/dqmgui/128/etc/profile.d/env.sh
            wget https://rovere.web.cern.ch/rovere/test_index.tar.bz2
            tar xf test_index.tar.bz2
            python test/index/regression.py test_index/
        '''
    }
//    stage('GUI Test') {
//        try {
//            // GUI tests use visDQMUpload script so we setup DQM env variables before running
//            sh '''
//                source /data/srv/current/apps/dqmgui/128/etc/profile.d/env.sh
//                nosetests --where=test/gui/ --with-xunit
//            '''
//        } finally {
//            // Application logs printed. Helpful in case of failures
//           sh '''
//           for i in /data/srv/logs/dqmgui/dev/*; do echo $i:; cat $i; done
//           '''
//           step([$class: 'JUnitResultArchiver', testResults: 'nosetests.xml'])
//        }
//    }
}
