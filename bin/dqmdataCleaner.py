#!/usr/bin/env python

import os
import time
import datetime
import re
import sys
from optparse import OptionParser


class RootFilesFilter:
    def __init__(self, path, filesMarkedForDeletion, referenceDate, versionsToKeep, noOutput):
        self.pathToAnalyse = path

        self.filesMarkedForDeletion = filesMarkedForDeletion

        # convert the given date to epoch time
        if referenceDate != None:
            self.referenceTimestamp = time.mktime(referenceDate.timetuple())
        else:
            self.referenceTimestamp = None

        self.versionsToKeep = versionsToKeep
        self.noOutput = noOutput

        self.OutdatedFiles = {}
        self.OutdatedFilesSize = 0 # in kBytes

        self.VersionedFiles = {}
        self.VersionedFilesSize = 0 # in kBytes

        self.RootFilesExtensions = ('.ROOT', '.root')


    def find_files(self):
        self.OutdatedFiles = {}
        self.VersionedFiles = {}

        self._walk(self.pathToAnalyse)


    def _walk(self, path):
        for currentDir, directories, files in os.walk(path):
            # filter the ROOT files and work only with them
            files = self._select_root_files_only(files)

            # apply all filters specified by the user

            # filter the outdated files and update the list of files to be processed by the other filter
            if self.referenceTimestamp != None:
                files = self._select_outdated_files(currentDir, files)

            # filter versioned files
            if self.versionsToKeep != None:
                self._select_versioned_files(currentDir, files)


    def _select_root_files_only(self, files):
        rootFiles = []

        for file in files:
            if os.path.splitext(file)[1] in self.RootFilesExtensions:
                rootFiles.append(file)

        return rootFiles


    def _select_outdated_files(self, currentDir, rootFiles):
        #self.OutdatedFiles[currentDir] = []
        upToDateFiles = []

        for file in rootFiles:
            fullFilePath = os.path.join(currentDir, file)
            if self.referenceTimestamp > os.path.getmtime(fullFilePath):
                # file is older than the date specified os it should be marked for delete
                self.OutdatedFiles.setdefault(currentDir, []).append(file)
                self.OutdatedFilesSize += os.path.getsize(fullFilePath) / 1024.
            else:
                upToDateFiles.append(file)

        # if there are some outdated files just sort them 
        if self.OutdatedFiles.has_key(currentDir):
            self.OutdatedFiles[currentDir].sort()

        return upToDateFiles


    def _select_versioned_files(self, currentDir, rootFiles):
        subsystemRunNumberGroups = {}

        for file in rootFiles:
            # separate files by sub-systems and run-numbers
            fileNameSplit = re.split('_', file)
            # the key consist of the sub-system and run-number concatenated with '_' - e.g. EcalPreshower_R000179816
            key = fileNameSplit[2] + '_' + fileNameSplit[3][:10]
            subsystemRunNumberGroups.setdefault(key, []).append(file) # put the file in the appropriate group

        self.VersionedFiles[currentDir] = {}
        for key in subsystemRunNumberGroups.iterkeys():
            # process only files that have more than "versionsToKeep" versions for a given set of sub-system_run-number
            if len(subsystemRunNumberGroups[key]) > self.versionsToKeep:
                # the individual sub-systems and run-numbers are separated so the list of version files can be sorted

                subsystemRunNumberGroups[key].sort() # DOES NOT WORK CORRECTLY WITH VERSION NUMBERS HIGHER THAN 9999

                # the list of sorted files is divided into two lists:
                # to be deleted - all the files with the exception of the last "versionsToKeep" files
                # to be kept - only the most recent "versionsToKeep" files
                self.VersionedFiles[currentDir][key] = [[],[]]
                self.VersionedFiles[currentDir][key][0] = subsystemRunNumberGroups[key][:-self.versionsToKeep]
                self.VersionedFiles[currentDir][key][1] = subsystemRunNumberGroups[key][-self.versionsToKeep:]

                # calculate the size of the files marked to be deleted
                for fileToBeDeleted in self.VersionedFiles[currentDir][key][0]:
                    self.VersionedFilesSize += os.path.getsize(os.path.join(currentDir, fileToBeDeleted)) / 1024.

        # if no versioned files are found remove the directory from the dictionary
        if len(self.VersionedFiles[currentDir]) == 0:
            del self.VersionedFiles[currentDir]


    def show_selected_files(self):
        if not self.noOutput:
            # join the two sets of directories with files to be deleted and sort them
            directories = sorted(self.OutdatedFiles.keys() + self.VersionedFiles.keys())
            for directory in directories:
                print('DIR: ' + '"' + directory + '"')

                # print the outdated files that are to be deleted if any
                if self.OutdatedFiles.has_key(directory):
                    print('\t' + 'Outdated files to be deleted:')
                    for file in self.OutdatedFiles[directory]:
                        print('\t\t' + file)
                    print('')

                # print the versioned files that are to be deleted and also that are to be kept
                if self.VersionedFiles.has_key(directory):
                    print('\t' + 'Versioned files:')
                    for key in sorted(self.VersionedFiles[directory].iterkeys()):
                        print('\t\t' + 'ToBe Deleted:')
                        for file in self.VersionedFiles[directory][key][0]:
                            print('\t\t\t' + file)
                        print('\t\t' + 'ToBe Kept:')
                        for file in self.VersionedFiles[directory][key][1]:
                            print('\t\t\t' + file)
                        print('')


    def log_selected_files(self):
        with open(self.filesMarkedForDeletion, 'w') as markedForDeletion:
            # join the two sets of directories with files to be deleted and sort them
            directories = sorted(self.OutdatedFiles.keys() + self.VersionedFiles.keys())
            for directory in directories:
                # log the outdated files that are to be deleted if any
                if self.OutdatedFiles.has_key(directory):
                    for file in self.OutdatedFiles[directory]:
                        markedForDeletion.write(directory + '/' + file + '\n')

                # log the versioned files that are to be deleted
                if self.VersionedFiles.has_key(directory):
                    for key in sorted(self.VersionedFiles[directory].iterkeys()):
                        for file in self.VersionedFiles[directory][key][0]:
                            markedForDeletion.write(directory + '/' + file + '\n')
        markedForDeletion.close()


    def show_some_statistics(self):
        print('The space freed by outdated files is: ' + '"' + 
              str( round( self.OutdatedFilesSize/(1024.*1024), 2)) + ' GB"')

        print('The space freed by versioned files is: ' + '"' + 
              str( round( self.VersionedFilesSize/(1024.*1024), 2)) + ' GB"')

        print('The total space freed is: ' + '"' + 
              str( round( (self.OutdatedFilesSize + self.VersionedFilesSize)/(1024.*1024), 2)) + ' GB"\n')


class CommandLineArgsCollector:
    def __init__(self):
        usage = sys.argv[0] + ' [options] PATH_TO_ANALYSE'
        parser = OptionParser(usage=usage)

        parser.add_option('-f',
                          '--file',
                          type='string',
                          dest='FilesMarkedForDeletion',
                          metavar='FILES_MARKED_FOR_DELETION',
                          default='FILES_MARKED_FOR_DELETION',
                          help='Print all ROOT files marked for deletion to a file [TO_BE_DELETED]. If the file exists it will be overwritten.')
        parser.add_option('-d',
                          '--date',
                          type='string',
                          dest='ReferenceDate',
                          metavar='YYYY-MM-DD',
                          help='All the ROOT files older than [YYYY-MM-DD] will be marked for deletion. If the '
                               'user does not specify this option no date filter will be applied at all')
        parser.add_option('-v',
                          '--versions_to_keep',
                          type='int',
                          dest='VersionsToKeep',
                          metavar='VERSIONS_TO_KEEP',
                          help='Specify number of versions to keep. If a ROOT file has many versions only the most '
                               'recent [VERSIONS_TO_KEEP] of them will be kept. The others will be marked for '
                               'deletion. It the user does not specify this option no version filter will be applied '
                               'at all')
        parser.add_option('-q',
                          '--quiet',
                          dest='Quiet',
                          action='store_true',
                          default=False,
                          help='If this flag is specified no output is printed to STDOUT.')

        # parse the user specified arguments
        (options, args) = parser.parse_args()
        self.FilesMarkedForDeletion = options.FilesMarkedForDeletion
        self.ReferenceDate = options.ReferenceDate
        self.VersionsToKeep = options.VersionsToKeep
        self.Quiet = options.Quiet

        self.ArgumentsOk = self._check_arguments(parser, args)


    def _check_arguments(self, parser, args):

        # check self.PathToAnalyse
        if len(args) == 1:
            self.PathToAnalyse = args[0]
        else:
            print('Wrong number of arguments. You have to specify at least PATH_TO_ANALYSE!\n')
            parser.print_help()
            return False

        if not os.path.exists(self.PathToAnalyse): # check whether self.PathToAnalyse exists
            print('The path "' + self.PathToAnalyse + '" does not exists or in not readable!')
            return False

        # check self.ReferenceDate - it should be a valid date string
        if self.ReferenceDate != None:
            dateSplit = self.ReferenceDate.split('-')
            try: # convert self.ReferenceDate to datetime.date object
                self.ReferenceDate = datetime.date(int(dateSplit[0]), int(dateSplit[1]), int(dateSplit[2]))
            except:
                print('"' + self.ReferenceDate + '" - Wrong date format (please use YYYY-MM-DD) or nonexistent date!')
                return False

        # check self.VersionsToKeep
        if (self.VersionsToKeep != None) and (self.VersionsToKeep < 1):
            print('Number of versions to keep should be a positive integer. '
                  'The value you specified is "' + str(self.VersionsToKeep) + '"')
            return False

        return True


if __name__ == '__main__':
    args = CommandLineArgsCollector()

    if args.ArgumentsOk:
        rootFilesFilter = RootFilesFilter(args.PathToAnalyse, args.FilesMarkedForDeletion, args.ReferenceDate, args.VersionsToKeep, args.Quiet)
        rootFilesFilter.find_files()
        rootFilesFilter.show_selected_files()
        rootFilesFilter.log_selected_files()
        rootFilesFilter.show_some_statistics()

