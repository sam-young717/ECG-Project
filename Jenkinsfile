pipeline {
    agent any
    stages {
        stage('Publishing Static Analysis Results') {
            steps {
                recordIssues (
                    tools: [
                        parasoftFindings (
                            pattern: 'reports/report.xml',
                            localSettingsPath: 'settings.properties'
                        )
                    ]
                )
            }
        }
    }
}